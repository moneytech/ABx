/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "InventoryComp.h"
#include "ItemFactory.h"
#include "Actor.h"
#include "Skill.h"
#include <sa/Transaction.h>
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>

namespace Game {
namespace Components {

void InventoryComp::Update(uint32_t timeElapsed)
{
    VisitEquipement([&](Item& item)
    {
        item.Update(timeElapsed);
        return Iteration::Continue;
    });
}

EquipPos InventoryComp::SetEquipment(uint32_t itemId)
{
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(itemId);
    if (!item)
        return EquipPos::None;

    const EquipPos pos = item->GetEquipPos();
    {
        uint32_t old = RemoveEquipment(pos);
        if (old)
        {
            if (!owner_.AddToInventory(old))
            {
                return EquipPos::None;
            }
            auto* oldItem = cache->Get(old);
            if (oldItem)
                oldItem->OnUnequip(&owner_);
        }
    }
    equipment_[pos] = itemId;
    item->OnEquip(&owner_);
    return pos;
}

Item* InventoryComp::GetEquipment(EquipPos pos) const
{
    auto item = equipment_.find(pos);
    if (item == equipment_.end() || (*item).second == 0)
        return nullptr;

    auto* cache = GetSubsystem<ItemsCache>();
    return cache->Get((*item).second);
}

uint32_t InventoryComp::RemoveEquipment(EquipPos pos)
{
    if (equipment_[pos] == 0)
        return 0;
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(equipment_[pos]);
    if (!item)
        return 0;

    item->OnUnequip(&owner_);
    equipment_[pos] = 0;
    return item->id_;
}

EquipPos InventoryComp::EquipInventoryItem(ItemPos pos)
{
    Item* pItem = GetInventoryItem(pos);
    if (!pItem || pItem->GetEquipPos() == EquipPos::None)
        return EquipPos::None;
    uint32_t item = RemoveInventoryItem(pos);
    EquipPos ePos = SetEquipment(item);
    if (ePos != EquipPos::None)
        return ePos;

    // Rollback
    inventory_->InternalSetItem(item);
    return EquipPos::None;
}

void InventoryComp::WriteItemUpdate(const Item* const item, Net::NetworkMessage* message, bool isChest)
{
    if (!item || !message)
        return;
    message->AddByte((!isChest) ? AB::GameProtocol::ServerPacketType::InventoryItemUpdate : AB::GameProtocol::ServerPacketType::ChestItemUpdate);
    AB::Packets::Server::InventoryItemUpdate packet = {
        static_cast<uint16_t>(item->data_.type),
        item->data_.index,
        static_cast<uint8_t>(item->concreteItem_.storagePlace),
        item->concreteItem_.storagePos,
        item->concreteItem_.count,
        item->concreteItem_.value,

    };
    AB::Packets::Add(packet, *message);
}

bool InventoryComp::SetInventoryItem(uint32_t itemId, Net::NetworkMessage* message)
{
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(itemId);
    if (!item)
        return false;

    item->concreteItem_.playerUuid = owner_.GetPlayerUuid();
    item->concreteItem_.accountUuid = owner_.GetAccountUuid();
    const bool ret = inventory_->SetItem(itemId, [message](const Item* const item)
    {
        InventoryComp::WriteItemUpdate(item, message, false);
    });
    if (!ret)
        owner_.CallEvent<void(void)>(EVENT_ON_INVENTORYFULL);
    return ret;
}

bool InventoryComp::SetChestItem(uint32_t itemId, Net::NetworkMessage* message)
{
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(itemId);
    if (!item)
        return false;

    item->concreteItem_.playerUuid = owner_.GetPlayerUuid();
    item->concreteItem_.accountUuid = owner_.GetAccountUuid();
    const bool ret = chest_->SetItem(itemId, [message](const Item* const item)
    {
        InventoryComp::WriteItemUpdate(item, message, true);
    });
    if (!ret)
        owner_.CallEvent<void(void)>(EVENT_ON_INVENTORYFULL);
    return ret;
}

void InventoryComp::SetUpgrade(Item& item, ItemUpgrade type, uint32_t upgradeId)
{
    const bool isEquipped = item.concreteItem_.storagePlace == AB::Entities::StoragePlaceEquipped;
    if (isEquipped)
    {
        Item* old = item.GetUpgrade(type);
        if (old)
            old->OnUnequip(&owner_);
    }
    Item* n = item.SetUpgrade(type, upgradeId);
    if (n && isEquipped)
        n->OnEquip(&owner_);
}

void InventoryComp::RemoveUpgrade(Item& item, ItemUpgrade type)
{
    const bool isEquipped = item.concreteItem_.storagePlace == AB::Entities::StoragePlaceEquipped;
    Item* old = item.GetUpgrade(type);
    if (old)
    {
        if (isEquipped)
            old->OnUnequip(&owner_);
        item.RemoveUpgrade(type);
    }
}

Item* InventoryComp::GetWeapon() const
{
    Item* result = GetEquipment(EquipPos::WeaponLeadHand);
    if (!result)
        result = GetEquipment(EquipPos::WeaponTwoHanded);
    return result;
}

int InventoryComp::GetArmor(DamageType damageType, DamagePos pos)
{
    Item* item = nullptr;
    auto* cache = GetSubsystem<ItemsCache>();
    switch (pos)
    {
    case DamagePos::Head:
        item = cache->Get(equipment_[EquipPos::ArmorHead]);
        break;
    case DamagePos::Chest:
        item = cache->Get(equipment_[EquipPos::ArmorChest]);
        break;
    case DamagePos::Hands:
        item = cache->Get(equipment_[EquipPos::ArmorHands]);
        break;
    case DamagePos::Legs:
        item = cache->Get(equipment_[EquipPos::ArmorLegs]);
        break;
    case DamagePos::Feet:
        item = cache->Get(equipment_[EquipPos::ArmorFeet]);
        break;
    case DamagePos::NoPos:
        return 0;
    default:
        break;
    }
    if (!item)
        return 0;
    int armor = 0;
    item->GetArmor(damageType, armor);
    return armor;
}

float InventoryComp::GetArmorPenetration()
{
    Item* weapon = GetWeapon();
    if (!weapon)
        return 0.0f;
    float value = 0.0f;
    weapon->GetArmorPenetration(value);
    return value;
}

uint32_t InventoryComp::GetAttributeRank(Attribute index)
{
    uint32_t result = 0;
    VisitEquipement([&](Item& item)
    {
        item.GetAttributeRank(index, result);
        return Iteration::Continue;
    });
    return result;
}

void InventoryComp::GetResources(int& maxHealth, int& maxEnergy)
{
    VisitEquipement([&](Item& item)
    {
        item.GetResources(maxHealth, maxEnergy);
        return Iteration::Continue;
    });
}

void InventoryComp::GetSkillRecharge(Skill* skill, uint32_t &recharge)
{
    VisitEquipement([&](Item& item)
    {
        item.GetSkillRecharge(skill, recharge);
        return Iteration::Continue;
    });
}

void InventoryComp::GetSkillCost(Skill* skill, int32_t& activation, int32_t& energy, int32_t& adrenaline, int32_t& overcast, int32_t& hp)
{
    VisitEquipement([&](Item& item)
    {
        item.GetSkillCost(skill, activation, energy, adrenaline, overcast, hp);
        return Iteration::Continue;
    });
}

}
}

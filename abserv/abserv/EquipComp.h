#pragma once

#include "Item.h"
#include "Damage.h"

namespace Game {

class Actor;

namespace Components {

enum class ItemPos : uint32_t
{
    ArmorHead = 0,
    ArmorChest,
    ArmorHands,
    ArmorLegs,
    ArmorFeet,

    WeaponLeadHand,
    WeaponOffHand
};

/// Equipment, like Armor, weapons, weapon mods etc.
/// All this stuff adds effects to the actor. These effects are not visible in the clients effects window.
class EquipComp
{
private:
    Actor& owner_;
    std::map<ItemPos, std::unique_ptr<Item>> items_;
public:
    EquipComp() = delete;
    explicit EquipComp(Actor& owner) :
        owner_(owner)
    { }
    // non-copyable
    EquipComp(const EquipComp&) = delete;
    EquipComp& operator=(const EquipComp&) = delete;
    ~EquipComp() = default;

    void Update(uint32_t timeElapsed);

    /// Set upgrade
    void SetItem(ItemPos pos, uint32_t index);
    Item* GetItem(ItemPos pos) const;
    void RemoveItem(ItemPos pos);
    void SetUpgrade(ItemPos pos, ItemUpgrade type, uint32_t index);
    /// Get lead hand weapon
    Item* GetWeapon() const;
    int GetArmor(DamageType damageType, DamagePos pos);
    float GetArmorPenetration();
};

}
}

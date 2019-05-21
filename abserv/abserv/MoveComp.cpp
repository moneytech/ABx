#include "stdafx.h"
#include "MoveComp.h"
#include "Actor.h"
#include "CollisionComp.h"
#include "MathUtils.h"
#include "Game.h"
#include "MathUtils.h"
#include "Mechanic.h"

namespace Game {
namespace Components {

void MoveComp::Update(uint32_t timeElapsed, uint32_t flags)
{
    if (owner_.stateComp_.GetState() != AB::GameProtocol::CreatureStateMoving)
        return;
    if (flags == 0)
        return;

    oldPosition_ = owner_.transformation_.position_;

    if ((flags & UpdateFlagMove) == UpdateFlagMove)
        UpdateMove(timeElapsed);
    if ((flags & UpdateFlagTurn) == UpdateFlagTurn)
        UpdateTurn(timeElapsed);

    velocity_ = ((oldPosition_ - owner_.transformation_.position_) / (static_cast<float>(timeElapsed) / 1000)).Abs();
    moved_ = !velocity_.Equals(Math::Vector3::Zero);
}

bool MoveComp::SetPosition(const Math::Vector3& pos)
{
    oldPosition_ = owner_.transformation_.position_;

    HeadTo(pos);
    owner_.transformation_.position_ = pos;
    // Keep on ground
    float y = owner_.GetGame()->map_->GetTerrainHeight(owner_.transformation_.position_);
    owner_.transformation_.position_.y_ = y;
    if (owner_.collisionComp_)
        // We need to do it here because this is not called from Update()
        owner_.collisionComp_->ResolveCollisions();

    bool moved = oldPosition_ != owner_.transformation_.position_;

    if (moved && owner_.octant_)
    {
        // We need to do it here because this is not called from Update()
        Math::Octree* octree = owner_.octant_->GetRoot();
        octree->AddObjectUpdate(&owner_);
    }

    if (moved)
        moved_ = true;

    return moved;
}

void MoveComp::HeadTo(const Math::Vector3& pos)
{
    const float worldAngle = -owner_.transformation_.position_.AngleY(pos) - Math::M_PIF;
    SetDirection(worldAngle);
}

void MoveComp::Move(float speed, const Math::Vector3& amount)
{
    owner_.transformation_.Move(speed, amount);

    // Keep on ground
    float y = owner_.GetGame()->map_->GetTerrainHeight(owner_.transformation_.position_);
    owner_.transformation_.position_.y_ = y;

    bool moved = !oldPosition_.Equals(owner_.transformation_.position_);
    if (moved)
        moved_ = true;
}

void MoveComp::UpdateMove(uint32_t timeElapsed)
{
    const float speed = GetSpeed(timeElapsed, BASE_SPEED);
    if ((moveDir_ & AB::GameProtocol::MoveDirectionNorth) == AB::GameProtocol::MoveDirectionNorth)
        Move(speed, Math::Vector3::UnitZ);
    if ((moveDir_ & AB::GameProtocol::MoveDirectionSouth) == AB::GameProtocol::MoveDirectionSouth)
        // Move slower backward
        Move(speed, Math::Vector3::Back / 2.0f);
    if ((moveDir_ & AB::GameProtocol::MoveDirectionWest) == AB::GameProtocol::MoveDirectionWest)
        Move(speed, Math::Vector3::Left / 2.0f);
    if ((moveDir_ & AB::GameProtocol::MoveDirectionEast) == AB::GameProtocol::MoveDirectionEast)
        Move(speed, Math::Vector3::UnitX / 2.0f);
}

void MoveComp::Turn(float angle)
{
    owner_.transformation_.Turn(angle);
}

void MoveComp::UpdateTurn(uint32_t timeElapsed)
{
    const float speed = GetSpeed(timeElapsed, BASE_TURN_SPEED);
    if ((turnDir_ & AB::GameProtocol::TurnDirectionLeft) == AB::GameProtocol::TurnDirectionLeft)
    {
        Turn(speed);
        turned_ = true;
    }
    if ((turnDir_ & AB::GameProtocol::TurnDirectionRight) == AB::GameProtocol::TurnDirectionRight)
    {
        Turn(-speed);
        turned_ = true;
    }
}

void MoveComp::SetDirection(float worldAngle)
{
    const float ang = owner_.transformation_.GetYRotation();
    Math::NormalizeAngle(worldAngle);
    if (!Math::Equals(ang, worldAngle, 0.0001f))
    {
        owner_.transformation_.SetYRotation(worldAngle);
        directionSet_ = true;
    }
}

void MoveComp::Write(Net::NetworkMessage& message)
{
    if (speedDirty_)
    {
        speedDirty_ = false;
        message.AddByte(AB::GameProtocol::GameObjectMoveSpeedChange);
        message.Add<uint32_t>(owner_.id_);
        message.Add<float>(GetSpeedFactor());
    }

    if (moved_)
    {
        message.AddByte(AB::GameProtocol::GameObjectPositionChange);
        message.Add<uint32_t>(owner_.id_);
        message.Add<float>(owner_.transformation_.position_.x_);
        message.Add<float>(owner_.transformation_.position_.y_);
        message.Add<float>(owner_.transformation_.position_.z_);
        moved_ = false;
    }

    // The rotation may change in 2 ways: Turn and SetWorldDirection
    if (turned_ || directionSet_)
    {
        message.AddByte(AB::GameProtocol::GameObjectRotationChange);
        message.Add<uint32_t>(owner_.id_);
        message.Add<float>(owner_.transformation_.GetYRotation());
        message.Add<uint8_t>(directionSet_ ? 1 : 0);
        turned_ = false;
        directionSet_ = false;
    }
}

}
}

#pragma once

#include <AB/ProtocolCodes.h>
#include "PropStream.h"
#include <Urho3DAll.h>

enum ObjectType
{
    ObjectTypeStatic = 0,
    ObjectTypeNpc = 1,
    ObjectTypePlayer = 2,
    ObjectTypeSelf = 3,
    ObjectTypeAreaOfEffect,          // Area that affects actors in it, e.g. a well
    ObjectTypeItemDrop,
    ObjectTypeProjectile,
};

class GameObject : public LogicComponent
{
    URHO3D_OBJECT(GameObject, LogicComponent)
protected:
    AB::GameProtocol::CreatureState creatureState_{ AB::GameProtocol::CreatureState::Idle };
    float speedFactor_{ 1.0f };
    bool HasHealthBar() const
    {
        return selectable_ && (objectType_ == ObjectTypeNpc || objectType_ == ObjectTypePlayer || objectType_ == ObjectTypeSelf);
    }
    // A real player, not an NPC or such
    bool IsPlayingCharacter() const
    {
        return objectType_ == ObjectTypePlayer || objectType_ == ObjectTypeSelf;
    }
public:
    GameObject(Context* context);
    ~GameObject() override;

    virtual void Init(Scene*, const Vector3&, const Quaternion&,
        AB::GameProtocol::CreatureState)
    {}

    uint32_t gameId_{ 0 };
    unsigned index_{ 0 };
    ObjectType objectType_{ ObjectTypeStatic };
    bool undestroyable_{ false };
    bool selectable_{ false };
    uint32_t count_ = 1;
    uint32_t value_ = 0;
    int64_t spawnTickServer_{ 0 };
    /// Player has selected this object
    bool playerSelected_{ false };
    uint32_t groupId_{ 0 };
    uint8_t groupPos_{ 0 };
    SharedPtr<SoundSource3D> soundSource_;

    virtual void Unserialize(PropReadStream&) {}

    double GetServerTime(int64_t tick) const
    {
        return static_cast<double>(tick - spawnTickServer_) / 1000.0;
    }
    double GetClientTime() const;

    virtual void SetYRotation(int64_t time, float rad, bool updateYaw);
    virtual void SetCreatureState(int64_t time, AB::GameProtocol::CreatureState newState);
    AB::GameProtocol::CreatureState GetCreatureState() const
    {
        return creatureState_;
    }
    float GetSpeedFactor() const
    {
        return speedFactor_;
    }
    virtual void SetSpeedFactor(int64_t time, float value);
    virtual void OnSkillError(AB::GameProtocol::SkillError) { }
    virtual void OnAttackError(AB::GameProtocol::AttackError) { }
    virtual void OnEffectAdded(uint32_t, uint32_t) { }
    virtual void OnEffectRemoved(uint32_t) { }

    float GetYRotation() const;
    virtual void MoveTo(int64_t time, const Vector3& newPos);
    virtual void ForcePosition(int64_t time, const Vector3& newPos);
    bool IsSelectable() const { return selectable_; }
    IntVector2 WorldToScreenPoint();
    IntVector2 WorldToScreenPoint(Vector3 pos);

    virtual void RemoveFromScene() {}
};

template <typename T>
inline bool Is(const GameObject&)
{
    return false;
}

/// Returns false when obj is null
template <typename T>
inline bool Is(const GameObject* obj)
{
    return obj && Is<T>(*obj);
}

template <>
inline bool Is<GameObject>(const GameObject&)
{
    return true;
}

template <typename T>
inline const T& To(const GameObject& obj)
{
    assert(Is<T>(obj));
    return static_cast<const T&>(obj);
}

template <typename T>
inline T& To(GameObject& obj)
{
    assert(Is<T>(obj));
    return static_cast<T&>(obj);
}

template <typename T>
inline const T* To(const GameObject* obj)
{
    if (!Is<T>(obj))
        return nullptr;
    return static_cast<const T*>(obj);
}

template <typename T>
inline T* To(GameObject* obj)
{
    if (!Is<T>(obj))
        return nullptr;
    return static_cast<T*>(obj);
}

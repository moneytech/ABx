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

#pragma once

#include "GameObject.h"
#include "Extrapolator.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Profession.h>
#include <AB/Entities/Item.h>
#include <abshared/TemplEncoder.h>
#include "ValueBar.h"
#include <Urho3DAll.h>

using namespace Urho3D;

static const float MOVE_FORCE = 0.7f;
static const float INAIR_MOVE_FORCE = 0.02f;
static const float BRAKE_FORCE = 0.2f;
static const float JUMP_FORCE = 6.0f;
static const float INAIR_THRESHOLD_TIME = 0.1f;

static const StringHash ANIM_IDLE("Idle");
static const StringHash ANIM_WALK("Walk");
static const StringHash ANIM_RUN("Run");
static const StringHash ANIM_JUMP("Jump");
static const StringHash ANIM_SIT("Sit");
static const StringHash ANIM_ATTACK_MELEE("Melee");
static const StringHash ANIM_ATTACK_PISTOL("Shoot Pistol");
static const StringHash ANIM_ATTACK_GUN("Shoot Gun");
static const StringHash ANIM_HURT("Hurt");
static const StringHash ANIM_DYING("Dying");
static const StringHash ANIM_DEAD("Dead");
static const StringHash ANIM_CRY("Cry");
static const StringHash ANIM_CASTING("Casting");
static const StringHash ANIM_TAUNTING("Taunting");
static const StringHash ANIM_PONDER("Ponder");
static const StringHash ANIM_WAVE("Wave");
static const StringHash ANIM_LAUGH("Laugh");
static const StringHash ANIM_ATTACK("Attack");
static const StringHash ANIM_CHEST_OPENING("ChestOpening");
static const StringHash ANIM_CHEST_CLOSING("ChestClosing");

/// Stop playing current sound
static const StringHash SOUND_NONE("None");
static const StringHash SOUND_FOOTSTEPS("Footsteps");
static const StringHash SOUND_JUMP("Jump");
static const StringHash SOUND_SKILLFAILURE("SkillFailure");
static const StringHash SOUND_DIE("Die");

static const StringHash COLLADJ_ADD("add");
static const StringHash COLLADJ_SUB("sub");
static const StringHash COLLADJ_MUL("mul");
static const StringHash COLLADJ_DIV("div");

struct ActorStats
{
    unsigned health{ 0 };
    unsigned maxHealth{ 0 };
    int healthRegen{ 0 };
    unsigned energy{ 0 };
    unsigned maxEnergy{ 0 };
    int energyRegen{ 0 };
    unsigned adrenaline{ 0 };
    unsigned overcast{ 0 };
    int morale{ 0 };
};

/// Character component, responsible for physical movement according to controls, as well as animation.
class Actor : public GameObject
{
    URHO3D_OBJECT(Actor, GameObject)
public:
    enum ModelType {
        Static,
        Animated
    };
public:
    /// Construct.
    Actor(Context* context);
    ~Actor() override;

    static void RegisterObject(Context* context);

    static Actor* CreateActor(uint32_t id, Scene* scene,
        const Vector3& position, const Quaternion& rotation,
        AB::GameProtocol::CreatureState state,
        PropReadStream& data);
    /// Handle physics world update. Called by LogicComponent base class.
    void Update(float timeStep) override;
    void MoveTo(int64_t time, const Vector3& newPos) override;
    void ForcePosition(int64_t time, const Vector3& newPos) override;
    void SetYRotation(int64_t time, float rad, bool updateYaw) override;
    void RemoveFromScene() override;
    void SetCreatureState(int64_t time, AB::GameProtocol::CreatureState newState) override;
    void SetSpeedFactor(int64_t time, float value) override;

    void Unserialize(PropReadStream& data) override;
    /// Get position of head or to of the model in world coordinates.
    Vector3 GetHeadPos() const;

    /// Initialize the vehicle. Create rendering and physics components. Called by the application.
    void Init(Scene* scene, const Vector3& position, const Quaternion& rotation,
        AB::GameProtocol::CreatureState state) override;
    bool LoadObject(uint32_t itemIndex, const Vector3& position, const Quaternion& rotation);
    /// Add a model like hair armor etc.
    void AddModel(uint32_t itemIndex);
    void PlaySoundEffect(SoundSource3D* soundSource, const StringHash& type, bool loop = false);
    void PlaySoundEffect(const StringHash& type, bool loop = false);
    void PlaySoundEffect(const String& fileName, const String& name = String::EMPTY);
    bool LoadSkillTemplate(const std::string& templ);
    std::string SaveSkillTemplate();
    void OnSkillError(AB::GameProtocol::SkillError error) override;

    String GetClasses() const;
    String GetClassLevel() const;
    String GetClassLevelName() const;

    void HandlePartyAdded(StringHash eventType, VariantMap& eventData);
    void HandlePartyRemoved(StringHash eventType, VariantMap& eventData);

    uint32_t GetAttributeRank(Game::Attribute index) const;
    void SetAttributeRank(Game::Attribute index, uint32_t value);

    Vector<String> materials_;
    Vector3 velocity_;
    bool autoRun_{ false };
    bool pvpCharacter_{ false };
private:
    /// Player hovers
    bool hovered_{ false };
    SharedPtr<Text> nameLabel_;
    SharedPtr<Window> nameWindow_;
    SharedPtr<Window> speechBubbleWindow_;
    SharedPtr<Text> speechBubbleText_;
    SharedPtr<ValueBar> hpBar_;
    SharedPtr<Text> classLevel_;
    float speechBubbleVisible_{ false };
    void UpdateTransformation();
    void RemoveActorUI();
    void HideSpeechBubble();
    String GetAnimation(const StringHash& hash);
    String GetSoundEffect(const StringHash& hash);
    void UpdateMoveSpeed();
    void HandleNameClicked(StringHash eventType, VariantMap& eventData);
    void HandleAnimationFinished(StringHash eventType, VariantMap& eventData);
    void HandleChatMessage(StringHash eventType, VariantMap& eventData);
    void HandleSkillUse(StringHash eventType, VariantMap& eventData);
    void HandleEndSkillUse(StringHash eventType, VariantMap& eventData);
    void HandleEffectAdded(StringHash eventType, VariantMap& eventData);
    void HandleItemDropped(StringHash eventType, VariantMap& eventData);
    void HandleObjectSecProfessionChange(StringHash eventType, VariantMap& eventData);
    void HandleLoadSkillTemplate(StringHash eventType, VariantMap& eventData);
    void HandleSetAttribValue(StringHash eventType, VariantMap& eventData);
    void HandleSetSkill(StringHash eventType, VariantMap& eventData);
    void HandleGroupMaskChanged(StringHash eventType, VariantMap& eventData);
    static void SetUIElementSizePos(UIElement* elem, const IntVector2& size, const IntVector2& pos);
    bool IsSpeechBubbleVisible() const;
protected:
    AnimatedModel* animatedModel_{ nullptr };
    Actor::ModelType type_{ ModelType::Static };
    SharedPtr<AnimationController> animController_;
    SharedPtr<StaticModel> model_;
    HashMap<StringHash, String> animations_;
    /// Footsteps etc.
    HashMap<StringHash, String> sounds_;
    WeakPtr<GameObject> selectedObject_;
    StringHash currentAnimation_;
public:
    static String GetAnimation(AB::Entities::ModelClass cls, const StringHash& hash);
    Vector3 moveToPos_;
    Quaternion rotateTo_;
    String name_;
    AB::Entities::CharacterSex sex_{ AB::Entities::CharacterSexUnknown };
    uint32_t level_{ 0 };
    AB::Entities::Profession* profession_{ nullptr };
    AB::Entities::Profession* profession2_{ nullptr };
    Game::SkillIndices skills_;
    Game::Attributes attributes_;
    /// Model or effect (in case of AOE) index
    uint32_t itemIndex_;
    AB::Entities::ModelClass modelClass_;
    Extrapolator<3, float> posExtrapolator_;
    ActorStats stats_;
    void ResetSecondProfAttributes();
    bool IsDead() const { return stats_.health == 0; }
    void AddActorUI();
    void SetSelectedObject(SharedPtr<GameObject> object);
    AnimatedModel* GetModel() const { return animatedModel_; }
    GameObject* GetSelectedObject() const
    {
        if (auto sel = selectedObject_.Lock())
            return sel.Get();
        return nullptr;
    }
    uint32_t GetSelectedObjectId() const
    {
        if (auto sel = selectedObject_.Lock())
            return sel->gameId_;
        return 0;
    }
    void PlayAnimation(StringHash animation, bool looped = true, float fadeTime = 0.2f, float speed = 1.0f);
    void PlayObjectAnimation(bool looped = false, float fadeTime = 0.2f, float speed = 1.0f);
    void PlayIdleAnimation(float fadeTime);
    void PlayStateAnimation(float fadeTime = 0.2f);
    void ShowSpeechBubble(const String& text);
    int GetAttributePoints() const;
    int GetUsedAttributePoints() const;
    int GetAvailableAttributePoints() const;
    bool CanIncreaseAttributeRank(Game::Attribute index) const;

    void ChangeResource(AB::GameProtocol::ResourceType resType, int32_t value);

    /// Get lower 16 bits of the group mask
    uint32_t GetFriendMask() const { return groupMask_ & 0xffff; }
    /// Get upper 16 bits of the group mask
    uint32_t GetFoeMask() const { return groupMask_ >> 16; }

    bool IsEnemy(Actor* other) const
    {

        if (!other || other->undestroyable_)
            return false;

        if (groupId_ != 0 && groupId_ == other->groupId_)
            // Return true if we have a matching bit of our foe mask in their friend mask
            return false;
        // Return true if we have a matching bit of our foe mask in their friend mask
        return ((GetFoeMask() & other->GetFriendMask()) != 0);
    }
    bool IsAlly(Actor* other) const
    {
        if (!other || other->undestroyable_)
            return false;
        if (groupId_ != 0 && groupId_ == other->groupId_)
            // Same group members are always friends
            return true;
        // Return true if we have a matching bit of our foe mask in their friend mask
        return ((GetFriendMask() & other->GetFriendMask()) != 0);
    }
    void HoverBegin() { hovered_ = true; }
    void HoverEnd() { hovered_ = false; }
};

template <>
inline bool Is<Actor>(const GameObject& obj)
{
    return obj.objectType_ > ObjectTypeStatic;
}

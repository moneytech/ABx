include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")
include("/scripts/includes/attributes.lua")

-- Touch skill
costEnergy = 0
costAdrenaline = 0
activation = 250
recharge = 30000
overcast = 0
-- HP cost
hp = 0
range = RANGE_CASTING
damageType = DAMAGETYPE_UNKNOWN
effect = SkillEffectInterrupt | SkillEffectGainEnergy
effectTarget = SkillTargetTarget
canInterrupt = SkillTypeSkill
targetType = SkillTargetTypeFoe

function canUse(source, target)
  if (target == nil) then
    -- This skill needs a target
    return SkillErrorInvalidTarget
  end
  if (source:GetId() == target:GetId()) then
    -- Can not use this skill on self
    return SkillErrorInvalidTarget
  end
  if (self:IsInRange(target) == false) then
    -- The target must be in range
    return SkillErrorOutOfRange
  end
  if (source:IsEnemy(target) == false) then
    -- Targets only enemies
    return SkillErrorInvalidTarget
  end
  if (target:IsDead()) then
    -- Can not kill what's already dead :(
    return SkillErrorInvalidTarget
  end
  if (target:IsUsingSkillOfType(SkillTypeSpell, 250) == false) then
    return SkillErrorNotAppropriate
  end
  return SkillErrorNone
end

function onStartUse(source, target)
  if (target == nil) then
    -- This skill needs a target
    return SkillErrorInvalidTarget
  end
  if (source:GetId() == target:GetId()) then
    -- Can not use this skill on self
    return SkillErrorInvalidTarget
  end
  if (self:IsInRange(target) == false) then
    -- The target must be in range
    return SkillErrorOutOfRange
  end
  if (source:IsEnemy(target) == false) then
    -- Targets only enemies
    return SkillErrorInvalidTarget
  end
  if (target:IsDead()) then
    -- Can not kill what's already dead :(
    return SkillErrorInvalidTarget
  end
  source:FaceObject(target)
  return SkillErrorNone
end

function onSuccess(source, target)
  if (target == nil) then
    return SkillErrorInvalidTarget
  end
  if (target:IsDead()) then
    return SkillErrorInvalidTarget
  end
  local skill = target:GetCurrentSkill();
  if (skill == nil) then
    -- No skill to interrupt -> no error just nothing happens
    return SkillErrorNone
  end

  if (target:Interrupt()) then
    if (skill:IsType(SkillTypeSpell)) then
      local attribVal = source:GetAttributeRank(ATTRIB_INSPIRATION)
      local energy = math.floor(attribVal)
      source:AddEnergy(energy)
    end
  end
  return SkillErrorNone
end

function idle(time)
  return self:CreateNode("Idle", { time })
end

function goHome()
  local node = self:CreateNode("GoHome")
    node:SetCondition(self:CreateCondition("HaveHome"))
  return node
end

function wander()
  local node = self:CreateNode("Wander")
    node:SetCondition(self:CreateCondition("HaveWanderRoute"))
  return node
end

local function avoidSelfMeleeDamage()
  -- Dodge melee attacks
  local node = self:CreateNode("Flee")
    node:SetCondition(self:CreateCondition("IsMeleeTarget"))
  return node
end

local function avoidSelfAoeDamage()
  -- Move out of AOE
  local node = self:CreateNode("MoveOutAOE")
    node:SetCondition(self:CreateCondition("IsInAOE"))
  return node
end

function avoidSelfDamage()
  local node = self:CreateNode("Priority")
    node:AddNode(avoidSelfMeleeDamage())
    node:AddNode(avoidSelfAoeDamage())
  return node
end

function stayAlive()
  -- Execute the first child that does not fail
  local node = self:CreateNode("Priority")
    local condition = self:CreateCondition("IsSelfHealthLow")
    -- If we have low HP
    node:SetCondition(condition)
    -- 1. try to heal
    node:AddNode(self:CreateNode("HealSelf"))
    -- 2. If that failes, flee
    node:AddNode(self:CreateNode("Flee"))
  return node
end

function defend()
  local node = self:CreateNode("AttackSelection")
    -- If we are getting attackend AND there is an attacker
    local andCond = self:CreateCondition("And")
      andCond:AddCondition(self:CreateCondition("IsAttacked"))
      local haveAttackers = self:CreateCondition("Filter")
        haveAttackers:SetFilter(self:CreateFilter("SelectAttackers"))
      andCond:AddCondition(haveAttackers)
    node:SetCondition(andCond)
  return node
end

function healAlly()
  -- Priority: Execute the first child that does not fail, either HealOther or MoveTo
  local node = self:CreateNode("Priority")
    local andCond = self:CreateCondition("And")
      andCond:AddCondition(self:CreateCondition("IsAllyHealthLow"))
      local haveTargets = self:CreateCondition("Filter")
        haveTargets:SetFilter(self:CreateFilter("SelectLowHealth"))
      andCond:AddCondition(haveTargets)
    node:SetCondition(andCond)
    -- Heal fails if out of range
    local heal = self:CreateNode("HealOther")
      heal:SetCondition(self:CreateCondition("IsInSkillRange"))
    node:AddNode(heal)

    -- If out of range move to target
    local move = self:CreateNode("MoveTo")
      -- Only move there when not in range
      local notinrange = self:CreateCondition("Not")
        notinrange:AddCondition(self:CreateCondition("IsInSkillRange"))

      move:SetCondition(notinrange)

    -- If out of range move to target
    node:AddNode(move)

  return node
end

function attackAggro()
  local node = self:CreateNode("AttackSelection")
    local haveAggro = self:CreateCondition("Filter")
      haveAggro:SetFilter(self:CreateFilter("SelectAggro"))
    node:SetCondition(haveAggro)
  return node
end

function rezzAlly()
  local node = self:CreateNode("Priority")
    local haveDeadAllies = self:CreateCondition("Filter")
      haveDeadAllies:SetFilter(self:CreateFilter("SelectDeadAllies"))
    node:SetCondition(haveDeadAllies)

    local rezz = self:CreateNode("ResurrectSelection")
      rezz:SetCondition(self:CreateCondition("IsInSkillRange"))
    node:AddNode(rezz)

    -- If out of range move to target
    local move = self:CreateNode("MoveTo")
      -- Only move there when not in range
      local notinrange = self:CreateCondition("Not")
        notinrange:AddCondition(self:CreateCondition("IsInSkillRange"))

      move:SetCondition(notinrange)
    node:AddNode(move)

  return node
end
include("/scripts/includes/chat.lua")
include("/scripts/actors/consts.lua")

name = "Guild Lord"
level = 20
modelIndex = 5     -- Smith body model
sex = SEX_MALE
creatureState = CREATURESTATE_IDLE
prof1Index = 1     -- Warrior
prof2Index = 2     -- Ranger

function onInit()
  return true
end

function onUpdate(timeElapsed)
end

function onClicked(creature)
end

-- self was selected by creature
function onSelected(creature)
end

-- creature collides with self
function onCollide(creature)
end

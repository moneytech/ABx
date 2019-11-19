#include "stdafx.h"
#include "AiIsSelfHealthCritical.h"
#include "../Game.h"
#include "../Npc.h"

namespace AI {
namespace Conditions {

bool IsSelfHealthCritical::Evaluate(Agent& agent, const Node&)
{
    auto& npc = AI::GetNpc(agent);
    if (npc.IsDead())
        // Too late
        return false;
    return npc.resourceComp_->GetHealth() < CRITICAL_HP_THRESHOLD;
}

}
}
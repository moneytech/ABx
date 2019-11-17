#include "stdafx.h"
#include "Node.h"
#include "Agent.h"

namespace AI {

sa::IdGenerator<Id> Node::sIDs;

Node::Node(const ArgumentsType&) :
    id_(sIDs.Next())
{ }

Node::~Node() = default;

bool Node::AddNode(std::shared_ptr<Node>)
{
    return false;
}

void Node::SetCondition(std::shared_ptr<Condition> condition)
{
    condition_ = condition;
}

Node::Status Node::Execute(Agent& agent, uint32_t)
{
    if (condition_ && !condition_->Evaluate(agent, *this))
        return Status::CanNotExecute;
    return Status::Finished;
}

}

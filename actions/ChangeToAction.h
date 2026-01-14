//
// Created by jacob on 26-1-2.
//

#ifndef CHANGETALKACTION_H
#define CHANGETALKACTION_H

#include "../Agent.h"
#include "Action.h"
class ChangeToAction : public Action {
  Character::Act _act;
public:
  ChangeToAction(Character::Act act) : _act(act) {}

  Status tick(ActExecutorCtx& ctx, Blackboard& bb) override {
    ctx.parent_agent->getCharacter().setAct(_act);
    return Status::Success;
  }
};
#endif //CHANGETALKACTION_H

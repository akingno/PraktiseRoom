//
// Created by jacob on 26-1-13.
//

#ifndef SIGNALCHATACTION_H
#define SIGNALCHATACTION_H
#include "Action.h"
#include "../Agent.h"

class SignalChatAction : public Action {
public:
  Status tick(ActExecutorCtx &ctx, Blackboard &bb) override {
    if (!bb.target_agent) {
      return Status::Failure;
    }

    //check action of target agent, if it is sleeping, cancel this action
    if (bb.target_agent->getCharacter().isSleeping()) {
      return Status::Failure;
    }

    bb.target_agent->receiveCall(ctx.parent_agent);

   return Status::Success;
 }
};
#endif //SIGNALCHATACTION_H

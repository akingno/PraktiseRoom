//
// Created by jacob on 26-1-14.
//

#ifndef WAITFORCHATACTION_H
#define WAITFORCHATACTION_H
#include "Action.h"

class WaitForChatAction : public Action {
public:
  Status tick(ActExecutorCtx& ctx, Blackboard& bb) override {
    if (bb.is_being_called) {
      return Status::Running;
    }

    return Status::Success;
  }
};
#endif //WAITFORCHATACTION_H

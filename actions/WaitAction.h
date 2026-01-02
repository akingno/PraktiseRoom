//
// Created by jacob on 26-1-2.
//

#ifndef WAITACTION_H
#define WAITACTION_H

#include "Action.h"
class WaitAction : public Action {

  int _ticksRemaining;

public:
  explicit WaitAction(int ticks) : _ticksRemaining(ticks) {}

  Status tick(ActExecutorCtx& ctx, Blackboard& bb) override {

    _ticksRemaining--;
    if (_ticksRemaining <= 0) return Status::Success;
    return Status::Running;

  }

};
#endif //WAITACTION_H

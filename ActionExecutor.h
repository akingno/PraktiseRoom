//
// Created by jacob on 25-9-25.
//

#ifndef ACTIONEXECUTOR_H
#define ACTIONEXECUTOR_H

#include "Blackboard.h"
#include "Character.h"
#include "actions/Action.h"

#include <memory>

class ActionExecutor {

public:
  ActionExecutor() = default;
  void tick(ActExecutorCtx& ctx, Blackboard& bb);

};

#endif //ACTIONEXECUTOR_H

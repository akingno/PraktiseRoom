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
  std::shared_ptr<Action> _currentAction;
  Character::Act _lastActEnum = Character::Act::Wander; // 记录上一帧的决策，判断是否需要切换

public:
  void tick(Character::Act desiredAct, ActExecutorCtx& ctx, Blackboard& bb);

private:
  // 这里是硬编码的工厂，未来这部分会被 JSON 读取替代
  std::shared_ptr<Action> createActionChain(Character::Act act);
};

#endif //ACTIONEXECUTOR_H

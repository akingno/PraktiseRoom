//
// Created by jacob on 26-3-8.
//

#ifndef CROSSAGENTACTIONS_H
#define CROSSAGENTACTIONS_H
#include "Action.h"
#include "../Agent.h"
#include <spdlog/spdlog.h>
class TeleportTargetAction : public Action {
public:
  TeleportTargetAction(int level, int x, int y) : level_(level), x_(x), y_(y) {}

  Status tick(ActExecutorCtx& ctx, Blackboard& bb) override {
    if (!bb.target_agent) return Status::Failure;

    if (ctx.room.isPassable(x_, y_)) {
      bb.target_agent->getCharacter().setLoc(x_, y_);

      // 清空被传送者的动作队列
      bb.target_agent->clearActionQueue();

      spdlog::info("Teleported {} to ({}, {})", bb.target_agent->getName(), x_, y_);
    } else {
      spdlog::warn("Teleport failed: Destination ({}, {}) is blocked!", x_, y_);
    }
    return Status::Success;
  }
private:
  int level_, x_, y_;
};
#endif //CROSSAGENTACTIONS_H

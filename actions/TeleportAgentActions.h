//
// Created by jacob on 26-3-8.
//

#ifndef CROSSAGENTACTIONS_H
#define CROSSAGENTACTIONS_H
#include "../Agent.h"
#include "Action.h"
#include <spdlog/spdlog.h>
#include "../WorldManager.h"

class TeleportTargetAction : public Action {
public:
  TeleportTargetAction(int level, int x, int y, bool has_triggered) : level_(level), x_(x), y_(y), _has_triggered(has_triggered) {}

  Status tick(ActExecutorCtx& ctx, Blackboard& bb) override {
    spdlog::info("TeleportTarget: tick 启动！_has_triggered = {}", _has_triggered);

    if (_has_triggered) return Status::Success;

    if (!bb.target_agent) {
      spdlog::error("TeleportTarget: 致命错误，bb.target_agent 为 NULL！");
      return Status::Failure;
    }

    spdlog::info("TeleportTarget: 目标确认为 {}, 准备检测楼层 {}", bb.target_agent->getName(), level_);

    Level* dest_lvl = WorldManager::inst().getLevel(level_);
    if (!dest_lvl) {
      spdlog::warn("Teleport failed: Level {} 不存在！", level_);
      return Status::Success;
    }

    if (!dest_lvl->room->isPassable(x_, y_)) {
      spdlog::warn("Teleport failed: 目的地 ({}, {}) 被地形阻挡！", x_, y_);
      return Status::Success;
    }

    // 跨维跃迁
    bb.target_agent->getCharacter().setLevel(level_);
    bb.target_agent->getCharacter().setLoc(x_, y_);

    spdlog::info("TeleportTarget: 成功传送 {} 到 Level {} 坐标 ({}, {})", bb.target_agent->getName(), level_, x_, y_);

    if (bb.target_agent->getAIType() == AIType::Player) {
      WorldManager::inst().current_active_level = level_;
      spdlog::info("TeleportTarget: 镜头已自动跟随 Player 到 Level {}", level_);
    }

    _has_triggered = true;
    return Status::Success;
  }
private:
  int level_, x_, y_;
  bool _has_triggered;
};
#endif //CROSSAGENTACTIONS_H

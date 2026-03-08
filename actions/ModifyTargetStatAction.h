//
// Created by jacob on 26-3-8.
//

#ifndef MODIFYTARGETSTATACTION_H
#define MODIFYTARGETSTATACTION_H

#include "Action.h"
#include "../Agent.h"
#include <spdlog/spdlog.h>

class ModifyTargetStatAction : public Action {
public:
  ModifyTargetStatAction(std::string stat_name, float value) : stat_name_(std::move(stat_name)), value_(value) {}

  Status tick(ActExecutorCtx& ctx, Blackboard& bb) override {
    if (!bb.target_agent) return Status::Failure;

    // 数据化修改对方的属性
    bb.target_agent->getCharacter().modifyStat(stat_name_, value_);
    spdlog::info("Modified stat [{}] of {} by {}", stat_name_, bb.target_agent->getName(), value_);

    return Status::Success;
  }
private:
  std::string stat_name_;
  float value_;
};

#endif //MODIFYTARGETSTATACTION_H

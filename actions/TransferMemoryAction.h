//
// Created by jacob on 26-1-13.
//

#ifndef TRANSFERMEMORYACTION_H
#define TRANSFERMEMORYACTION_H
#include "Action.h"

class TransferMemoryAction : public Action {
public:
  Status tick(ActExecutorCtx& ctx, Blackboard& bb) override {
    if (!bb.target_agent) return Status::Failure;

    // 获取最后一条记忆
    auto& my_mems = ctx.ch.get_short_memory().entries();

    if (my_mems.empty()) {
      bb.target_agent->finishChat();
      return Status::Success;
    } // 没东西可聊

    std::string topic = my_mems.back().content;

    // 传给对方
    std::string msg = ctx.parent_agent->getName()+" said:" + topic;
    bb.target_agent->headMessage(msg);

    // 解除对方的等待状态
    bb.target_agent->finishChat();

    return Status::Success;
  }
};
#endif //TRANSFERMEMORYACTION_H

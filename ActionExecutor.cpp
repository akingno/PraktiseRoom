//
// Created by jacob on 25-9-25.
//
#include "ActionExecutor.h"


// ActionExecutor.cpp

// 确保包含所有 Action 的定义
#include "actions/ChangeToAction.h"
#include "actions/InteractAction.h"
#include "actions/MoveToAction.h"
#include "actions/SelectAgentAction.h"
#include "actions/SequenceAction.h"
#include "actions/SignalChatAction.h"
#include "actions/TransferMemoryAction.h"
#include "actions/WaitAction.h"
#include "actions/WaitForChatAction.h"

void ActionExecutor::tick(ActExecutorCtx& ctx, Blackboard& bb)
{
  /* 1. 若当前没有动作 → 从队列取一个 */
  if (bb.currentAction == nullptr) {
    std::lock_guard<std::mutex> lk(bb.queueMutex);
    if (!bb.actionQueue.empty()) {
      bb.currentAction = bb.actionQueue.front();
      bb.actionQueue.pop_front();
      bb.currentAction->onEnter(ctx, bb);
    }
  }

  /* 2. 执行 & 判断完成 */
  if (bb.currentAction) {
    auto st = bb.currentAction->tick(ctx, bb);
    if (st != Action::Status::Running) {
      bb.currentAction->onExit(ctx, bb);
      bb.currentAction = nullptr;
    }
  }
}












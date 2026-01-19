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

void ActionExecutor::tick(Character::Act desiredAct, ActExecutorCtx& ctx, Blackboard& bb) {

  // 切换逻辑：如果决策层改变了主意（比如突然饿了，打断闲逛）
  if (desiredAct != bb.lastActEnum || bb.currentAction == nullptr) {
    // 构建新的动作
    bb.currentAction = createActionChain(desiredAct);
    bb.currentAction->onEnter(ctx, bb);
    bb.lastActEnum = desiredAct;
  }

  // 执行逻辑
  if (bb.currentAction) {
    auto status = bb.currentAction->tick(ctx, bb);
    if (status != Action::Status::Running) {
      bb.currentAction->onExit(ctx, bb);
      bb.currentAction = nullptr; // 等待下一帧决策生成新的
    }
  }
}

std::shared_ptr<Action> ActionExecutor::createActionChain(Character::Act act) {
  auto seq = std::make_shared<SequenceAction>();

  switch (act) {
    case Character::Act::Talk:
      seq->add(std::make_shared<SelectAgentAction>());
      seq->add(std::make_shared<SignalChatAction>());
      seq->add(std::make_shared<MoveToAction>(TargetKind::Character));
      seq->add(std::make_shared<TransferMemoryAction>());
      seq->add(std::make_shared<WaitAction>(30));
      seq->add(std::make_shared<ChangeToAction>(Character::Act::Wander));
      break;
    case Character::Act::Eat:
      seq->add(std::make_shared<MoveToAction>(TargetKind::Food));
      seq->add(std::make_shared<InteractAction>());
      break;

    case Character::Act::Sleep:
      seq->add(std::make_shared<MoveToAction>(TargetKind::Bed));
      seq->add(std::make_shared<InteractAction>());
      // 睡眠可能不需要 WaitAction，因为 Character 状态变成了 Sleep，
      break;

    case Character::Act::UseComputer: {
      seq->add(std::make_shared<MoveToAction>(TargetKind::Computer));
      seq->add(std::make_shared<InteractAction>());
      // 随机时间
      int useTicks = AkRandom::randint(MIN_USE_COMPUTER_TIME, MAX_USE_COMPUTER_TIME) * TICKS_PER_SEC;
      seq->add(std::make_shared<WaitAction>(useTicks));
      break;
    }

    case Character::Act::Wander:
      // 闲逛 = 走到随机点 + 发呆一会
      seq->add(std::make_shared<MoveToAction>(TargetKind::WanderPt));
      if (AkRandom::bernoulli(CHANGE_ACTION_PROB)) {
        auto stopTicks = AkRandom::randint(MIN_STOP_TIME,MAX_STOP_TIME) * TICKS_PER_SEC;
        seq->add(std::make_shared<WaitAction>(stopTicks));
      }
      else if (AkRandom::bernoulli(CHANGE_TALK_PROB)) {
        seq->add(std::make_shared<ChangeToAction>(Character::Act::Talk));
      }
      break;

    case Character::Act::Stop:
      // 纯发呆
      seq->add(std::make_shared<WaitAction>(60));
      break;

    case Character::Act::WaitAlways:
      seq->add(std::make_shared<WaitForChatAction>());
      break;
  }
  return seq;
}










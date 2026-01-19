//
// Created by jacob on 26-1-19.
//

#include "ActionFactory.h"
#include "../Config.h"
#include "../tools/Random.h"
#include "ChangeToAction.h"
#include "InteractAction.h"
#include "MoveToAction.h"
#include "SelectAgentAction.h"
#include "SequenceAction.h"
#include "SignalChatAction.h"
#include "TransferMemoryAction.h"
#include "WaitAction.h"
#include "WaitForChatAction.h"

std::shared_ptr<Action> ActionFactory::createFromEnum(Character::Act act) {
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
      // 闲逛 = 走到随机点+发呆一会
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
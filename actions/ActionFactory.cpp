//
// Created by jacob on 26-1-19.
//

#include "ActionFactory.h"
#include "../Config.h"
#include "../tools/Random.h"
#include "ChangeToAction.h"
#include "InteractAction.h"
#include "ModifyTargetStatAction.h"
#include "MoveToAction.h"
#include "SelectAgentAction.h"
#include "SequenceAction.h"
#include "SignalChatAction.h"
#include "TeleportAgentActions.h"
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
    case Character::Act::Wander:
      // 闲逛 = 走到随机点+发呆一会
      seq->add(std::make_shared<MoveToAction>(TargetKind::WanderPt));
      if (AkRandom::bernoulli(Cfg::prob::change_action)) {
        auto stopTicks = AkRandom::randint(Cfg::time::min_stop, Cfg::time::max_stop) * Cfg::core::ticks_per_sec;
        seq->add(std::make_shared<WaitAction>(stopTicks));
      } else if (AkRandom::bernoulli(Cfg::prob::change_talk)) {
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
    case Character::Act::PlayerMove:
      seq->add(std::make_shared<MoveToAction>(TargetKind::Coordinate));
      break;
    case Character::Act::PlayerInteract:
      seq->add(std::make_shared<InteractAction>());
      break;
  }
  return seq;
}
std::shared_ptr<Action> ActionFactory::createFromSmartItem(const SmartItem *item) {
  return createFromDescriptors(item->getSequence());
}

std::shared_ptr<Action> ActionFactory::createFromDescriptors(const std::vector<ActionDescriptor> &seqDef) {
  auto seq = std::make_shared<SequenceAction>();

  for (const auto &desc : seqDef) {
    if (desc.name == "Interact") {
      seq->add(std::make_shared<InteractAction>());
    } else if (desc.name == "Wait") {
      seq->add(std::make_shared<WaitAction>(desc.intParam));
    } else if (desc.name == "MoveToTarget") {
      seq->add(std::make_shared<MoveToAction>(TargetKind::Coordinate));
    }
    else if (desc.name == "TeleportTarget") {
      // level: l, X, Y
      seq->add(std::make_shared<TeleportTargetAction>(static_cast<int>(desc.floatParam), desc.intParam, desc.intParam2, false));
    } else if (desc.name == "ModifyTargetStat") {
      // strParam: 属性名，floatParam: 数值
      seq->add(std::make_shared<ModifyTargetStatAction>(desc.strParam, desc.floatParam));
    }
  }
  return seq;
}
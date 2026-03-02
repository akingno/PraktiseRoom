//
// Created by jacob on 26-1-2.
//

#include "InteractAction.h"
#include "../ActionExecutor.h"
#include "../GameContentInit.h"
#include "../ItemRegistry.h"
#include "../SmartItem.h"
#include <iostream>

bool try_use_item_at(ActExecutorCtx &ctx, int x, int y) {
  auto iid = ctx.items.idAt(x, y);
  if (!iid) return false;
  if (Item *def = ItemRegistry::inst().get(*iid)) {
    if (auto *smartItem = dynamic_cast<SmartItem *>(def)) {
      for (const auto &effect : smartItem->getEffects()) {
        if (effect.type == EffectType::ModifyStat) {
          ctx.ch.modifyStat(effect.target, effect.value);
        } else if (effect.type == EffectType::SetState) {
          if (effect.target == "sleeping") {
            ctx.ch.setSleeping(effect.value > 0.0);
          }
        } else if (effect.type == EffectType::AddMemory) {
          // 如果是特殊的标记，随机抽一条
          if (effect.target == "$RANDOM_COMPUTER$") {
            int idx = AkRandom::randint(0, static_cast<int>(kComputerFeed.size()) - 1);
            ctx.ch.short_memory().add(kComputerFeed[idx]);
          } else {
            ctx.ch.short_memory().add(effect.target);
          }
        }
      }
      return true;
    } else [[unlikely]]{
      spdlog::warn("Interact Action: Impossible branch happen");
    }
  }
  return false;
}

Action::Status InteractAction::tick(ActExecutorCtx &ctx, Blackboard &bb) {
  auto pos = ctx.ch.getLoc();
  // 尝试使用脚下的物品
  if (try_use_item_at(ctx, pos.first, pos.second)) {
    return Status::Success;
  }
  return Status::Failure;
}

//
// Created by jacob on 26-1-2.
//

#include "InteractAction.h"

#include "../ActionExecutor.h"
#include "../ItemRegistry.h"

bool try_use_item_at(ActExecutorCtx& ctx, int x, int y) {
  auto iid = ctx.items.idAt(x,y);
  if (!iid) return false;
  if (Item* def = ItemRegistry::inst().get(*iid)) {

    UseCtx ux{ ctx.ch, ctx.room, ctx.items };
    return def->onUse(ux, x, y);
  }
  return false;
}

Action::Status InteractAction::tick(ActExecutorCtx& ctx, Blackboard& bb) {
  auto pos = ctx.ch.getLoc();
  // 尝试使用脚下的物品
  if (try_use_item_at(ctx, pos.first, pos.second)) {
    return Status::Success;
  }
  return Status::Failure;
}


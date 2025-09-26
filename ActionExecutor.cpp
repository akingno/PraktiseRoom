//
// Created by jacob on 25-9-25.
//
#include "ActionExecutor.h"

void ActionExecutor::tick(Character::Act act, ActExecutorCtx& ctx, Blackboard& bb){
  if (act == Character::Act::Eat) tick_eat(ctx, bb);
  else                            tick_wander(ctx);
}

void ActionExecutor::tick_eat(ActExecutorCtx& ctx, Blackboard& bb) {
  // 1) FindFood：写入 target
  if (!bb.target_valid) {
    if (!ctx.room.hasFood()) return; // 没食物：这一帧啥也不做（下帧 Utility 可能回 Wander）
    bb.target = { ctx.room.foodPos().x, ctx.room.foodPos().y };
    bb.target_valid = true;
    bb.path_invalid = true;
  }

  // 2) Plan（只在必要时重算；并做每帧一次的节流）
  const auto cur = ctx.ch.getLoc();
  if (need_replan(ctx, bb, cur, bb.target)) {
    bb.path.clear();
    // 防止一帧内多次 plan
    bb.last_planned_for_tick = ctx.tick_index;

    if (pf_.plan(cur.first, cur.second, bb.target.first, bb.target.second, bb.path)) {
      // 下标 0 是当前格，下一步应走 1
      bb.path_i = (int)bb.path.size() >= 2 ? 1 : (int)bb.path.size();
      bb.path_invalid = false;
    } else {
      // 找不到路：这帧放弃（可能被墙/门挡着）
      return;
    }
  }

  // 3) FollowPath：每帧只尝试迈一步
  if (!bb.path_invalid) {
    if (!follow_one_step(ctx, bb)) {
      // 被挡：标记失效，留到下帧重算
      bb.path_invalid = true;
      return;
    }
  }

  // 4) 到达则 EatNow（站在 F 上且无冷却）
  const auto pos  = ctx.ch.getLoc();
  const auto fpos = std::pair<int,int>{ ctx.room.foodPos().x, ctx.room.foodPos().y };
  if (ctx.room.hasFood() && pos == fpos && ctx.ch.eatAvailable()) {
    ctx.ch.eat(FOOD_CALORIES);
    ctx.room.consumeFood();
    // 本次 Eat 完成；清理目标（下次 Utility 可能切回 Wander）
    bb.target_valid = false;
    bb.path.clear();
    bb.path_i = 0;
    bb.path_invalid = true;
  }
}
bool ActionExecutor::need_replan(const ActExecutorCtx& ctx,
                                 const Blackboard& bb,
                                 std::pair<int,int> cur,
                                 std::pair<int,int> target) const {
  if (!bb.target_valid)            return false;
  if (bb.path_invalid)             return true;
  if (bb.path_i <= 0 || bb.path_i > (int)bb.path.size()) return true;
  // 目标改变 / 当前位置与路径不一致（例如被传送/打断）
  if (bb.path.empty() || bb.path.front()!=cur || bb.path.back()!=target) return true;
  // 本帧已经算过就别再算（节流）
  if (bb.last_planned_for_tick == ctx.tick_index) return false;
  return false;
}
bool ActionExecutor::follow_one_step(ActExecutorCtx& ctx, Blackboard& bb) {
  if (bb.path_i >= (int)bb.path.size()) return true; // 已到
  auto [nx,ny] = bb.path[bb.path_i];

  auto is_passable = [&](int x,int y){
    auto t = ctx.room.getBlocksType(x,y);
    return t != TileType::WallV && t != TileType::WallH;
  };

  if (ctx.ch.tryStepTo(nx,ny, is_passable)) {
    ++bb.path_i;
    return true;
  }
  return false; // 下一格被堵
}
void ActionExecutor::tick_wander(ActExecutorCtx& ctx) {
  auto is_passable = [&](int x,int y){
    auto t = ctx.room.getBlocksType(x,y);
    return t != TileType::WallV && t != TileType::WallH;
  };
  ctx.ch.tryMove(is_passable, Character::KEEP_LAST_DIR_PROB);
}

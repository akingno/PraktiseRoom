//
// Created by jacob on 25-9-25.
//
#include "ActionExecutor.h"

#include "ItemRegistry.h"
#include "RoomAdapters.h"

bool try_use_item_at(ActExecutorCtx& ctx, int x, int y) {
  auto t = ctx.room.getBlocksType(x, y);
  auto iid_opt = tile_to_item(t);
  if (!iid_opt) return false;

  if (Item* def = ItemRegistry::inst().get(*iid_opt)) {
    return def->onUse(ctx.ch, ctx.room, x, y);
  }
  return false;
}

bool ActionExecutor::plan_if_needed(ActExecutorCtx& ctx, Blackboard& bb, std::pair<int,int> cur) {
  if (!need_replan(ctx, bb, cur, bb.target)) return true;

  bb.path.clear();
  bb.mark_planned(ctx.tick_index);

  if (pf_.plan_path(cur.first, cur.second, bb.target.first, bb.target.second, bb.path)) {

    bb.init_path_after_planned();
    return true;
  }
  // 规划失败：留到下帧重试
  return false;
}

bool ActionExecutor::follow_one_step_or_invalidate(ActExecutorCtx& ctx, Blackboard& bb) {
  if (bb.path_invalid) return false;
  if (!follow_one_step(ctx, bb)) {
    bb.path_invalid = true;
    return false;
  }
  return true;
}


std::pair<int,int> ActionExecutor::pick_random_reachable(ActExecutorCtx& ctx,
                                                         std::pair<int,int> from,
                                                         int max_tries) {
  for (int t=0; t<max_tries; ++t) {
    int rx = Random::randint(1, VIEW_W - 2);
    int ry = Random::randint(1, VIEW_H - 2);

    // 路点必须可走（不是墙）
    auto tt = ctx.room.getBlocksType(rx, ry);
    if (tt == TileType::WallV || tt == TileType::WallH) continue;

    // 用 A* 验证可达（不保存路径，只做可达性检查）
    std::vector<std::pair<int,int>> tmp;
    if (pf_.plan_path(from.first, from.second, rx, ry, tmp)) {
      return {rx, ry};
    }
  }
  return {-1, -1}; // 尝试失败
}


std::pair<int,int> ActionExecutor::expected_target_for(TargetKind kind, const Room& room) const {
  switch (kind) {
    case TargetKind::Food: return {room.foodPos().x, room.foodPos().y};
    case TargetKind::Bed:  return {room.bedPos().x,  room.bedPos().y};
    case TargetKind::None:
    default:               return {-1,-1};
  }
}

void ActionExecutor::ensure_target(TargetKind need, ActExecutorCtx& ctx, Blackboard& bb) {

  // 世界可用性检查（Food/Bed 各自判断）
  bool available = false;
  switch (need) {
    case TargetKind::Food: available = ctx.room.hasFood(); break;
    case TargetKind::Bed:  available = ctx.room.hasBed();  break;
    default: break;
  }
  if (!available) {
    // 资源不可用，先把目标清空（上层可以选择本帧什么都不做）
    bb.clear_path_and_target_if_any();

    return;
  }
  // 期望目标位置
  const auto want = expected_target_for(need, ctx.room);
  const bool kind_mismatch   = (bb.target_kind != need);
  const bool target_mismatch = (!bb.target_valid || !same_pos(bb.target, want));

  if (kind_mismatch || target_mismatch) {
    // 覆盖为正确的目标，并重置路径
    bb.set_target_and_invalidate(need, want);
  }
}

void ActionExecutor::tick(Character::Act current_action, ActExecutorCtx& ctx, Blackboard& bb) {
  switch (current_action) {
    case Character::Act::Eat:
      tick_eat(ctx, bb);
      break;
    case Character::Act::Sleep:
      tick_sleep(ctx, bb);
      break;
    case Character::Act::Stop:
      tick_stop(ctx, bb);
      break;
    case Character::Act::Wander:
    default:
      tick_wander(ctx, bb);
      break;
  }
}

void ActionExecutor::tick_eat(ActExecutorCtx& ctx, Blackboard& bb) {
  if (ctx.ch.isSleeping()) ctx.ch.setSleeping(false);

  ensure_target(TargetKind::Food, ctx, bb);
  if (bb.target_kind != TargetKind::Food || !bb.target_valid) return;

  const auto cur = ctx.ch.getLoc();
  if (!plan_if_needed(ctx, bb, cur)) return;

  // 路径仍然合法， 试图走一步
  follow_one_step_or_invalidate(ctx, bb);

  // 如果到达则开吃
  const auto pos  = ctx.ch.getLoc();
  const auto fpos = expected_target_for(TargetKind::Food, ctx.room);
  if (same_pos(pos, fpos)) {
    if (try_use_item_at(ctx, pos.first, pos.second)) {
      bb.clear_path_and_target();   // ← 仅清理；效果在 onUse 里
      return;
    }
    // 使用失败（例如瞬间被移除），让下帧重算
    bb.path_invalid = true;
  }
}

// Sleep 行为：FindBed → Plan → Follow → SleepNow
void ActionExecutor::tick_sleep(ActExecutorCtx& ctx, Blackboard& bb) {
  ensure_target(TargetKind::Bed, ctx, bb);
  if (bb.target_kind != TargetKind::Bed || !bb.target_valid) return;

  const auto cur = ctx.ch.getLoc();
  if (!plan_if_needed(ctx, bb, cur)) return;

  // 路径仍然合法， 试图走一步
  follow_one_step_or_invalidate(ctx, bb);

  // 如果到达则开睡
  const auto pos  = ctx.ch.getLoc();
  const auto bpos = expected_target_for(TargetKind::Bed, ctx.room);
  if (same_pos(pos, bpos)) {
    if (try_use_item_at(ctx, pos.first, pos.second)) {
      bb.clear_path_and_target();
      return;
    }
    bb.path_invalid = true;
  } else { // 还在路上，不在床上
    if (ctx.ch.isSleeping()) ctx.ch.setSleeping(false);
  }
}

bool ActionExecutor::need_replan(const ActExecutorCtx& ctx,
                                 const Blackboard& bb,
                                 std::pair<int,int> current_loc,
                                 std::pair<int,int> target_loc) const {

  if (!bb.target_valid) return false;
  if (bb.path_invalid) return true;

  if (bb.path_i <= 0 || bb.path_i > (int)bb.path.size()) {
    //说明路径为空或者路径目标在路径外
    return true;
  }
  // 目标改变 / 当前位置与路径不一致（例如被传送/打断）
  if (bb.path.empty() || bb.path.front()!=current_loc || bb.path.back()!=target_loc) {
    return true;
  }
  // 本帧已经算过就别再算（节流）
  if (bb.last_planned_for_tick == ctx.tick_index) return false;

  return false;
}


bool ActionExecutor::follow_one_step(ActExecutorCtx& ctx, Blackboard& bb) {
  if (bb.path_i >= (int)bb.path.size()) return true; // 已到

  auto [nx,ny] = bb.path[bb.path_i];

  if (!ctx.room.isPassable(nx, ny)) return false;

  if (!ctx.ch.tryStepTo(nx, ny)) return false;

  // 说明可以走
  ++bb.path_i;
  return true;
}

void ActionExecutor::tick_wander(ActExecutorCtx& ctx, Blackboard& bb) {
  if (ctx.ch.isSleeping()) ctx.ch.setSleeping(false);

  const auto cur = ctx.ch.getLoc();

  // 1) 若黑板不是 WanderPt 或目标无效/已到达，则挑新点
  const bool wrong_kind  = (bb.target_kind != TargetKind::WanderPt);
  const bool need_new    = wrong_kind || !bb.target_valid || same_pos(cur, bb.target);

  if (need_new) {
    auto pick = pick_random_reachable(ctx, cur, 20);
    if (pick.first != -1) {
      bb.set_target_and_invalidate(TargetKind::WanderPt, pick);
    }
  }

  // 2) 需要则规划
  if (!plan_if_needed(ctx, bb, cur)) {
    // 规划失败：清空，下一帧重新挑点
    bb.clear_path_and_target();
    return;
  }

  // 3) 沿路径走一步
  follow_one_step_or_invalidate(ctx, bb);

  // 4) 到达则让下一帧重新挑点（不在当前帧挑，避免一帧内多次寻路）
  if (same_pos(ctx.ch.getLoc(), bb.target) || bb.path_i >= (int)bb.path.size()) {
    bb.clear_path_and_target();

    // 到达后有几率进入stop
    const bool want_stop = Random::bernoulli(ENTER_STOP_POSSI);
    if (want_stop) {
      const int hold_ticks = Random::randint(MIN_STOP_TIME * TICKS_PER_SEC, MAX_STOP_TIME * TICKS_PER_SEC); // 50ms *
      bb.start_stop_until(ctx.tick_index, hold_ticks);
    }
  }

}

void ActionExecutor::tick_stop(ActExecutorCtx& ctx, Blackboard& bb) {
  if (ctx.ch.isSleeping()) ctx.ch.setSleeping(false);
  // Stop 不需要目标：确保黑板无目标、无路径（避免残留导致意外移动）
  bb.clear_path_and_target_if_any();
  // 不移动
}



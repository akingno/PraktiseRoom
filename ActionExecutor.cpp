//
// Created by jacob on 25-9-25.
//
#include "ActionExecutor.h"

// ========= 目标类型化实现 =========
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
    if (bb.target_kind != TargetKind::None || bb.target_valid) {
      bb.target = {-1,-1};
      bb.target_valid = false;
      bb.target_kind  = TargetKind::None;
      bb.path.clear();
      bb.path_i = 0;
      bb.path_invalid = true;
    }
    return;
  }
  // 期望目标位置
  const auto want = expected_target_for(need, ctx.room);
  const bool kind_mismatch   = (bb.target_kind != need);
  const bool target_mismatch = (!bb.target_valid || !same_pos(bb.target, want));

  if (kind_mismatch || target_mismatch) {
    // 覆盖为正确的目标，并重置路径
    bb.target       = want;
    bb.target_valid = true;
    bb.target_kind  = need;
    bb.path.clear();
    bb.path_i = 0;
    bb.path_invalid = true;
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
    case Character::Act::Wander:
    default:
      tick_wander(ctx);
      break;
  }
}

void ActionExecutor::tick_eat(ActExecutorCtx& ctx, Blackboard& bb) {
  if (ctx.ch.isSleeping()) {
    ctx.ch.setSleeping(false);
  }

  // 1) 确认有食物存在，设定食物为target
  ensure_target(TargetKind::Food, ctx, bb);
  if (bb.target_kind != TargetKind::Food || !bb.target_valid) {
    // 没有食物可用，本帧不动
    return;
  }

  // 2) 如果需要重算，则重算一次路径
  const auto current_loc = ctx.ch.getLoc();
  if (need_replan(ctx, bb, current_loc, bb.target)) {
    bb.path.clear();
    // 防止一帧内多次 plan
    bb.last_planned_for_tick = ctx.tick_index;

    if (pf_.plan(current_loc.first, current_loc.second, bb.target.first, bb.target.second, bb.path)) {
      // 下标 0 是当前格，如果路径里有下一格，则从 1 开始走；否则已经在目标（size=0/1）
      if ((int)bb.path.size() >= 2) {
        bb.path_i = 1; //走
      }
      else {
        bb.path_i =  (int)bb.path.size();
      }

      bb.path_invalid = false;
    } else {
      // 找不到路：这帧放弃（可能被墙/门挡着）
      return;
    }
  }

  // 3) FollowPath：每帧只尝试移动一步
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
    bb.target_kind  = TargetKind::None;
    bb.path.clear();
    bb.path_i = 0;
    bb.path_invalid = true;
  }
}

// Sleep 行为：FindBed → Plan → Follow → SleepNow
void ActionExecutor::tick_sleep(ActExecutorCtx& ctx, Blackboard& bb) {

  ensure_target(TargetKind::Bed, ctx, bb);
  if (bb.target_kind != TargetKind::Bed || !bb.target_valid) {
    // 没床（或不可用），本帧不动
    return;
  }

  // 2) 需要则重算路径
  const auto cur = ctx.ch.getLoc();
  if (need_replan(ctx, bb, cur, bb.target)) {
    bb.path.clear();
    bb.last_planned_for_tick = ctx.tick_index;

    if (pf_.plan(cur.first, cur.second, bb.target.first, bb.target.second, bb.path)) {
      bb.path_i = (bb.path.size() > 1) ? 1 : static_cast<int>(bb.path.size());
      bb.path_invalid = false;
    } else {
      return;
    }
  }

  // 3) 跟随路径（到床之前正常行走）
  if (!bb.path_invalid) {
    if (!follow_one_step(ctx, bb)) {
      bb.path_invalid = true;
      return;
    }
  }

  // 4) 到达床位 → 进入睡眠：不移动，交给 tickNeeds 持续恢复疲劳
  const auto pos  = ctx.ch.getLoc();
  const auto bpos = std::pair<int,int>{ ctx.room.bedPos().x, ctx.room.bedPos().y };

  if (pos == bpos) {
    // 在床上：置为睡眠；不清理 target（保持粘性，直到 Utility 切走）
    if (!ctx.ch.isSleeping()) ctx.ch.setSleeping(true);
    // 这里不做其它移动；当主循环里 scoreSleep 降到比其他更低时，会切出 Sleep
  } else {
    // 还在路上：确保不是“正在睡觉”状态
    if (ctx.ch.isSleeping()) ctx.ch.setSleeping(false);
  }
}

bool ActionExecutor::need_replan(const ActExecutorCtx& ctx,
                                 const Blackboard& bb,
                                 std::pair<int,int> current_loc,
                                 std::pair<int,int> target_loc) const {
  if (!bb.target_valid)            return false;
  if (bb.path_invalid)             return true;
  if (bb.path_i <= 0 || bb.path_i > (int)bb.path.size()) return true;
  // 目标改变 / 当前位置与路径不一致（例如被传送/打断）
  if (bb.path.empty() || bb.path.front()!=current_loc || bb.path.back()!=target_loc) return true;
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
  if (ctx.ch.isSleeping()) {
    ctx.ch.setSleeping(false);
  }
  auto is_passable = [&](int x,int y){
    auto t = ctx.room.getBlocksType(x,y);
    return t != TileType::WallV && t != TileType::WallH;
  };
  ctx.ch.tryMove(is_passable, Character::KEEP_LAST_DIR_PROB);
}

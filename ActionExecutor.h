//
// Created by jacob on 25-9-25.
//

#ifndef ACTIONEXECUTOR_H
#define ACTIONEXECUTOR_H

#include "Blackboard.h"
#include "IPathfinder.h"
#include "Room.h"
#include "Character.h"

const int FOOD_CALORIES = 80;

// 本帧的资源上下文
struct ActExecutorCtx {
  Room& room;
  Character& ch;
  uint64_t tick_index;          // 帧号（可用于节流）
};

class ActionExecutor {
public:
  ActionExecutor(IPathfinder& pf) : pf_(pf) {}

  // 外部只需要调用这一句
  void tick(Character::Act current_action, ActExecutorCtx& ctx, Blackboard& bb);

private:
  // Eat：Find→(Plan if needed)→FollowStep→EatNow
  void tick_eat(ActExecutorCtx& ctx, Blackboard& bb);
  void tick_wander(ActExecutorCtx& ctx);
  void tick_sleep(ActExecutorCtx& ctx, Blackboard& bb);

  void ensure_target(TargetKind need, ActExecutorCtx& ctx, Blackboard& bb);

  // 返回某种目标在世界中的“期望坐标”
  std::pair<int,int> expected_target_for(TargetKind kind, const Room& room) const;

  static bool same_pos(std::pair<int,int> a, std::pair<int,int> b) {
    return a.first==b.first && a.second==b.second;
  }
  // 工具
  bool need_replan(const ActExecutorCtx& ctx, const Blackboard& bb,
                   std::pair<int,int> current_loc, std::pair<int,int> target_loc) const;
  bool follow_one_step(ActExecutorCtx& ctx, Blackboard& bb);

private:
  IPathfinder& pf_;
};

#endif //ACTIONEXECUTOR_H

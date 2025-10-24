//
// Created by jacob on 25-9-25.
//

#ifndef ACTIONEXECUTOR_H
#define ACTIONEXECUTOR_H

#include "Blackboard.h"
#include "IPathfinder.h"
#include "Room.h"
#include "Character.h"



// 本帧的资源上下文
struct ActExecutorCtx {
  Room& room;
  Character& ch;
  uint64_t tick_index;          // 帧号（可用于节流）
  IPathfinder& pf;
};

class ActionExecutor {
public:
  ActionExecutor(IPathfinder& pf) : pf_(pf) {}

  // 外部调用使用，只需要调用这一句
  void tick(Character::Act current_action, ActExecutorCtx& ctx, Blackboard& bb);

private:
  // Eat：Find→(Plan if needed)→FollowStep→EatNow
  void tick_eat(ActExecutorCtx& ctx, Blackboard& bb);
  void tick_sleep(ActExecutorCtx& ctx, Blackboard& bb);
  void tick_wander(ActExecutorCtx& ctx, Blackboard& bb);
  void tick_stop(ActExecutorCtx& ctx, Blackboard& bb);

  // 确保目标存在并且在那个位置
  void ensure_target(TargetKind need, ActExecutorCtx& ctx, Blackboard& bb);

  // 返回某种目标在世界中的“期望坐标”
  std::pair<int,int> expected_target_for(TargetKind kind, const Room& room) const;


  // Wander 用：挑一个随机的可达点（带 A* 验证），失败返回 {-1,-1}
  std::pair<int,int> pick_random_reachable(ActExecutorCtx& ctx,
                                            std::pair<int,int> from,
                                            int max_tries = 20);

  // 工具： 判断是否是一个位置
  static bool same_pos(std::pair<int,int> a, std::pair<int,int> b) {
    return a.first==b.first && a.second==b.second;
  }
  // 工具：判断是否路径出现变化或者目标出现变化
  bool need_replan(const ActExecutorCtx& ctx, const Blackboard& bb,
                   std::pair<int,int> current_loc, std::pair<int,int> target_loc) const;

  // 重新规划路径
  bool plan_if_needed(ActExecutorCtx& ctx, Blackboard& bb, std::pair<int,int> cur);

  // 判断路径是否正确， 是则调用 走一步
  bool follow_one_step_or_invalidate(ActExecutorCtx& ctx, Blackboard& bb);

  // 走一步
  bool follow_one_step(ActExecutorCtx& ctx, Blackboard& bb);

private:
  IPathfinder& pf_;
};

#endif //ACTIONEXECUTOR_H

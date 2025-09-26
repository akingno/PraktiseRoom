//
// Created by jacob on 25-9-25.
//

#ifndef ACTIONEXECUTOR_H
#define ACTIONEXECUTOR_H

#include "Blackboard.h"
#include "IPathfinder.h"
#include "Room.h"
#include "Character.h"

const int FOOD_CALORIES = 70;

struct ActExecutorCtx {
  Room& room;
  Character& ch;
  int tick_index;          // 帧号（可用于节流）
};

class ActionExecutor {
public:
  ActionExecutor(IPathfinder& pf) : pf_(pf) {}

  // 外部只需要调用这一句
  void tick(Character::Act act, ActExecutorCtx& ctx, Blackboard& bb);

private:
  // Eat：Find→(Plan if needed)→FollowStep→EatNow
  void tick_eat(ActExecutorCtx& ctx, Blackboard& bb);
  void tick_wander(ActExecutorCtx& ctx);

  // 工具
  bool need_replan(const ActExecutorCtx& ctx, const Blackboard& bb,
                   std::pair<int,int> cur, std::pair<int,int> target) const;
  bool follow_one_step(ActExecutorCtx& ctx, Blackboard& bb);

private:
  IPathfinder& pf_;
};

#endif //ACTIONEXECUTOR_H

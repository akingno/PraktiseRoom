//
// Created by jacob on 26-1-2.
//

#ifndef ACTION_H
#define ACTION_H

#include "../Blackboard.h"
#include "../Room.h"
#include "../tools/IPathfinder.h"
#include "../ItemLayer.h"

class Agent;

// 本帧的资源上下文
struct ActExecutorCtx {
  Room& room;
  Character& ch;
  uint64_t tick_index; // 帧号（用于节流）
  IPathfinder& pf;
  ItemLayer& items;
  Agent* parent_agent;
};

class Action {
public:
  enum class Status { Running, Success, Failure };

  virtual ~Action() = default;

  virtual void onEnter(ActExecutorCtx& ctx, Blackboard& bb){}

  virtual Status tick(ActExecutorCtx& ctx, Blackboard& bb) = 0;

  virtual void onExit(ActExecutorCtx& ctx, Blackboard& bb){}


};




#endif //ACTION_H

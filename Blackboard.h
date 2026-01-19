//
// Created by jacob on 25-9-25.
//

#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include "Character.h"
#include <deque>
#include <mutex>
#include <atomic>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

class Agent;
class Action;
enum class TargetKind { None, Food, Bed, WanderPt, Computer, Character};

struct Blackboard {
  Blackboard() = default;
  // 目标
  std::pair<int,int> target{-1,-1};
  bool target_valid = false;
  TargetKind target_kind = TargetKind::None;
  // 小人之间交互
  Agent* target_agent = nullptr;
  bool is_being_called = false;
  Agent* caller_agent = nullptr;

  // 路径：从起点到终点（含自己与终点）
  std::vector<std::pair<int,int>> path;
  int path_i = 0;            // 下一步要走到的下标（一般从 1 开始）
  bool path_invalid = true;  // 需要重算
  bool _using_computer = false;

  //当前的动作

  uint64_t last_planned_for_tick = -999999; // 本帧已算过就不再算

  // action 生产消费相关
  std::deque<std::shared_ptr<Action>> actionQueue;
  std::mutex                          queueMutex;
  std::shared_ptr<Action>             currentAction = nullptr;  // 正在执行的action
  std::atomic<bool>                   is_thinking{false};     // Brain 是否在正在决策
  Character::Act                      lastActEnum = Character::Act::Wander;


  // 辅助工具，用于路径操作
  // 让stop action持续到这个 tick（包含当前tick）<0 表示不在 Stop
  long long stop_until_tick = -1; //TODO: 是否可以删除？

  bool is_using_computer() const {
    return _using_computer;
  }
  // 是否处于 Stop
  [[nodiscard]] bool in_stop(uint64_t now_tick) const {
    return stop_until_tick >= 0 && static_cast<long long>(now_tick) <= stop_until_tick;
  }
  // 仅清路径（保持 target 不变）
  void clear_path() {
    path.clear();
    path_i = 0;
    path_invalid = true;
  }
  // 规划后初始化 path_i，并把路径标记为合法
  void init_path_after_planned() {
    // path[0] = 当前格；若有下一格，从 1 走起；否则 size==0/1 已在终点
    path_i = (path.size() > 1) ? 1 : static_cast<int>(path.size());
    path_invalid = false;
  }

};

#endif //BLACKBOARD_H

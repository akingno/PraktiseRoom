//
// Created by jacob on 25-9-25.
//

#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include "Character.h"

#include <iostream>
#include <memory>
#include <utility>
#include <vector>

class Agent;
class Action;
enum class TargetKind { None, Food, Bed, WanderPt, Computer, Character};

struct Blackboard {
  // 目标（食物或者床的位置）
  std::pair<int,int> target{-1,-1};
  bool target_valid = false;
  TargetKind target_kind = TargetKind::None;

  Agent* target_agent = nullptr;
  bool is_being_called = false;
  Agent* caller_agent = nullptr;

  // 路径：从起点到终点（含自己与终点）
  std::vector<std::pair<int,int>> path;
  int path_i = 0;            // 下一步要走到的下标（一般从 1 开始）
  bool path_invalid = true;  // 需要重算
  bool _using_computer = false;

  //当前的动作
  std::shared_ptr<Action> currentAction = nullptr;
  Character::Act lastActEnum = Character::Act::Wander;

  // 让 Stop 持续到这个 tick（包含）<0 表示不在 Stop
  bool is_using_computer() const {
    return _using_computer;
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

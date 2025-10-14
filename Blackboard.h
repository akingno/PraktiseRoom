//
// Created by jacob on 25-9-25.
//

#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <cstdint>
#include <utility>
#include <vector>

enum class TargetKind { None, Food, Bed };

struct Blackboard {
  // 目标（食物或者床的位置）
  std::pair<int,int> target{-1,-1};
  bool target_valid = false;
  TargetKind target_kind = TargetKind::None;

  // 路径：从起点到终点（含自己与终点）
  std::vector<std::pair<int,int>> path;
  int path_i = 0;            // 下一步要走到的下标（一般从 1 开始）
  bool path_invalid = true;  // 需要重算

  // 诊断/节流
  uint64_t last_planned_for_tick = -999999; // 本帧已算过就不再算
};

#endif //BLACKBOARD_H

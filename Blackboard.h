//
// Created by jacob on 25-9-25.
//

#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

enum class TargetKind { None, Food, Bed, WanderPt, Computer };

struct Blackboard {
  // 目标（食物或者床的位置）
  std::pair<int,int> target{-1,-1};
  bool target_valid = false;
  TargetKind target_kind = TargetKind::None;

  // 路径：从起点到终点（含自己与终点）
  std::vector<std::pair<int,int>> path;
  int path_i = 0;            // 下一步要走到的下标（一般从 1 开始）
  bool path_invalid = true;  // 需要重算
  bool _using_computer = false;

  // 诊断/节流
  uint64_t last_planned_for_tick = -999999; // 本帧已算过就不再算

  // 让 Stop 持续到这个 tick（包含）；<0 表示不在 Stop
  long long stop_until_tick = -1;
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

  // 仅清目标（保持路径不变——一般也会清路径，见 clear_nav）
  void clear_target() {
    target = {-1,-1};
    target_valid = false;
    target_kind = TargetKind::None;
  }

  // 同时清目标与路径（最常用的“归零”操作）
  void clear_path_and_target() {
    clear_target();
    clear_path();
  }

  // 设置目标并使路径无效（切换目标时调用）
  void set_target_and_invalidate(TargetKind kind, std::pair<int,int> pos) {
    target = pos;
    target_valid = true;
    target_kind = kind;
    clear_path(); // 切目标必然需要重算路径
  }

  // 规划后初始化 path_i，并把路径标记为合法
  void init_path_after_planned() {
    // path[0] = 当前格；若有下一格，从 1 走起；否则 size==0/1 已在终点
    path_i = (path.size() > 1) ? 1 : static_cast<int>(path.size());
    path_invalid = false;
  }

  // 标记“本帧已规划”（节流）
  void mark_planned(uint64_t tick) {
    last_planned_for_tick = tick;
  }

  // 停止一段时间
  void start_stop_until(uint64_t now_tick, int hold_ticks) {
    stop_until_tick = static_cast<long long>(now_tick) + hold_ticks;
  }
  void computer_used() {
      std::cout<<"使用完毕"<<std::endl;
    _using_computer = false;
  }

  // 若当前有任何目标/路径，则清空（避免无谓写入）
  void clear_path_and_target_if_any() {
    if (target_kind != TargetKind::None || target_valid || !path.empty()) {
      clear_path_and_target();
    }
  }
};

#endif //BLACKBOARD_H

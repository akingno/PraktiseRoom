//
// Created by jacob on 2025/9/11.
//

#ifndef ROOM_TEMP__CHARACTER_H_
#define ROOM_TEMP__CHARACTER_H_

#include <random>
#include <utility>
#include "Directions.h"


class Character {

public:
  enum class Act {
    Eat,
    Wander
  };

 //构造函数
  Character() : _loc(1,1) {
    int dir = Random::randint(0,MAX_DIR);
    _last_dir = Int2Dir(dir);
  }

  // 关于饥饿和进食的计算
  [[nodiscard]] double  get_hunger_score() const { return _hunger; }
  void tickNeeds(double dt_sec) {
    _hunger = std::min(100.0, _hunger + _hunger_rate * dt_sec);
    if (_eat_cooldown > 0.0) _eat_cooldown = std::max(0.0, _eat_cooldown - dt_sec);
  }
  bool eatAvailable() const { return _eat_cooldown <= 0.0; }
  void eat(int calories) {                        // 简化：卡路里直接换成饱腹度
    _hunger = std::max(0.0, _hunger - calories);
    _eat_cooldown = _eat_cooldown_secs;
  }
  template<class IsPassable>
  bool tryStepTo(int nx,int ny, IsPassable&& passable) {
    auto [x,y] = _loc;
    if (std::abs(nx-x)+std::abs(ny-y)!=1) return false; // 只允许 4 邻接
    if (!passable(nx,ny)) return false;
    _loc = {nx,ny};  // 原子移动
    _last_dir = (nx>x) ? Dir::Right : (nx<x) ? Dir::Left : (ny>y) ? Dir::Down : Dir::Up;
    return true;
  }

  void  setAct(Act a) { act_ = a; }
  Act   act() const { return act_; }
  static const char* Act2Str(const Act a) {
    switch (a) {
      case Act::Eat:
        return "Eat";
      case Act::Wander:
        return "Wander";
      default:
        return "Unknown";
    }
  }

  [[nodiscard]] std::pair<int, int> getLoc() const{return _loc; }
  [[nodiscard]] Dir getLastDir() const{return _last_dir; }


  /*
   *
   * 尝试随机从四个方向中移动
   */
  template<class IsPassable>
  bool tryMove(IsPassable&& is_passable, double keep_prob = KEEP_LAST_DIR_PROB){
    Dir order[5];

    order[0] = pick_biased_dir(_last_dir, keep_prob);
    order[1] = opposite(order[0]);

    int oi=2;
    for (Dir d : {Dir::Right, Dir::Left, Dir::Down, Dir::Up, Dir::Stay}) {
      if (d!=order[0] && d!=order[1]) order[oi++]=d;
    }

    for (Dir d : order) {
      auto [dx,dy] = dir_vec(d);
      int nx = _loc.first + dx;
      int ny = _loc.second + dy;
      if (is_passable(nx, ny)) { _loc = {nx,ny}; _last_dir = d; return true; }
    }
    return false; // 四个方向都不行
  }

 private:
  Dir pick_biased_dir(Dir last, double keep_prob = KEEP_LAST_DIR_PROB) {
    if (Random::bernoulli(keep_prob)) return last;
    return _dir_weights.random_dir_weights();
  }

  std::pair<int, int> _loc;
  Dir _last_dir;
  DirWeights _dir_weights;
  Act act_ = Act::Wander;

  double _hunger = 20.0;          // 0=饱 100=极饿，起点 = 30
  double _hunger_rate = 2;      // 每秒 +1
  double _eat_cooldown = 0.0;   // 当前冷却剩余秒
  double _eat_cooldown_secs = 1.0;

 public:
  constexpr static double KEEP_LAST_DIR_PROB = 0.9;

};

#endif//ROOM_TEMP__CHARACTER_H_

//
// Created by jacob on 2025/9/11.
//

#ifndef ROOM_TEMP__CHARACTER_H_
#define ROOM_TEMP__CHARACTER_H_

#include "Random.h"
#include <random>
#include <utility>

constexpr int MAX_DIR = 4;

enum class Dir{
  Right = 0,
  Left = 1,
  Down = 2,
  Up = 3,
  Stay = 4
};



struct DirWeights{
 const int right = 1, left = 1, down = 1, up = 1, stay = 1;
 public:
  [[nodiscard]] int sum() const{
    return right + left + down + up + stay;
  }
  [[nodiscard]] Dir random_dir_weights() const{
    double r = Random::rand01() * sum();
    if((r -= right) < 0 ) return Dir::Right;
    if((r -= left) < 0 ) return Dir::Left;
    if((r -= down) < 0 ) return Dir::Down;
    if((r -= stay) < 0) return Dir::Stay;
    return Dir::Stay;
  }
};

static std::pair<int, int> dir_vec(Dir d){
  switch (d) {
    case Dir::Right: return {+1, 0};
    case Dir::Left: return  {-1, 0};
    case Dir::Down: return  {0,+1};
    case Dir::Up:   return  {0, -1};
    case Dir::Stay:   return  {0, 0};
  }
  return {0, 0};
}

static Dir opposite(Dir d){
  switch (d) {
    case Dir::Right:return  Dir::Left;
    case Dir::Left: return  Dir::Right;
    case Dir::Down: return  Dir::Up;
    case Dir::Up:   return  Dir::Down;
    case Dir::Stay: return  Dir::Stay;
  }
  return d;
}

class Character {

public:
  enum class Act {
    Eat,
    Wander
  };

 public:
  static Dir Int2Dir(int num) {
    switch (num) {
      case 0:
        return Dir::Right;
      case 1:
        return Dir::Left;
      case 2:
        return Dir::Down;
      case 3:
        return Dir::Up;
      case 4:
        return Dir::Stay;
      default:
        return Dir::Stay;
    }
  }

  static std::string Dir2Str(Dir dir) {
    switch (dir) {
      case Dir::Right:
        return "Right";
      case Dir::Left:
        return "Left";
      case Dir::Down:
        return "Down";
      case Dir::Up:
        return "Up";
      case Dir::Stay:
        return "Stay";
      default:
        return "Unknown";
    }
  }
  //构造函数
  Character() : _loc(1,1) {
    int dir = Random::randint(0,MAX_DIR);
    _last_dir = Int2Dir(dir);
  }

  // 关于饥饿和进食的计算
  int  hunger() const { return _hunger; }
  void tickNeeds(double dt_sec) {
    _hunger = std::min(100.0, _hunger + _hunger_rate * dt_sec);
    if (_eat_cooldown > 0.0) _eat_cooldown = std::max(0.0, _eat_cooldown - dt_sec);
  }
  bool eatAvailable() const { return _eat_cooldown <= 0.0; }
  void eat(int calories) {                        // 简化：卡路里直接换成饱腹度
    _hunger = std::max(0.0, _hunger - calories);
    _eat_cooldown = _eat_cooldown_secs;
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

 private:
  std::pair<int, int> _loc;
  Dir _last_dir;
  DirWeights _dir_weights;

  double _hunger = 30.0;          // 0=饱 100=极饿，起点 = 30
  double _hunger_rate = 1.0;      // 每秒 +1
  double _eat_cooldown = 0.0;   // 当前冷却剩余秒
  double _eat_cooldown_secs = 1.0;

  Act act_ = Act::Wander;
 public:
  constexpr static double KEEP_LAST_DIR_PROB = 0.9;

};

#endif//ROOM_TEMP__CHARACTER_H_

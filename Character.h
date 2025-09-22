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
    last_dir_ = Int2Dir(dir);
  }

  [[nodiscard]] std::pair<int, int> getLoc() const{return _loc; }
  [[nodiscard]] Dir getLastDir() const{return last_dir_; }


  template<class IsPassable>
  bool tryMove(IsPassable&& is_passable, double keep_prob = KEEP_LAST_DIR_PROB){
    Dir order[5];

    order[0] = pick_biased_dir(last_dir_, keep_prob);
    order[1] = opposite(order[0]);

    int oi=2;
    for (Dir d : {Dir::Right, Dir::Left, Dir::Down, Dir::Up, Dir::Stay}) {
      if (d!=order[0] && d!=order[1]) order[oi++]=d;
    }

    for (Dir d : order) {
      auto [dx,dy] = dir_vec(d);
      int nx = _loc.first + dx;
      int ny = _loc.second + dy;
      if (is_passable(nx, ny)) { _loc = {nx,ny}; last_dir_ = d; return true; }
    }
    return false; // 四个方向都不行
  }

 private:
  Dir pick_biased_dir(Dir last, double keep_prob = KEEP_LAST_DIR_PROB) {
    if (Random::bernoulli(keep_prob)) return last;

    return weights.random_dir_weights();

  }

 private:
  std::pair<int, int> _loc;
  Dir last_dir_;
  DirWeights weights;
 public:
  constexpr static double KEEP_LAST_DIR_PROB = 0.9;

};

#endif//ROOM_TEMP__CHARACTER_H_

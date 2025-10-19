//
// Created by jacob on 25-9-21.
//

#ifndef UTILS_H
#define UTILS_H

// 判断房间坐标(x,y)是否可走
inline bool is_passable(const Room& room, int x, int y) {
  auto t = room.getBlocksType(x, y);
  // 可通行：不是墙
  return t != TileType::WallV && t != TileType::WallH;
}

inline double clamp01(double x){ return x<0?0:(x>1?1:x); }


// 目前无用
inline int manhattan(std::pair<int,int> a, std::pair<int,int> b){
  return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

/*
 *
 * 关于hunger的计算：线性增长的desire和 计算得到的Score
 *
 */
inline double desire_from_hunger(double hunger, double enter){
  if (hunger <= enter) return 0.0;
  return clamp01( (hunger - enter) / (100 - enter) );
}

// scoreEat 计算
inline double CalcScoreEat(
    double hunger,
    bool hasFood,
    bool onCooldown,
    bool stickyEatActive,
    double hungryEnter,
    double sticky   = 0.2
){
  if (!hasFood) return 0.0;
  if (onCooldown) return 0.0; // 冷却直接禁止

  const double desire = desire_from_hunger(hunger, hungryEnter);
  const double base = desire * 0.833;

  return stickyEatActive ? (base + sticky) : base;
}


/*
 *
 * 关于fatigue的计算：线性增长的desire 和计算得到的socre
 *
 */
inline double desire_from_fatigue(double fatigue, double enter){
  if (fatigue <= enter) return 0.0;
  return clamp01( (fatigue - enter) / (100.0 - enter) );
}

inline double CalcScoreSleep(
    double fatigue,
    bool hasBed,
    bool stickySleepActive,
    double tiredEnter,
    double restedExit,
    double sticky   = 0.2 // should be less than wander score
){
  if (!hasBed) return 0.0;

  const double desire = desire_from_fatigue(fatigue, tiredEnter);
  const double base = desire * 0.833;

  double gate = 0.0;
  if (fatigue > restedExit) {
    if (fatigue >= tiredEnter) gate = 1.0;
    else gate = (fatigue - restedExit) / static_cast<double>(tiredEnter - restedExit); // 0~1
  }

  const double stick = stickySleepActive ? (sticky * gate) : 0.0;

  return base + stick;
}

#endif //UTILS_H

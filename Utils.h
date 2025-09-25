//
// Created by jacob on 25-9-21.
//

#ifndef UTILS_H
#define UTILS_H
inline double clamp01(double x){ return x<0?0:(x>1?1:x); }

inline int manhattan(std::pair<int,int> a, std::pair<int,int> b){
  return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}

// 线性 desire
inline double desire_from_hunger(double hunger, int enter=60){
  if (hunger <= enter) return 0.0;
  return clamp01( (hunger - enter) / double(100 - enter) );

}

// scoreEat 计算
inline double CalcScoreEat(
    int hunger,
    std::pair<int,int> agentPos,
    bool hasFood,
    std::pair<int,int> foodPos,
    bool onCooldown,
    bool stickyEatActive,
    double k_dist   = 0.2,
    int hungryEnter = 60,
    double sticky   = 0.2
){
  if (!hasFood) return 0.0;
  if (onCooldown) return 0.0; // 冷却直接禁止

  const double desire = desire_from_hunger(hunger, hungryEnter);
  const int dist = manhattan(agentPos, foodPos);
  const double feasibility = 1.0 / (1.0 + k_dist * double(dist));
  const double base = desire * feasibility;

  return stickyEatActive ? (base + sticky) : base;
}

#endif //UTILS_H

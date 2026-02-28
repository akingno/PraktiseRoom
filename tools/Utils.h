//
// Created by jacob on 25-9-21.
//

#ifndef UTILS_H
#define UTILS_H


inline double clamp01(double x){ return x<0?0:(x>1?1:x); }


// 目前无用
inline int manhattan(std::pair<int,int> a, std::pair<int,int> b){
  return std::abs(a.first - b.first) + std::abs(a.second - b.second);
}


// 通用需求打分函数
// currentValue:当前属性值
// thresholdEnter:开始想去解决的阈值
// hasItem:环境里有没有能解决这个需求的物品？
// isDoingIt:当前是不是正在执行这个动作？ 用于增加粘性
// weight:自定义需求可以设置不同的权重，比如求生需求权重高，娱乐需求权重低，默认给 1.0
inline double CalcScoreGeneric(double currentValue, double thresholdEnter,double thresholdExit,
                               bool hasItem, bool isDoingIt, double weight = 1.0)
{
  if (!hasItem) return 0.0; // 没东西可交互，分数为 0

  // 如果正在做这件事，给一个极高的粘性分数，防止被打断
  if (isDoingIt) {
    // 只要还没完全满足到退出值，就继续做
    if (currentValue > thresholdExit) return 1000.0;
    return 0.0; // 满足了
  }

  // 如果没在做，且超过了阈值，计算一个基础分数
  if (currentValue > thresholdEnter) {
    // 超出阈值越多，分数越高，并乘上权重
    // 加 10.0 是为了保证基础分高于 Wander(0.05)
    return (currentValue * 0.2 + 10.0) * weight;
  }

  return 0.0;
}



#endif //UTILS_H

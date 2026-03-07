//
// Created by jacob on 26-3-3.
//

#ifndef IBRAIN_H
#define IBRAIN_H

#include <vector>
#include <cstdint>
#include <string>

// 提前声明需要的类，避免循环包含
class Agent;
class Room;
class ItemLayer;

enum class AIType {
  Static,
  Utility,
  LLM, // llm驱动的ai
  Player // 玩家控制
};

// 大脑基类接口
class IBrain {
public:
  virtual ~IBrain() = default;

  // 核心接口：每帧观察世界得到动作
  virtual void think(Agent* body, double dt_sec, uint64_t tick_index, Room& room, ItemLayer& items, const std::vector<Agent*>& others) = 0;

  virtual AIType getType() const = 0;

  virtual void onTriggerNotified(Agent* body, const std::string& triggerer_id) {}
};
#endif //IBRAIN_H

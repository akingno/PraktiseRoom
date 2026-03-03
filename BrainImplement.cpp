//
// Created by jacob on 26-3-3.
//
#include "BrainImplement.h"
#include "Agent.h"

void StaticBrain::think(Agent* body, double dt_sec, uint64_t tick_index, Room& room, ItemLayer& items, const std::vector<Agent*>& others) {
  // 没事干就原地发呆
  body->applyDecision(Character::Act::Stop);
}

void UtilityBrain::think(Agent* body, double dt_sec, uint64_t tick_index, Room& room, ItemLayer& items, const std::vector<Agent*>& others) {
  // Decisionmaker处理
}

void LLMBrain::think(Agent* body, double dt_sec, uint64_t tick_index, Room& room, ItemLayer& items, const std::vector<Agent*>& others) {
  // 预留给llm
}

void PlayerBrain::think(Agent* body, double dt_sec, uint64_t tick_index, Room& room, ItemLayer& items, const std::vector<Agent*>& others) {
  // 预留给玩家输入
}

std::unique_ptr<IBrain> createBrain(AIType type) {
  switch (type) {
    case AIType::Static:  return std::make_unique<StaticBrain>();
    case AIType::Utility: return std::make_unique<UtilityBrain>();
    case AIType::LLM:     return std::make_unique<LLMBrain>();
    case AIType::Player:  return std::make_unique<PlayerBrain>();
  }
  return std::make_unique<StaticBrain>();
}
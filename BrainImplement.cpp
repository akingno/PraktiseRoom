//
// Created by jacob on 26-3-3.
//
#include "BrainImplement.h"
#include "Agent.h"
#include "spdlog/spdlog.h"
#include <SDL3/SDL.h>

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
  const bool* state = SDL_GetKeyboardState(NULL);
  auto pos = body->getCharacter().getLoc();

  if (state[SDL_SCANCODE_J]) {
    // 检查脚下有没有物品
    auto iid = items.idAt(pos.first, pos.second);
    if (iid) {
      body->applyDecision(Character::Act::PlayerInteract);
      spdlog::info("Player used item: {}", *iid);
    }
    return;
  }
  int dx = 0, dy = 0;
  if (state[SDL_SCANCODE_W]) dy = -1;
  else if (state[SDL_SCANCODE_S]) dy = 1;
  else if (state[SDL_SCANCODE_A]) dx = -1;
  else if (state[SDL_SCANCODE_D]) dx = 1;

  if (dx != 0 || dy != 0) {
    int nx = pos.first + dx;
    int ny = pos.second + dy;

    if (room.isPassable(nx, ny)) {
      body->applyDecision(Character::Act::PlayerMove, "", {nx, ny});
    }
  }
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
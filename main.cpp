#include "ActionExecutor.h"
#include "Character.h"
#include "GameContentInit.h"
#include "Room.h"
#include "renders/IRender.h"
#include "renders/SDL3Render.h"
#include "tools/AStarPathfinder.h"
#include <chrono>
#include <thread>

#include "Agent.h"
#include "DecisionMaker.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>


int main() {

#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
  std::ios::sync_with_stdio(false);


  // 初始化
  uint64_t seed = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  AkRandom::init(seed);
  //初始化结束

  bool running = true;
  Room room;
  ItemLayer items;
  std::string name_char1 = "张三";
  std::string name_char2 = "李四";

  register_default_items();
  items.ensureBedPlaced();
  items.ensureFoodSpawned();
  items.ensureComputerPlaced();

  AStarPathfinder path_finder(
    {VIEW_W, VIEW_H},
    [&](int x, int y) { return room.isPassable(x, y); }
  );

  std::vector<std::unique_ptr<Agent>> agents;
  agents.push_back(std::make_unique<Agent>(name_char1, 5, 5, &path_finder));
  agents.push_back(std::make_unique<Agent>(name_char2, 1, 1, &path_finder));

  const Character& character1 = agents[0]->getCharacter();
  const Character& character2 = agents[1]->getCharacter();

  std::vector<Agent*> raw_agents_ptrs;
  for(auto& a : agents) {
    raw_agents_ptrs.push_back(a.get());
  }

  //全局决策器
  DecisionMaker decisionMaker;

  //SDL3渲染器
  std::unique_ptr<IRender> render = std::make_unique<SDL3Render>(VIEW_W, VIEW_H, TILE_PX, "Little Room");

  //计时器
  using clock = std::chrono::steady_clock;
  auto next_tick = clock::now();
  const auto dt = std::chrono::milliseconds(TICK_MILLI_INT);
  uint64_t tick_index = 0;

  while(running) {

    if (render->poll_quit()) break;

    // 每循环固定刷新一下食物
    items.ensureFoodSpawned();

    //更新+移动
    for (auto& agent : agents) {
      // Pathfinder 已经在 Agent 内部了，不需要在这里传
      agent->update(TICK_MILLI/1000.0, tick_index, room, items, raw_agents_ptrs);
    }

    if (!decisionMaker.isThinking()) {
      bool anyNeedsDecision = false;
      // 检查是否有任何一个agent处于空闲缺策状态
      for (auto* agent : raw_agents_ptrs) {
        if (agent->needsNewDecision()) {
          anyNeedsDecision = true;
          // 标记为思考中防止下一帧重复触发
          agent->markThinking();
        }
      }

      if (anyNeedsDecision) {
        decisionMaker.requestBatchDecision(raw_agents_ptrs, tick_index);
      }
    }
    decisionMaker.poll(raw_agents_ptrs);

    //渲染
    render->render_frame(items, agents, room);

#ifndef NDEBUG
    if (tick_index % 20 == 0) {
      const auto& c1 = agents[0]->getCharacter();
      std::cout << "[Tick " << tick_index << "] " << agents[0]->getName()
      << " Act: " << Character::Act2Str(c1.act())
      << " Board: " << std::to_string(c1.get_boredom())
      << " Mem: " << c1.get_short_memory().to_string()<< std::endl <<std::endl;
    }

#endif

    next_tick += dt;
    std::this_thread::sleep_until(next_tick);
    ++tick_index;


  // End of Loop
  }

}


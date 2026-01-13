#include "ActionExecutor.h"
#include "Character.h"
#include "GameContentInit.h"
#include "Room.h"
#include "renders/IRender.h"
#include "renders/SDL3Render.h"
#include "tools/AStarPathfinder.h"
#include "tools/Utils.h"
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif
#include "Agent.h"

#include <iostream>


int main() {

#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
  std::ios::sync_with_stdio(false);


  // 初始化
  uint64_t seed = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  Random::init(seed);
  //初始化结束

  bool running = true;
  Room room;
  ItemLayer items;
  std::string name_char1, name_char2;
  name_char1 = "Akingno";
  name_char2 = "Lilu";

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


  Blackboard bb;

  //SDL3渲染器
  std::unique_ptr<IRender> render = std::make_unique<SDL3Render>(VIEW_W, VIEW_H, TILE_PX, "Little Room (SDL3)");

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
      agent->Update(TICK_MILLI/1000.0, tick_index, room, items);
    }

    //渲染
    render->render_frame(items, agents, room);

    std::cout<< std::string("Action: ") << Character::Act2Str(character1.act())<<"\n"<<
    "Inner Hunger= " + std::to_string(character1.get_hunger_inner())<<"\n"<<
      "Inner Fatigue= " + std::to_string(character1.get_fatigue_score())<<"\n"<<
        "Bored score= " + std::to_string(character1.get_boredom()) <<"\n" <<
          "Sleeping Status= " + std::to_string(character1.isSleeping())<<"\n"<<
            "Recent Memory: " << character1.get_short_memory().to_string()<<"\n\n";

    std::cout<< std::string("Action2: ") << Character::Act2Str(character2.act())<<"\n"<<
        "Inner Hunger= " + std::to_string(character2.get_hunger_inner())<<"\n"<<
          "Inner Fatigue= " + std::to_string(character2.get_fatigue_score())<<"\n"<<
            "Bored score= " + std::to_string(character2.get_boredom()) <<"\n" <<
              "Sleeping Status= " + std::to_string(character2.isSleeping())<<"\n"<<
                "Recent Memory: " << character2.get_short_memory().to_string()<<"\n";

    next_tick += dt;
    std::this_thread::sleep_until(next_tick);
    ++tick_index;


  // End of Loop
  }

}


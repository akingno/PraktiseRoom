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
#include <SDL3/SDL.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "EditorUI.h"
#include "InputController.h"

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
  Cfg::load("config.json");
  //初始化结束

  bool running = true;
  Room room;
  ItemLayer items;
  std::string name_char1 = "张三";
  std::string name_char2 = "李四";

  //物品加载和读取
  loadItems("items.json");
  items.loadFromFile("world.json");

  AStarPathfinder path_finder(
      {Cfg::room::view_w, Cfg::room::view_h},
      [&](int x, int y) { return room.isPassable(x, y); });

  std::vector<std::unique_ptr<Agent>> agents;
  agents.push_back(std::make_unique<Agent>(name_char1, 5, 5, &path_finder));
  agents.push_back(std::make_unique<Agent>(name_char2, 1, 1, &path_finder));

  // 用于debug打印
  const Character &character1 = agents[0]->getCharacter();
  const Character &character2 = agents[1]->getCharacter();

  // decision那边需要一个指针
  std::vector<Agent *> raw_agents_ptrs;
  for (auto &a : agents) {
    raw_agents_ptrs.push_back(a.get());
  }

  //全局决策器
  DecisionMaker decisionMaker;

  //SDL3渲染器
  std::unique_ptr<IRender> render = std::make_unique<SDL3Render>(Cfg::room::view_w, Cfg::room::view_h, Cfg::core::tile_px, "Room Simulator");
  //ImGui初始化
  EditorUI editorUI;
  editorUI.init(render.get());

  //计时器
  using clock = std::chrono::steady_clock;
  auto next_tick = clock::now();
  const auto dt = std::chrono::milliseconds(Cfg::core::tick_milli_int);
  uint64_t tick_index = 0;
  bool is_paused = false;
  InputController input;

  while (running) {
    input.handleEvents(running, is_paused ,editorUI, room, items, agents);

    if (!running) break;

    if (!is_paused) {

      //更新+移动
      for (auto &agent : agents) {
        agent->update(Cfg::core::tick_milli / 1000.0, tick_index, room, items, raw_agents_ptrs);
      }

      if (!decisionMaker.isThinking()) {
        bool anyNeedsDecision = false;
        for (auto *agent : raw_agents_ptrs) {
          if (agent->needsNewDecision()) {
            anyNeedsDecision = true;
            agent->markThinking();
          }
        }
        if (anyNeedsDecision) {
          decisionMaker.requestBatchDecision(raw_agents_ptrs, tick_index, items);
        }
      }
      decisionMaker.poll(raw_agents_ptrs);

      ++tick_index; // tick 计数也放进不暂停的逻辑里
    }

    //渲染
    std::string preview_id = (input.current_mode == EditorMode::Placement) ? input.selected_placement_item : "";
    render->render_frame(items, agents, room, preview_id, input.mouse_gx, input.mouse_gy);

    editorUI.render(is_paused, render->getRenderer(), render.get(), input.current_mode, input.selected_placement_item, agents, items);

    SDL_RenderPresent(render->getRenderer());

#ifndef NDEBUG
    if (tick_index % 20 == 0 && !is_paused) {
      const auto &c1 = agents[0]->getCharacter();
      std::cout << "[Tick " << tick_index << "] " << agents[0]->getName() << " Inner Hunger: " + std::to_string(character1.getStat("hunger")) << "\n"
                << " Inner Fatigue: " + std::to_string(character1.getStat("fatigue")) << "\n"
                << " Bored: " + std::to_string(character1.getStat("boredom")) << "\n"
                << " Mem: " << c1.get_short_memory().to_string() << std::endl
                << std::endl;
    }

#endif

    next_tick += dt;
    std::this_thread::sleep_until(next_tick);


  }// End of Loop

  editorUI.closeClear();

}

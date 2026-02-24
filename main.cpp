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

  loadItems("items.json");
  items.ensureBedPlaced();
  items.ensureFoodSpawned();
  items.ensureComputerPlaced();

  AStarPathfinder path_finder(
      {Cfg::room::view_w, Cfg::room::view_h},
      [&](int x, int y) { return room.isPassable(x, y); });

  std::vector<std::unique_ptr<Agent>> agents;
  agents.push_back(std::make_unique<Agent>(name_char1, 5, 5, &path_finder));
  agents.push_back(std::make_unique<Agent>(name_char2, 1, 1, &path_finder));

  const Character &character1 = agents[0]->getCharacter();
  const Character &character2 = agents[1]->getCharacter();

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

  while (running) {
    SDL_Event e;
    // Input Process
    while (SDL_PollEvent(&e)) {
      if (editorUI.processEvent(&e)) {
        continue;
      }

      if (e.type == SDL_EVENT_QUIT) {
        running = false;
      }
      if (e.type == SDL_EVENT_KEY_DOWN) {
        if (e.key.key == SDLK_ESCAPE) running = false;

        //空格暂停
        if (e.key.key == SDLK_SPACE) {
          is_paused = !is_paused;
          std::cout << (is_paused ? "System: Game Paused." : "System: Game Resumed.") << std::endl;
        }
      }
      if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && e.button.button == SDL_BUTTON_LEFT) {
        // 计算游戏原始画面的像素边界
        int gameW = Cfg::room::view_w * Cfg::core::tile_px;
        int gameH = Cfg::room::view_h * Cfg::core::tile_px;

        float mx = e.button.x;
        float my = e.button.y;

        // 确保点击是在画面内，而不是在右侧或下方
        if (mx >= 0 && mx < gameW && my >= 0 && my < gameH) {
          // 像素坐标 -> 网格坐标
          int gx = static_cast<int>(mx) / Cfg::core::tile_px;
          int gy = static_cast<int>(my) / Cfg::core::tile_px;

          Agent* clicked_agent = nullptr;

          // 遍历寻找坐标匹配的小人
          for (auto& a : agents) {
            if (a->getCharacter().getLoc() == std::make_pair(gx, gy)) {
              clicked_agent = a.get();
              break; // 找到了就跳出循环
            }
          }

          editorUI.setSelectedAgent(clicked_agent);
        }
      }
    }
    if (!running) break;

    if (!is_paused) {
      // 每循环固定刷新一下食物
      items.ensureFoodSpawned();

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

    //渲染到后台
    render->render_frame(items, agents, room);

    editorUI.render(is_paused, render->getRenderer());

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

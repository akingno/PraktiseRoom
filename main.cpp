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
#include "event_bus/EventBindings.h"
#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

void initLogger() {
  try {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("engine.log", true);

    spdlog::sinks_init_list sink_list = {console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("engine", sink_list.begin(), sink_list.end());

    spdlog::set_default_logger(logger);

    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
    spdlog::set_level(spdlog::level::debug);

    spdlog::info("Spdlog initialized successfully! Welcome.");
  } catch (const spdlog::spdlog_ex& ex) {
    std::cout << "Log initialization failed: " << ex.what() << std::endl;
  }
}

int main() {
  initLogger();
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

  //物品加载和读取
  loadItems("items.json");
  loadTerrains("terrains.json");
  loadTriggers("triggers.json");

  //世界初始化
  Room room(Cfg::room::view_w, Cfg::room::view_h);
  room.loadFromFile("room_map.json");

  //物品初始化
  ItemLayer items;
  items.loadFromFile("world.json");

  //agents
  std::vector<std::unique_ptr<Agent>> agents;
  //SDL3渲染器
  std::unique_ptr<IRender> render = std::make_unique<SDL3Render>(Cfg::room::view_w, Cfg::room::view_h, Cfg::core::tile_px, "Room Engine");

  loadAgents(agents, render.get(), "agents.json");


  if (agents.empty()) {
    agents.push_back(std::make_unique<Agent>("张三", "npc_zhang", 5, 5, AIType::Utility, "character.png"));
    agents.push_back(std::make_unique<Agent>("李四", "npc_li", 1, 1, AIType::Utility, "character.png"));
  }


  //全局决策器
  DecisionMaker decisionMaker;


  //ImGui初始化
  EditorUI editorUI;
  editorUI.init(render.get());

  //计时器
  using clock = std::chrono::steady_clock;
  auto next_tick = clock::now();
  const auto dt = std::chrono::milliseconds(Cfg::core::tick_milli_int);
  uint64_t tick_index = 0;
  bool is_paused = false;

  // 输入处理
  InputController input;
  SystemBindings::bindAllUIEvents(room, items, agents, render.get());

  while (running) {
    input.handleEvents(running, is_paused ,editorUI, room, items, agents);

    if (!running) break;

    if (!is_paused) {
      //重新构造当前的小人指针（因为可能加了新的）
      std::vector<Agent*> raw_agents_ptrs;
      for (auto &a : agents) {
        if (a->getAIType()!=AIType::Player) {
          raw_agents_ptrs.push_back(a.get());
        }
      }

      //更新+移动
      for (auto &agent : agents) {
        agent->update(Cfg::core::tick_milli / 1000.0, tick_index, room, items, raw_agents_ptrs);
      }

      if (!decisionMaker.isThinking()) {
        std::vector<Agent*> thinking_agents;
        for (auto *agent : raw_agents_ptrs) {
          if (agent->needsNewDecision()) {
            agent->markThinking();
            thinking_agents.push_back(agent);
          }
        }
        if (!thinking_agents.empty()) {
          decisionMaker.requestBatchDecision(thinking_agents, tick_index, items);
        }
      }
      decisionMaker.poll(raw_agents_ptrs);

      ++tick_index; // tick 计数也放进不暂停的逻辑里
    }

    //渲染
    bool show_triggers = input.current_mode != EditorMode::Play;
    std::string preview_id = (input.current_mode == EditorMode::Placement) ? input.selected_placement_item : "";
    render->render_frame(items, agents, room, preview_id, input.mouse_gx, input.mouse_gy, show_triggers);

    editorUI.render(is_paused,
      render->getRenderer(),
      input.current_mode,
      input.selected_placement_item,
      input.selected_terrain_id
      );

    SDL_RenderPresent(render->getRenderer());

#ifndef NDEBUG
    if (tick_index % 20 == 0 && !is_paused && !agents.empty()) {
      const auto &c1 = agents[0]->getCharacter();

      spdlog::debug("[Tick {}] {} Inner Hunger: {:.2f} | Inner Fatigue: {:.2f} | Bored: {:.2f} | Mem: {}",
                    tick_index,
                    agents[0]->getName(),
                    c1.getStat("hunger"),
                    c1.getStat("fatigue"),
                    c1.getStat("boredom"),
                    c1.get_short_memory().to_string());
    }
#endif

    next_tick += dt;
    std::this_thread::sleep_until(next_tick);


  }// End of Loop

  editorUI.closeClear();

}

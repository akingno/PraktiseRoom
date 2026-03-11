//
// Created by jacob on 26-3-1.
//

#include "EventBindings.h"
#include "../GameContentInit.h"
#include "../WorldManager.h"
#include "EventBus.h"
#include <chrono>
#include <iostream>

void SystemBindings::bindAllUIEvents(std::vector<std::unique_ptr<Agent>> &agents, IRender *render) {

  EventBus::onUI_SaveAllRequested.connect([ &agents]() {
    WorldManager::inst().saveAllWorlds();
    saveItems("items.json");
    Cfg::save("config.json");
    saveTerrains("terrains.json");
    saveAgents(agents, "agents.json");
    saveTriggers("triggers.json");
    spdlog::info("EventBus: 保存了所有更改");
  });

  EventBus::onUI_LoadItemsRequested.connect([]() { loadItems("items.json"); });
  EventBus::onUI_LoadConfigRequested.connect([]() { Cfg::load("config.json"); });
  EventBus::onUI_LoadWorldRequested.connect([]() {
    if (auto* lvl = WorldManager::inst().getActiveLevel()) {
          lvl->items->loadFromFile("world_items_" + std::to_string(lvl->level_id) + ".json");
      }
  });
  EventBus::onUI_LoadTerrainsRequested.connect([]() { loadTerrains("terrains.json"); });
  EventBus::onUI_LoadRoomMapRequested.connect([]() {
    if (auto* lvl = WorldManager::inst().getActiveLevel()) {
          lvl->room->loadFromFile("room_map_" + std::to_string(lvl->level_id) + ".json");
      }
  });
  EventBus::onUI_LoadAgentsRequested.connect([&agents, render]() {
    loadAgents(agents, render, "agents.json");
  });

  EventBus::onUI_ApplyNewNeed.connect([&agents](std::string name, float growth, float enter, float exit, float weight) {
    bool exists = false;
    for (const auto &r : Cfg::need_rules) {
      if (r.name == name) {
        exists = true;
        break;
      }
    }
    if (!exists) {
      Cfg::NeedRule rule{name, growth, enter, exit, weight};
      Cfg::need_rules.push_back(rule);
      for (auto &agent : agents) agent->getCharacter().registerNewStat(name, growth);
      spdlog::info("EventBus: 应用新需求");
    }
  });

  EventBus::onUI_CreateItem.connect([render](std::string id, std::string tex, bool use, bool block, std::string tgt, float val) {
    auto smartItem = std::make_unique<SmartItem>(id, ItemProps{false, block, use, tex});
    if (!tgt.empty()) smartItem->addEffect({EffectType::ModifyStat, tgt, static_cast<double>(val)});
    smartItem->setSequence({
      {"MoveToTarget", "", 0, 0, 0.0f},
      {"Interact", "", 0, 0, 0.0f}
    });
    ItemRegistry::inst().register_item(std::move(smartItem));
    render->loadDynamicTexture(id, tex);
    spdlog::info("EventBus: 热加载物品: {}", id);
  });

  EventBus::onUI_CreateTerrain.connect([render](std::string id, std::string tex, bool blocks) {
    TerrainRegistry::inst().register_terrain({id, tex, blocks});
    render->loadTerrainTexture(id, tex);
    spdlog::info("EventBus: 热加载地形: {}", id);
  });

  EventBus::onUI_CreateAgent.connect([&agents, render](std::string name, std::string id, int x, int y, AIType type, std::string tex) {
    auto new_agent = std::make_unique<Agent>(name, id, x, y, type, tex);
    new_agent->getCharacter().setLevel(WorldManager::inst().current_active_level);

    agents.push_back(std::move(new_agent));
    if (type == AIType::Utility) {
      for (const auto &r : Cfg::need_rules) agents.back()->getCharacter().registerNewStat(r.name, r.growth_rate);
    }
    render->loadAgentTexture(tex);
    spdlog::info("EventBus: 创造新实体: {} at {}, {} in Level {}", name, x, y, WorldManager::inst().current_active_level);
  });

  EventBus::onTriggerStepped.connect([&agents](const std::string& trigger_id, const std::string& triggerer_id) {
        //根据 id得到 trigger
        const auto& all_triggers = TriggerManager::inst().getAllTriggers();
        auto it = all_triggers.find(trigger_id);

        if (it != all_triggers.end()) {
            const TriggerDef& def = it->second;

            // 找到trigger执行的agent和triggerer
            Agent* triggerer_ptr = nullptr;
            Agent* target_ptr = nullptr;

            for (auto& agent : agents) {
                if (agent->getId() == triggerer_id) triggerer_ptr = agent.get();
                if (agent->getId() == def.target_id) target_ptr = agent.get();
            }

            if (triggerer_ptr && target_ptr) {
                target_ptr->receiveTrigger(triggerer_ptr);
            }
        }
    });

  EventBus::onStaticSequenceUpdated.connect([&agents](std::string agent_id, std::vector<ActionDescriptor> new_seq) {
    for (auto& agent : agents) {
      if (agent->getId() == agent_id && agent->getAIType() == AIType::Static) {
        agent->setStaticAISequence(new_seq);
        spdlog::info("EventBus: Successfully updated sequence for Static AI: {}", agent_id);
        break;
      }
    }
  });

  EventBus::onUI_CreateTrigger.connect([](int x, int y, std::string target_id) {
    TriggerDef def;
    def.id = "trg_" + std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    def.level = WorldManager::inst().current_active_level;
      def.x = x;
      def.y = y;
      def.type = TriggerType::NotifyBind;
      def.target_id = target_id;

      TriggerManager::inst().addTrigger(def);
      spdlog::info("[EventBus] Created trigger [{}] at ({}, {}) targeting [{}]", def.id, x, y, target_id);
    });

  EventBus::onUI_CreateLevel.connect([](int id, int w, int h) {
      WorldManager::inst().createLevel(id, w, h);
      WorldManager::inst().current_active_level = id;
      spdlog::info("EventBus: Created and switched to Level {}", id);
    });
}
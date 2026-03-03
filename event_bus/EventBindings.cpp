//
// Created by jacob on 26-3-1.
//

#include "EventBindings.h"
#include "../GameContentInit.h"
#include "EventBus.h"
#include <iostream>

void SystemBindings::bindAllUIEvents(Room &room, ItemLayer &items, std::vector<std::unique_ptr<Agent>> &agents, IRender *render) {

  EventBus::onUI_SaveAllRequested.connect([&room, &items, &agents]() {
    saveItems("items.json");
    Cfg::save("config.json");
    items.saveToFile("world.json");
    saveTerrains("terrains.json");
    room.saveToFile("room_map.json");
    saveAgents(agents, "agents.json");
    spdlog::info("EventBus: 保存了所有更改");
  });

  EventBus::onUI_LoadItemsRequested.connect([]() { loadItems("items.json"); });
  EventBus::onUI_LoadConfigRequested.connect([]() { Cfg::load("config.json"); });
  EventBus::onUI_LoadWorldRequested.connect([&items]() { items.loadFromFile("world.json"); });
  EventBus::onUI_LoadTerrainsRequested.connect([]() { loadTerrains("terrains.json"); });
  EventBus::onUI_LoadRoomMapRequested.connect([&room]() { room.loadFromFile("room_map.json"); });
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
    smartItem->setSequence({{"MoveToTarget", 0, ""}, {"Interact", 0, ""}});
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
    agents.push_back(std::make_unique<Agent>(name, id, x, y, type, tex));
    if (type == AIType::Utility) {
      for (const auto &r : Cfg::need_rules) agents.back()->getCharacter().registerNewStat(r.name, r.growth_rate);
    }
    render->loadAgentTexture(tex);
    spdlog::info("EventBus: 创造新实体: {} at {}, {}.", name, x, y);
  });
}
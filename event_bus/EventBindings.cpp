//
// Created by jacob on 26-3-1.
//

#include "EventBindings.h"
#include "EventBus.h"
#include "../GameContentInit.h"
#include <iostream>

void SystemBindings::bindAllUIEvents(Room& room, ItemLayer& items, std::vector<std::unique_ptr<Agent>>& agents, IPathfinder* pf, IRender* render) {

    EventBus::onUI_SaveAllRequested.connect([&room, &items, &agents]() {
        saveItems("items.json");
        Cfg::save("config.json");
        items.saveToFile("world.json");
        saveTerrains("terrains.json");
        room.saveToFile("room_map.json");
        saveAgents(agents, "agents.json");
        std::cout << "EventBus: 保存了所有更改。" << std::endl;
    });

    EventBus::onUI_LoadItemsRequested.connect([]() { loadItems("items.json"); });
    EventBus::onUI_LoadConfigRequested.connect([]() { Cfg::load("config.json"); });
    EventBus::onUI_LoadWorldRequested.connect([&items]() { items.loadFromFile("world.json"); });
    EventBus::onUI_LoadTerrainsRequested.connect([]() { loadTerrains("terrains.json"); });
    EventBus::onUI_LoadRoomMapRequested.connect([&room]() { room.loadFromFile("room_map.json"); });
  EventBus::onUI_LoadAgentsRequested.connect([&agents, pf, render]() {
      loadAgents(agents, pf, render, "agents.json");
  });

  EventBus::onUI_ApplyNewNeed.connect([&agents](std::string name, float growth, float enter, float exit, float weight) {
        bool exists = false;
        for (const auto &r : Cfg::need_rules) { if (r.name == name) { exists = true; break; } }
        if (!exists) {
            Cfg::NeedRule rule{name, growth, enter, exit, weight};
            Cfg::need_rules.push_back(rule);
            for (auto &agent : agents) agent->getCharacter().registerNewStat(name, growth);
            std::cout << "EventBus: 应用新需求: " << name << std::endl;
        }
    });

  EventBus::onUI_CreateItem.connect([render](std::string id, std::string tex, bool use, bool block, std::string tgt, float val) {
        auto smartItem = std::make_unique<SmartItem>(id, ItemProps{false, block, use, tex});
        if (!tgt.empty()) smartItem->addEffect({EffectType::ModifyStat, tgt, static_cast<double>(val)});
        smartItem->setSequence({{"MoveToTarget", 0, ""}, {"Interact", 0, ""}});
        ItemRegistry::inst().register_item(std::move(smartItem));
        render->loadDynamicTexture(id, tex);
        std::cout << "EventBus: 热加载物品: " << id << std::endl;
    });

  EventBus::onUI_CreateTerrain.connect([render](std::string id, std::string tex, bool blocks) {
        TerrainRegistry::inst().register_terrain({id, tex, blocks});
        render->loadTerrainTexture(id, tex);
        std::cout << "EventBus: 热加载地形: " << id << std::endl;
    });

   EventBus::onUI_CreateAgent.connect([&agents, pf, render](std::string name, std::string id, int x, int y, AIType type, std::string tex) {
        agents.push_back(std::make_unique<Agent>(name, id, x, y, pf, type, tex));
        if (type == AIType::Utility) {
            for (const auto &r : Cfg::need_rules) agents.back()->getCharacter().registerNewStat(r.name, r.growth_rate);
        }
        render->loadAgentTexture(tex);
        std::cout << "EventBus: 创造实体: " << name << " at " << x << "," << y << std::endl;
    });
}
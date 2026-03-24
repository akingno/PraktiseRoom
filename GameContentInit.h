//
// Created by jacob on 25-10-24.
//
#pragma once

#include "Agent.h"
#include "Character.h"
#include "Config.h"
#include "ItemRegistry.h"
#include "SmartItem.h"
#include "TriggerManager.h"
#include "renders/IRender.h"
#include "spdlog/spdlog.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

inline EffectType parseEffectType(const std::string &typeStr) {
  if (typeStr == "SetState") {
    return EffectType::SetState;
  }
  if (typeStr == "AddMemory") {
    return EffectType::AddMemory;
  }
  return EffectType::ModifyStat;//default
}

inline std::string effectTypeToString(EffectType type) {
  if (type == EffectType::SetState) {
    return "SetState";
  }
  if (type == EffectType::AddMemory) {
    return "AddMemory";
  }
  return "ModifyStat";
}

inline const std::vector<std::string> kComputerFeed = {
    {u8"看了一段搞笑视频"},
    {u8"读到一条尖刻评论"},
    {u8"学到一个新技巧"},
    {u8"看到坏消息"},
    {u8"完成日常签到"},
    {u8"听了点轻音乐"},
    {u8"刷到宠物视频"},
    {u8"被广告刷屏"},
    {u8"读到励志故事"},
    {u8"无聊地发呆"},
};

inline void generateDefaultTerrains() {
  TerrainRegistry::inst().clear();
  TerrainRegistry::inst().register_terrain({"grass", "grass.png", false});
  TerrainRegistry::inst().register_terrain({"wall", "wall_h.png", true});
  TerrainRegistry::inst().register_terrain({"door", "door.png", false});

  spdlog::info("GameInit:TerrainLoader: Generated default terrains in memory");
}

inline void generateDefaultItems() {
  ItemRegistry::inst().clear();

  // 食物
  auto food = std::make_unique<SmartItem>("food", ItemProps{false, false, true, "food.png"});
  food->addEffect({EffectType::ModifyStat, "hunger", static_cast<double>(-Cfg::item::food_calories)});
  food->setSequence({{"MoveToTarget", "", 0, 0, 0.0f},
    {"Interact", "", 0, 0, 0.0f}});
  ItemRegistry::inst().register_item(std::move(food));

  // 床
  auto bed = std::make_unique<SmartItem>("bed", ItemProps{false, false, true, "bed.png"});
  bed->addEffect({EffectType::ModifyStat, "fatigue", -100.0});
  bed->setSequence({{"MoveToTarget", "", 0, 0, 0.0f},
    {"Interact", "", 0, 0, 0.0f},
    {"Wait", "", 100, 0, 0.0f}});
  ItemRegistry::inst().register_item(std::move(bed));

  // 电脑
  auto computer = std::make_unique<SmartItem>("computer", ItemProps{false, false, true, "computer.png"});
  computer->addEffect({EffectType::ModifyStat, "boredom", static_cast<double>(-Cfg::item::play_computer_entertain)});
  computer->addEffect({EffectType::AddMemory, "$RANDOM_COMPUTER$", 0.0});
  int useTicks = AkRandom::randint(Cfg::time::min_use_computer, Cfg::time::max_use_computer) * Cfg::core::ticks_per_sec;
  computer->setSequence({{"MoveToTarget", "", 0, 0, 0.0f},
    {"Interact", "", 0, 0, 0.0f},
    {"Wait", "", useTicks, 0, 0.0f}});
  ItemRegistry::inst().register_item(std::move(computer));

  spdlog::info("GameInit:ItemLoader: Generated default items in memory.");
}

inline void saveTerrains(const std::string& filename = "terrains.json") {
  json jArray = json::array();
  for (const auto& [id, def] : TerrainRegistry::inst().getAllTerrains()) {
    json j;
    j["id"] = def.id;
    j["texture_name"] = def.texture_name;
    j["blocks"] = def.blocks;
    jArray.push_back(j);
  }
  std::ofstream out(filename);
  if (out.is_open()) {
    out << jArray.dump(4);
    spdlog::info("GameInit:TerrainLoader: Saved to file {}", filename);
  }
}

inline void loadTerrains(const std::string& filename = "terrains.json") {
  std::ifstream file(filename);
  if (!file.is_open()) {
    spdlog::info("GameInit:TerrainLoader: Failed to load from{}, generating terrains...", filename);
    generateDefaultTerrains();
    saveTerrains(filename);
    return;
  }
  try {
    json jArray;
    file >> jArray;
    TerrainRegistry::inst().clear();
    for (const auto& jItem : jArray) {
      TerrainDef def;
      def.id = jItem.value("id", "");
      def.texture_name = jItem.value("texture_name", "");
      def.blocks = jItem.value("blocks", false);
      if (!def.id.empty()) {
        TerrainRegistry::inst().register_terrain(def);
      }
    }
    spdlog::info("GameInit:TerrainLoader: Loaded {} terrains from {}", jArray.size(), filename);
  } catch (const std::exception& e) {
    spdlog::error("GameInit:TerrainLoader: Failed to parse JSON: {}", e.what());
  }
}

inline void saveItems(const std::string &filename = "items.json") {
  json jArray = json::array();

  // 遍历内存中的所有物品
  for (const auto &[id, itemPtr] : ItemRegistry::inst().getAllItems()) {
    auto *smartItem = dynamic_cast<SmartItem *>(itemPtr.get());
    if (!smartItem) continue;

    json jItem;
    jItem["id"] = id;

    //Props
    jItem["props"] = {
        {"pickable", smartItem->props().pickable},
        {"blocks", smartItem->props().blocks},
        {"useable", smartItem->props().useable},
        {"texture_name", smartItem->props().texture_name}};

    //Effects
    json jEffects = json::array();
    for (const auto &eff : smartItem->getEffects()) {
      jEffects.push_back({{"type", effectTypeToString(eff.type)},
                          {"target", eff.target},
                          {"value", eff.value}});
    }
    if (!jEffects.empty()) jItem["effects"] = jEffects;

    //Sequence
    json jSeq = json::array();
    for (const auto &desc : smartItem->getSequence()) {
      jSeq.push_back({{"name", desc.name},
                          {"strParam", desc.strParam},
                          {"intParam", desc.intParam},
                          {"intParam2", desc.intParam2},
                          {"floatParam", desc.floatParam}});
    }
    if (!jSeq.empty()) jItem["sequence"] = jSeq;

    jArray.push_back(jItem);
  }

  std::ofstream out(filename);
  if (out.is_open()) {
    out << jArray.dump(4);
    spdlog::info("GameInit:ItemLoader: Saved to file {}", filename);
  }
}

inline void loadItems(const std::string &filename = "items.json") {
  std::ifstream file(filename);

  // 如果文件不存在，先生成默认数据保存到硬盘，再读取
  if (!file.is_open()) {
    spdlog::info("GameInit:ItemLoader: Failed to load {}, generate default items", filename);
    generateDefaultItems();
    saveItems(filename);
    return;
  }

  try {
    json j;
    file >> j;

    // 先清空内存的旧数据
    ItemRegistry::inst().clear();

    for (const auto &item_json : j) {
      std::string id = item_json["id"];

      auto jProps = item_json["props"];
      ItemProps props;
      props.pickable = jProps.value("pickable", false);
      props.blocks = jProps.value("blocks", false);
      props.useable = jProps.value("useable", true);
      props.texture_name = jProps.value("texture_name", "");

      auto smart_item = std::make_unique<SmartItem>(id, props);

      if (item_json.contains("effects")) {
        for (const auto &jEff : item_json["effects"]) {
          ItemEffect eff;
          eff.type = parseEffectType(jEff.value("type", "ModifyStat"));
          eff.target = jEff.value("target", "");
          eff.value = jEff.value("value", 0.0);
          smart_item->addEffect(eff);
        }
      }

      if (item_json.contains("sequence")) {
        std::vector<ActionDescriptor> seq;
        for (const auto &jSeq : item_json["sequence"]) {
          ActionDescriptor desc;
          desc.name = jSeq.value("name", "");
          desc.strParam = jSeq.value("strParam", "");
          desc.intParam = jSeq.value("intParam", 0);
          desc.intParam2 = jSeq.value("intParam2", 0);
          desc.floatParam = jSeq.value("floatParam", 0.0f);
          seq.push_back(desc);
        }
        smart_item->setSequence(seq);
      }

      ItemRegistry::inst().register_item(std::move(smart_item));
    }
    spdlog::info("ItemLoader: Loaded {} items from {}", j.size(), filename);
  } catch (const std::exception &e) {
    spdlog::error("ItemLoader: Failed to parse JSON: {}", e.what());
  }
}

inline void saveAgents(const std::vector<std::unique_ptr<Agent>>& agents, const std::string& filename = "agents.json") {
  json jArray = json::array();
  for (const auto& a : agents) {
    json j;
    j["name"] = a->getName();
    j["id"] = a->getId();
    j["x"] = a->getCharacter().getLoc().first;
    j["y"] = a->getCharacter().getLoc().second;
    j["level"] = a->getCharacter().getLevel();
    j["ai_type"] = static_cast<int>(a->getAIType());
    j["texture_name"] = a->getTextureName();
    if (a->getAIType() == AIType::Static) {
      json jSeq = json::array();
      for (const auto& desc : a->getStaticAISequence()) {
        jSeq.push_back({
          {"name", desc.name},
          {"strParam", desc.strParam},
          {"intParam", desc.intParam},
          {"intParam2", desc.intParam2},
          {"floatParam", desc.floatParam}
        });
      }
      if (!jSeq.empty()) j["sequence"] = jSeq;
    }
    jArray.push_back(j);
  }
  std::ofstream out(filename);
  if (out.is_open()) {
    out << jArray.dump(4);
    spdlog::info("GameInit:AgentLoader: Saved to file {}", filename);
  }
}

inline void loadAgents(std::vector<std::unique_ptr<Agent>>& agents, IRender* render, const std::string& filename = "agents.json") {
  std::ifstream file(filename);
  if (!file.is_open()) return;

  try {
    json jArray;
    file >> jArray;
    agents.clear(); // 清空旧小人
    for (const auto& jItem : jArray) {
      std::string name = jItem.value("name", "Unknown");
      std::string id = jItem.value("id", "npc_unknown");
      int x = jItem.value("x", 0);
      int y = jItem.value("y", 0);
      int level = jItem.value("level", 0);
      AIType type = static_cast<AIType>(jItem.value("ai_type", 1));
      std::string tex = jItem.value("texture_name", "character.png");

      auto new_agent = std::make_unique<Agent>(name, id, x, y, type, tex);
      new_agent->getCharacter().setLevel(level);

      // 如果是Static ai，解析序列数据
      if (type == AIType::Static && jItem.contains("sequence")) {
        std::vector<ActionDescriptor> seq;
        for (const auto& jSeq : jItem["sequence"]) {
          ActionDescriptor desc;
          desc.name = jSeq.value("name", "");
          desc.strParam = jSeq.value("strParam", "");
          desc.intParam = jSeq.value("intParam", 0);
          desc.intParam2 = jSeq.value("intParam2", 0);
          desc.floatParam = jSeq.value("floatParam", 0.0f);
          spdlog::info("Load Agents: action name: {}", desc.name);
          seq.push_back(desc);
        }
        new_agent->setStaticAISequence(seq);
      }

      agents.push_back(std::move(new_agent));
      render->loadAgentTexture(tex);
    }
    spdlog::info("GameInit:AgentLoader: Loaded {} agents from {} successfully",agents.size(),filename);
  } catch (const std::exception& e) {
    spdlog::error("GameInit:AgentLoader: Failed to parse JSON: {}", e.what());
  }
}


inline void saveTriggers(const std::string& filename = "triggers.json") {
  json jArray = json::array();
  for (const auto& [id, def] : TriggerManager::inst().getAllTriggers()) {
    json j;
    j["id"] = def.id;
    j["level"] = def.level;
    j["x"] = def.x;
    j["y"] = def.y;
    j["level"] = def.level;
    j["type"] = static_cast<int>(def.type);
    j["target_id"] = def.target_id;
    jArray.push_back(j);
  }
  std::ofstream out(filename);
  if (out.is_open()) {
    out << jArray.dump(4);
    spdlog::info("GameInit:TriggerLoader: Saved triggers to {}", filename);
  }
}

inline void loadTriggers(const std::string& filename = "triggers.json") {
  std::ifstream file(filename);
  if (!file.is_open()) {
    //第一次是没有的，如果找不到不管
    spdlog::info("GameInit:TriggerLoader: {} not found, skipping.", filename);
    return;
  }

  try {
    json jArray;
    file >> jArray;
    TriggerManager::inst().clear();
    for (const auto& jItem : jArray) {
      TriggerDef def;
      def.id = jItem.value("id", "");
      def.level = jItem.value("level", 0);
      def.x = jItem.value("x", 0);
      def.y = jItem.value("y", 0);
      def.level = jItem.value("level", 0);
      def.type = static_cast<TriggerType>(jItem.value("type", 0));
      def.target_id = jItem.value("target_id", "");

      if (!def.id.empty()) {
        TriggerManager::inst().addTrigger(def);
      }
    }
    spdlog::info("GameInit:TriggerLoader: Loaded {} triggers from {}", jArray.size(), filename);
  } catch (const std::exception& e) {
    spdlog::error("GameInit:TriggerLoader: Failed to parse JSON: {}", e.what());
  }
}
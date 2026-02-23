//
// Created by jacob on 25-10-24.
//
#pragma once

#include "Character.h"
#include "Config.h"
#include "ItemRegistry.h"
#include "SmartItem.h"
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
inline void generateDefaultItems() {
  ItemRegistry::inst().clear();

  // 食物
  auto food = std::make_unique<SmartItem>("food", ItemProps{false, false, true, "food.png"});
  food->addEffect({EffectType::ModifyStat, "hunger", static_cast<double>(-Cfg::item::food_calories)});
  food->setSequence({{"MoveToTarget", 0, ""}, {"Interact", 0, ""}});
  ItemRegistry::inst().register_item(std::move(food));

  // 床
  auto bed = std::make_unique<SmartItem>("bed", ItemProps{false, false, true, "bed.png"});
  bed->addEffect({EffectType::ModifyStat, "fatigue", -100.0});
  bed->setSequence({{"MoveToTarget", 0, ""}, {"Interact", 0, ""}, {"Wait", 100, ""}});
  ItemRegistry::inst().register_item(std::move(bed));

  // 电脑
  auto computer = std::make_unique<SmartItem>("computer", ItemProps{false, false, true, "computer.png"});
  computer->addEffect({EffectType::ModifyStat, "boredom", static_cast<double>(-Cfg::item::play_computer_entertain)});
  computer->addEffect({EffectType::AddMemory, "$RANDOM_COMPUTER$", 0.0});
  int useTicks = AkRandom::randint(Cfg::time::min_use_computer, Cfg::time::max_use_computer) * Cfg::core::ticks_per_sec;
  computer->setSequence({{"MoveToTarget", 0, ""}, {"Interact", 0, ""}, {"Wait", useTicks, ""}});
  ItemRegistry::inst().register_item(std::move(computer));

  std::cout << "[ItemLoader] Generated default items in memory.\n";
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
                      {"intParam", desc.intParam},
                      {"strParam", desc.strParam}});
    }
    if (!jSeq.empty()) jItem["sequence"] = jSeq;

    jArray.push_back(jItem);
  }

  std::ofstream out(filename);
  if (out.is_open()) {
    out << jArray.dump(4);
    std::cout << "[ItemLoader] Saved items to " << filename << std::endl;
  }
}

inline void loadItems(const std::string &filename = "items.json") {
  std::ifstream file(filename);

  // 如果文件不存在，先生成默认数据保存到硬盘，再读取
  if (!file.is_open()) {
    std::cout << "[ItemLoader] " << filename << " not found, triggering fallback..." << std::endl;
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
          desc.intParam = jSeq.value("intParam", 0);
          desc.strParam = jSeq.value("strParam", "");
          seq.push_back(desc);
        }
        smart_item->setSequence(seq);
      }

      ItemRegistry::inst().register_item(std::move(smart_item));
    }
    std::cout << "[ItemLoader] Loaded " << j.size() << " items from " << filename << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "[ItemLoader] Failed to parse JSON: " << e.what() << std::endl;
  }
}
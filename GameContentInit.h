//
// Created by jacob on 25-10-24.
//
#pragma once

#include "Character.h"
#include "Config.h"
#include "ItemRegistry.h"
#include "SmartItem.h"

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

inline void register_default_items() {
  // 食物
  auto food = std::make_unique<SmartItem>(
      "food", ItemProps{0, 0, false, false, true});
  food->addEffect({EffectType::ModifyStat, "hunger", static_cast<double>(-Cfg::item::food_calories)});
  food->setSequence({{"MoveToTarget", 0, ""},
                     {"Interact", 0, ""}});
  ItemRegistry::inst().register_item(std::move(food));

  // 床
  auto bed = std::make_unique<SmartItem>(
      "bed", ItemProps{0, 0, false, false, true});
  // TODO: 我在想这里持续性使用床是否是可行的。
  bed->addEffect({EffectType::ModifyStat, "fatigue", -1.0});
  // value>0 代表 true
  //bed->addEffect({EffectType::SetState, "sleeping", 1.0});
  bed->setSequence({{"MoveToTarget", 0, ""},
                    {"Interact", 0, ""}});
  ItemRegistry::inst().register_item(std::move(bed));

  // 电脑
  auto computer = std::make_unique<SmartItem>(
      "computer", ItemProps{0, 0, false, false, true});
  computer->addEffect({EffectType::ModifyStat, "boredom", static_cast<double>(-Cfg::item::play_computer_entertain)});
  computer->addEffect({EffectType::AddMemory, "$RANDOM_COMPUTER$", 0.0});
  // 从 Cfg 读取随机时间
  int useTicks = AkRandom::randint(Cfg::time::min_use_computer, Cfg::time::max_use_computer) * Cfg::core::ticks_per_sec;
  computer->setSequence({{"MoveToTarget", 0, ""},
                         {"Interact", 0, ""},
                         {"Wait", useTicks, ""}});
  ItemRegistry::inst().register_item(std::move(computer));
}

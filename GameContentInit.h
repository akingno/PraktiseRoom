//
// Created by jacob on 25-10-24.
//
#pragma once

#include "Character.h"
#include "Config.h"
#include "ItemBuilder.h"
#include "ItemRegistry.h"

static const std::vector<std::pair<std::string, double>> kComputerFeed = {
  {"看了一段搞笑视频", +0.5},
  {"读到一条尖刻评论", -1.0},
  {"学到一个新技巧", +1.0},
  {"看到坏消息", -2.0},
  {"完成日常签到",0.0},
  {"听了点轻音乐",+0.5},
  {"刷到宠物视频",+1.0},
  {"被广告刷屏",-1.0},
  {"读到励志故事",+2.0},
  {"无聊地发呆",0.0},
};

inline void register_default_items() {
  // 食物：被使用时喂饱，并刷新房间里的 food tile
  auto food = ItemBuilder("food")
  .useable(true)
  .onUse([](UseCtx& ux, int, int){
    if (!ux.ch.eatAvailable()) return false;
    if (!ux.items.hasFood())   return false;
    ux.ch.eat(FOOD_CALORIES);
    ux.items.consumeFood();
    return true;
  }).build();

  ItemRegistry::inst().register_item(std::move(food));

  // 床：被使用时进入睡眠状态
  auto bed = ItemBuilder("bed")
  .useable(true)
  .onUse([](UseCtx& ux, int, int){
    if (!ux.ch.isSleeping()) ux.ch.setSleeping(true);
    return true;
  }).build();

  ItemRegistry::inst().register_item(std::move(bed));

  auto computer = ItemBuilder("computer")
  .useable(true)
  .onUse([](UseCtx& ux, int , int ){
        // 随机抽一条信息
        int idx = Random::randint(0, static_cast<int>(kComputerFeed.size()) - 1);
        const auto& [content, val] = kComputerFeed[idx];
        // 记入 short_memory
        ux.ch.short_memory().add(content, val);
        std::cout<<"Memory added: "<<content<<std::endl;
        return true;
      })
  .build();

  ItemRegistry::inst().register_item(std::move(computer));

}

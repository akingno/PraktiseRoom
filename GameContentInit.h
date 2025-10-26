//
// Created by jacob on 25-10-24.
//
#pragma once

#include "Character.h"
#include "Config.h"
#include "ItemBuilder.h"
#include "ItemRegistry.h"

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
  .onUse([](UseCtx& ux, int, int) {
    //TODO:目前是个空方法
    return true;
  }).build();

  ItemRegistry::inst().register_item(std::move(computer));

}

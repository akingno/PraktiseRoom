//
// Created by jacob on 25-10-24.
//

#include "Character.h"
#include "Config.h"
#include "ItemBuilder.h"
#include "ItemRegistry.h"
#include "Room.h"

inline void register_default_items() {
  // 食物：被使用时喂饱，并刷新房间里的 food tile
  auto food = ItemBuilder("food")
    .useable(true)
    .pickable(false)
    .blocks(false)
    .onUse([](Character& ch, Room& room, int, int){
      ch.eat(FOOD_CALORIES);
      room.consumeFood();
      return true;
    }).build();

  ItemRegistry::inst().register_item(std::move(food));

  // 床：被使用时进入睡眠状态
  auto bed = ItemBuilder("bed")
    .useable(true)
    .blocks(false)
    .onUse([](Character& ch, Room&, int, int){
    // 到达床格：维持睡眠标记（幂等）
      if (!ch.isSleeping()) ch.setSleeping(true);
      return true;
    }).build();

  ItemRegistry::inst().register_item(std::move(bed));
}

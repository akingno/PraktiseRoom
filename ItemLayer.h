//
// Created by jacob on 25-10-24.
//

#ifndef ITEMLAYER_H
#define ITEMLAYER_H

#include <unordered_map>
#include <optional>
#include "Config.h"
#include "Item.h"
class Room;

class ItemLayer {

public:
  ItemLayer() = default;
  // 基础操作
  bool place(const ItemId& id, int x, int y); //放置
  bool removeAt(int x, int y); //移除
  bool hasAt(int x, int y) const;
  std::optional<ItemId> idAt(int x, int y) const;

  // 查询一类物品
  std::optional<Pos> firstOf(const ItemId& id) const;
  bool anyOf(const ItemId& id) const { return firstOf(id).has_value(); }

  bool hasFood() const  { return anyOf("food"); }
  bool hasBed()  const  { return anyOf("bed");  }
  bool hasComputer() const  { return anyOf("computer");  }

  std::optional<Pos> foodPos() const { return firstOf("food"); }
  std::optional<Pos> bedPos()  const { return firstOf("bed");  }
  std::optional<Pos> computerPos() const { return firstOf("computer"); }

  //生成/消耗
  void ensureFoodSpawned() {
    if (!hasFood()) place("food", FOOD_X, FOOD_Y);
  }
  void consumeFood() { // 找到一个food就删
    if (auto p = foodPos()) removeAt(p->x, p->y);
  }

  void ensureBedPlaced() {
    if (!hasBed()) place("bed", BED_X, BED_Y);
  }

  void ensureComputerPlaced() {
    if (!hasComputer()) place("computer", COMPUTER_X, COMPUTER_Y);
  }

  // 迭代用：给渲染器遍历
  const std::unordered_map<int, ItemId>& items() const { return items_; }
private:
  static int key(int x, int y) { return y * VIEW_W + x; }
  std::unordered_map<int, ItemId> items_; // key= y*W+x
};



#endif //ITEMLAYER_H

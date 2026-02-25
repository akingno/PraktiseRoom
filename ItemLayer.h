//
// Created by jacob on 25-10-24.
//

#ifndef ITEMLAYER_H
#define ITEMLAYER_H

#include "Config.h"
#include "Item.h"
#include <optional>
#include <unordered_map>
class Room;

class ItemLayer {

 public:
  ItemLayer() = default;
  // 基础操作
  bool place(const ItemId &id, int x, int y);// 放置/覆盖
  bool removeAt(int x, int y);               // 移除
  bool hasAt(int x, int y) const;
  std::optional<ItemId> idAt(int x, int y) const;

  // 查询一类物品（第一个/任意一个）
  std::optional<Pos> firstOf(const ItemId &id) const;
  bool anyOf(const ItemId &id) const { return firstOf(id).has_value(); }

  bool hasFood() const { return anyOf("food"); }
  bool hasBed() const { return anyOf("bed"); }
  bool hasComputer() const { return anyOf("computer"); }

  std::optional<Pos> foodPos() const { return firstOf("food"); }
  std::optional<Pos> bedPos() const { return firstOf("bed"); }
  std::optional<Pos> computerPos() const { return firstOf("computer"); }

  void initDefaultLayout();

  // 迭代用：给渲染器遍历
  const std::unordered_map<int, ItemId> &items() const { return items_; }

 private:
  static int key(int x, int y) { return y * Cfg::room::view_w + x; }
  std::unordered_map<int, ItemId> items_;// key= y*W+x
};

#endif//ITEMLAYER_H

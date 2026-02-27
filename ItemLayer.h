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

  bool place(const ItemId &id, int x, int y);// 放置/覆盖
  bool removeAt(int x, int y); // 移除
  std::optional<ItemId> idAt(int x, int y) const;


  void initDefaultLayout();
  void clear();
  void saveToFile(const std::string& filename = "world.json") const;
  void loadFromFile(const std::string& filename = "world.json");

  // 给渲染器遍历
  const std::unordered_map<int, ItemId> &items() const { return items_; }

 private:
  static int key(int x, int y) { return y * Cfg::room::view_w + x; }
  std::unordered_map<int, ItemId> items_;// key= y*W+x
};

#endif//ITEMLAYER_H

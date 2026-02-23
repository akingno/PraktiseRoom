//
// Created by jacob on 25-10-24.
//

#ifndef ITEM_H
#define ITEM_H

#include <functional>
#include <string>

class Room;
class ItemLayer;
class Character;

using ItemId = std::string;

struct ItemProps {
  bool pickable = false;
  bool blocks = false;// 是否阻挡移动
  bool useable = false;

  std::string texture_name = "";
};

class Item {
 public:
  explicit Item(ItemId id, ItemProps props) : id_(std::move(id)), props_(props) {}
  virtual ~Item() = default;

  const ItemId &id() const { return id_; }
  const ItemProps &props() const { return props_; }

 private:
  ItemId id_;
  ItemProps props_;
};

#endif//ITEM_H

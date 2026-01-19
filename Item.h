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

struct UseCtx {
  Character& ch;
  Room& room;
  ItemLayer& items;
};

using ItemId = std::string;

struct ItemProps {
  int  light     = 0;
  int  hardness  = 0;
  bool pickable  = false;
  bool blocks    = false;  // 是否阻挡移动
  bool useable= false;  // 可被使用
};

class Item {
public:
  explicit Item(ItemId id, ItemProps props) : id_(std::move(id)), props_(props) {}
  virtual ~Item() = default;

  const ItemId& id()           const { return id_; }
  const ItemProps& props()     const { return props_; }

  // 定义使用该物品时的行为
  // 返回true表示使用成功
  virtual bool onUse(UseCtx& ux, int x, int y) { (void)ux;(void)x;(void)y; return false; }

private:
  ItemId id_;
  ItemProps props_;
};



#endif //ITEM_H

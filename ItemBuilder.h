//
// Created by jacob on 25-10-24.
//

#ifndef ITEMBUILDER_H
#define ITEMBUILDER_H



#include "Item.h"

#include <memory>

class ItemBuilder {
public:
  explicit ItemBuilder(ItemId id) : id_(std::move(id)) {}

  ItemBuilder& light(int v)        { props_.light = v; return *this; }
  ItemBuilder& hardness(int v)     { props_.hardness = v; return *this; }
  ItemBuilder& pickable(bool v)    { props_.pickable = v; return *this; }
  ItemBuilder& blocks(bool v)      { props_.blocks = v; return *this; }
  ItemBuilder& useable(bool v)     { props_.useable = v; return *this; }

  // 提供自定义 onUse
  ItemBuilder& onUse(std::function<bool(Character&, Room&, int,int)> fn) {
    on_use_ = std::move(fn); return *this;
  }

  // 构建：若提供 onUse 则生成带回调的派生类
  std::unique_ptr<Item> build() const;

private:
  ItemId id_;
  ItemProps props_;
  std::function<bool(Character&, Room&, int,int)> on_use_;
};



#endif //ITEMBUILDER_H

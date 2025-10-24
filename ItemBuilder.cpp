//
// Created by jacob on 25-10-24.
//

#include "ItemBuilder.h"


namespace {
class CustomItem : public Item {
public:
  CustomItem(ItemId id, ItemProps props, std::function<bool(Character&, Room&, int,int)> fn)
    : Item(std::move(id), props), fn_(std::move(fn)) {}

  bool onUse(Character& ch, Room& room, int x, int y) override {
    return fn_ ? fn_(ch, room, x, y) : false;
  }


private:
  std::function<bool(Character&, Room&, int,int)> fn_;
};
}

std::unique_ptr<Item> ItemBuilder::build() const {
  if (on_use_) return std::make_unique<CustomItem>(id_, props_, on_use_);
  return std::make_unique<Item>(id_, props_);
}
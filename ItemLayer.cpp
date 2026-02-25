//
// Created by jacob on 25-10-24.
//

#include "ItemLayer.h"

bool ItemLayer::place(const ItemId &id, int x, int y) {
  if (x < 0 || x >= Cfg::room::view_w || y < 0 || y >= Cfg::room::view_h) return false;
  items_[key(x, y)] = id;
  return true;
}
bool ItemLayer::removeAt(int x, int y) {
  return items_.erase(key(x, y)) > 0;
}
bool ItemLayer::hasAt(int x, int y) const {
  return items_.find(key(x, y)) != items_.end();
}
std::optional<ItemId> ItemLayer::idAt(int x, int y) const {
  auto it = items_.find(key(x, y));
  if (it == items_.end()) return std::nullopt;
  return it->second;
}
std::optional<Pos> ItemLayer::firstOf(const ItemId &id) const {
  for (auto &[k, vid] : items_) {
    if (vid == id) return Pos{k % Cfg::room::view_w, k / Cfg::room::view_w};
  }
  return std::nullopt;
}
void ItemLayer::initDefaultLayout() {
  place("food", Cfg::room::food_x, Cfg::room::food_y);
  place("bed", Cfg::room::bed_x, Cfg::room::bed_y);
  place("computer", Cfg::room::computer_x, Cfg::room::computer_y);
}
//
// Created by jacob on 25-10-24.
//

#include "ItemLayer.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool ItemLayer::place(const ItemId &id, int x, int y) {
  if (x < 0 || x >= Cfg::room::view_w || y < 0 || y >= Cfg::room::view_h) return false;
  items_[key(x, y)] = id;
  return true;
}
bool ItemLayer::removeAt(int x, int y) {
  return items_.erase(key(x, y)) > 0;
}

std::optional<ItemId> ItemLayer::idAt(int x, int y) const {
  auto it = items_.find(key(x, y));
  if (it == items_.end()) return std::nullopt;
  return it->second;
}

void ItemLayer::initDefaultLayout() {
  place("food", Cfg::room::food_x, Cfg::room::food_y);
  place("bed", Cfg::room::bed_x, Cfg::room::bed_y);
  place("computer", Cfg::room::computer_x, Cfg::room::computer_y);
}

void ItemLayer::clear() {
  items_.clear();
}
void ItemLayer::saveToFile(const std::string& filename) const {
  json jArray = json::array();

  // 遍历地图上所有的物品
  for (const auto& [k, id] : items_) {
    int x = k % Cfg::room::view_w;
    int y = k / Cfg::room::view_w;
    // 构造单个物品的json
    json jItem;
    jItem["id"] = id;
    jItem["x"] = x;
    jItem["y"] = y;

    jArray.push_back(jItem);
  }

  std::ofstream out(filename);
  if (out.is_open()) {
    out << jArray.dump(4);
    std::cout << "ItemLayer: Saved world layout to " << filename << std::endl;
  } else {
    std::cerr << "ItemLayer: Error saving world layout to " << filename << std::endl;
  }
}

void ItemLayer::loadFromFile(const std::string& filename) {
  std::ifstream file(filename);

  // 如果文件不存在，则调用默认布局并保存
  if (!file.is_open()) {
    std::cout << "[ItemLayer] " << filename << " not found, generating default layout..." << std::endl;
    clear();
    initDefaultLayout();
    saveToFile(filename);
    return;
  }

  try {
    json jArray;
    file >> jArray;

    clear();
    for (const auto& jItem : jArray) {
      std::string id = jItem.value("id", "");
      int x = jItem.value("x", -1);
      int y = jItem.value("y", -1);

      if (!id.empty() && x >= 0 && y >= 0) {
        place(id, x, y);
      }
    }
    std::cout << "ItemLayer: Loaded layout from " << filename << std::endl;
  } catch (const std::exception& e) {
    std::cerr << "ItemLayer: Failed to parse JSON: " << e.what() << std::endl;
  }
}
//
// Created by jacob on 25-10-24.
//

#include "ItemLayer.h"

#include "spdlog/spdlog.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool ItemLayer::place(const ItemId &id, int x, int y) {
  if (x < 0 || x >= width_ || y < 0 || y >= height_) return false;
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
  if (level_id_ == 0) {
    place("food", Cfg::room::food_x, Cfg::room::food_y);
    place("bed", Cfg::room::bed_x, Cfg::room::bed_y);
    place("computer", Cfg::room::computer_x, Cfg::room::computer_y);
  }
}

void ItemLayer::clear() {
  items_.clear();
}

void ItemLayer::saveToFile(const std::string& filename) const {
  json jArray = json::array();

  // 遍历地图上所有的物品
  for (const auto& [k, id] : items_) {
    int x = k % width_;
    int y = k / width_;
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
    spdlog::info("ItemLayer: Saved world layout to " + filename);
  } else {
    spdlog::warn("ItemLayer: Failed to open file " + filename);
  }
}

void ItemLayer::loadFromFile(const std::string& filename) {
  std::ifstream file(filename);

  // 如果文件不存在，则调用默认布局并保存
  if (!file.is_open()) {
    spdlog::info("ItemLayer: Failed to load default layout, generating default layout...");
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
    spdlog::info("ItemLayer: Loaded layout from "+filename);
  } catch (const std::exception& e) {
    spdlog::error("ItemLayer: Failed to parse JSON: {} ", e.what());
  }
}
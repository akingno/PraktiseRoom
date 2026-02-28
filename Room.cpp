//
// Created by jacob on 2025/9/14.
//

#include "Room.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Room::Room(int w, int h) : width_(w), height_(h), _door{w/2, 0} {
  _blocks.assign(width_ * height_, "grass");
}

TileId Room::getBlocksType(const int x, const int y) const {
  if (x < 0 || x >= width_ || y < 0 || y >= height_) return "wall"; // 越界视为墙壁
  return _blocks[y * width_ + x];
}

bool Room::setBlock(const TileId& typeId, const int x, const int y) {
  if (x < 0 || x >= width_ || y < 0 || y >= height_) return false;
  _blocks[y * width_ + x] = typeId;
  return true;
}

bool Room::isPassable(int x, int y) const{
  // 越界一律不可走
  if (x < 0 || x >= width_ || y < 0 || y >= height_) return false;

  TileId id = getBlocksType(x, y);
  const TerrainDef* def = TerrainRegistry::inst().get(id);
  if (def) { // 如果存在
    return !def->blocks;
  }
  return true;//默认可走
}
void Room::initDefaultLayout() {
  _blocks.assign(width_ * height_, "grass");
  for (int y = 0; y < height_; ++y) {
    setBlock("wall", 0, y);
    setBlock("wall", width_ - 1, y);
  }
  for (int x = 0; x < width_; ++x) {
    setBlock("wall", x, 0);
    setBlock("wall", x, height_ - 1);
  }
  setBlock("door", _door.x, _door.y);
}

void Room::saveToFile(const std::string& filename) const {
  json j;
  j["width"] = width_;
  j["height"] = height_;
  j["door"] = { {"x", _door.x}, {"y", _door.y} };
  j["tiles"] = _blocks;

  std::ofstream out(filename);
  if (out.is_open()) {
    out << j.dump(4);
    std::cout << "Room: Saved terrain to " << filename << std::endl;
  }
}

void Room::loadFromFile(const std::string& filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    initDefaultLayout();
    saveToFile(filename);
    return;
  }
  json j; file >> j;
  width_ = j.value("width", width_);
  height_ = j.value("height", height_);
  if (j.contains("door")) {
    _door.x = j["door"].value("x", _door.x);
    _door.y = j["door"].value("y", _door.y);
  }
  if (j.contains("tiles")) {
    _blocks = j["tiles"].get<std::vector<TileId>>();
  }
}
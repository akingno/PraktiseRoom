//
// Created by jacob on 2025/9/14.
//

#ifndef ROOM_TEMP__ROOM_H_
#define ROOM_TEMP__ROOM_H_

#include "Block.h"
#include <vector>

constexpr int VIEW_W = 120;
constexpr int VIEW_H = 40;
constexpr int DOOR_X = 80;
constexpr int DOOR_Y = 0;
constexpr int FOOD_X = 80;
constexpr int FOOD_Y = 1;

struct Pos { int x, y; };

class Room {
 public:
  Room();
  [[nodiscard]] TileType getBlocksType(int x, int y) const;
  bool setBlock(TileType type, int x, int y);

  const Pos& doorPos()  const { return _door; }
  const Pos& foodPos()  const { return _food; }
  bool hasFood()        const { return has_food_; }
  bool hasFoodAt(int x,int y) const { return has_food_ && x==_food.x && y==_food.y; }

  void ensureFoodSpawned() { if (!has_food_) { _food = {_door.x, _door.y + 1}; has_food_ = true; } }
  void consumeFood() {
    has_food_ = false;
    setBlock(TileType::Grass, _food.x, _food.y);

    //又生成食物
    has_food_ = true;
    setBlock(TileType::FOOD, _food.x, _food.y);
  }

 private:
  std::vector<Block>  _blocks;
  Pos  _door{DOOR_X, DOOR_Y};
  Pos  _food{FOOD_X, FOOD_Y};
  bool has_food_ = true;

};

#endif//ROOM_TEMP__ROOM_H_

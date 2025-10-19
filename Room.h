//
// Created by jacob on 2025/9/14.
//

#ifndef ROOM_TEMP_ROOM_H_
#define ROOM_TEMP_ROOM_H_

#include "Block.h"
#include <vector>
#include "Config.h"


struct Pos { int x, y; };

class Room {
 public:
  Room();
  [[nodiscard]] TileType getBlocksType(int x, int y) const;
  bool setBlock(TileType type, int x, int y);

  bool isPassable(int x, int y) const;


  const Pos& doorPos()  const { return _door;}
  const Pos& foodPos()  const { return _food;}
  const Pos& bedPos()   const { return _bed;}



  bool hasFood()        const { return has_food_; }
  bool hasFoodAt(int x,int y) const { return has_food_ && x==_food.x && y==_food.y; }
  bool hasBed()         const { return true; }
  bool isBedAt(int x,int y) const { return x==_bed.x && y==_bed.y; }

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
  Pos  _bed {BED_X,  BED_Y};
  bool has_food_ = true;

};

#endif//ROOM_TEMP_ROOM_H_

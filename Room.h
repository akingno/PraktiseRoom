//
// Created by jacob on 2025/9/14.
//

#ifndef ROOM_TEMP__ROOM_H_
#define ROOM_TEMP__ROOM_H_

#include "Block.h"
#include <vector>

constexpr int VIEW_W = 120;
constexpr int VIEW_H = 40;
class Room {
 public:
  Room();
  TileType getBlocksType(int x, int y);
  bool setBlock(TileType type, int x, int y);

 private:
  std::vector<Block> _blocks;

};

#endif//ROOM_TEMP__ROOM_H_

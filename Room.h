//
// Created by jacob on 2025/9/14.
//

#ifndef ROOM_TEMP_ROOM_H_
#define ROOM_TEMP_ROOM_H_

#include "Block.h"
#include <vector>
#include "Config.h"



class Room {
 public:
  Room();
  [[nodiscard]] TileType getBlocksType(int x, int y) const;
  bool setBlock(TileType type, int x, int y);

  bool isPassable(int x, int y) const;

  const Pos& doorPos()  const { return _door;}


 private:
  std::vector<Block>  _blocks;
  Pos  _door{DOOR_X, DOOR_Y};

};

#endif//ROOM_TEMP_ROOM_H_

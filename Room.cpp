//
// Created by jacob on 2025/9/14.
//

#include "Room.h"
TileType Room::getBlocksType(int x, int y) {
  if (x < 0 || x >= VIEW_W || y < 0 || y >= VIEW_H) {
    return TileType::WallH;
  }
  return _blocks[y * VIEW_W + x].getTileType();
}
bool Room::setBlock(TileType type, int x, int y) {
  if (x < 0 || x >= VIEW_W || y < 0 || y >= VIEW_H) return false;
  _blocks[y * VIEW_W + x].setTileType(type);
  return true;
}
Room::Room() {
  _blocks.assign(VIEW_W * VIEW_H, Block(TileType::Grass));
  // 四周置墙
  for (int x=0; x<VIEW_W; ++x) {
    _blocks[x].setTileType(TileType::WallH);
    _blocks[(VIEW_H-1) * VIEW_W + x].setTileType(TileType::WallH);
  }
  for (int y=0; y<VIEW_H; ++y) {
    _blocks[y * VIEW_W + 0].setTileType(TileType::WallV);
    _blocks[y * VIEW_W + (VIEW_W-1)].setTileType(TileType::WallV);
  }
}

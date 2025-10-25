//
// Created by jacob on 2025/9/14.
//

#include "Room.h"
TileType Room::getBlocksType(const int x, const int y) const{
  if (x < 0 || x >= VIEW_W || y < 0 || y >= VIEW_H) {
    return TileType::WallH;
  }
  return _blocks[y * VIEW_W + x].getTileType();
}
bool Room::setBlock(const TileType type,const int x,const int y) {
  if (x < 0 || x >= VIEW_W || y < 0 || y >= VIEW_H) return false;
  _blocks[y * VIEW_W + x].setTileType(type);
  return true;
}
Room::Room() {
  _blocks.assign(VIEW_W * VIEW_H, Block(TileType::Grass));
  // 四周置墙
  for (int y=0; y<VIEW_H; ++y) {
    _blocks[y * VIEW_W + 0].setTileType(TileType::WallV);
    _blocks[y * VIEW_W + (VIEW_W-1)].setTileType(TileType::WallV);
  }
  for (int x=0; x<VIEW_W; ++x) {
    _blocks[x].setTileType(TileType::WallH);
    _blocks[(VIEW_H-1) * VIEW_W + x].setTileType(TileType::WallH);
  }

  // 物品：床、墙、食物等
  _blocks[DOOR_X].setTileType(TileType::DOOR);
  _blocks[VIEW_W+DOOR_X].setTileType(TileType::FOOD);
  _blocks[_bed.y * VIEW_W + _bed.x].setTileType(TileType::BED);

}

bool Room::isPassable(int x, int y) const{
  // 越界一律不可走
  if (x < 0 || x >= VIEW_W || y < 0 || y >= VIEW_H) return false;

  TileType t = getBlocksType(x, y);
  switch (t) {
    case TileType::WallV:
    case TileType::WallH:
      return false;
    default:
      return true;
  }
}

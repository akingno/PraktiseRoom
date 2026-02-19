//
// Created by jacob on 2025/9/14.
//

#include "Room.h"
TileType Room::getBlocksType(const int x, const int y) const{
  if (x < 0 || x >= Cfg::room::view_w || y < 0 || y >= Cfg::room::view_h) {
    return TileType::WallH;
  }
  return _blocks[y * Cfg::room::view_w + x].getTileType();
}
bool Room::setBlock(const TileType type,const int x,const int y) {
  if (x < 0 || x >= Cfg::room::view_w || y < 0 || y >= Cfg::room::view_h) return false;
  _blocks[y * Cfg::room::view_w + x].setTileType(type);
  return true;
}
Room::Room() {
  _blocks.assign(Cfg::room::view_w * Cfg::room::view_h, Block(TileType::Grass));
  // 四周置墙
  for (int y=0; y<Cfg::room::view_h; ++y) {
    _blocks[y * Cfg::room::view_w + 0].setTileType(TileType::WallV);
    _blocks[y * Cfg::room::view_w + (Cfg::room::view_w-1)].setTileType(TileType::WallV);
  }
  for (int x=0; x<Cfg::room::view_w; ++x) {
    _blocks[x].setTileType(TileType::WallH);
    _blocks[(Cfg::room::view_h-1) * Cfg::room::view_w + x].setTileType(TileType::WallH);
  }

  // 物品：床、墙、食物等
  _blocks[Cfg::room::door_x].setTileType(TileType::DOOR);

}

bool Room::isPassable(int x, int y) const{
  // 越界一律不可走
  if (x < 0 || x >= Cfg::room::view_w || y < 0 || y >= Cfg::room::view_h) return false;

  TileType t = getBlocksType(x, y);
  switch (t) {
    case TileType::WallV:
    case TileType::WallH:
      return false;
    default:
      return true;
  }
}

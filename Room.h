//
// Created by jacob on 2025/9/14.
//

#ifndef ROOM_TEMP_ROOM_H_
#define ROOM_TEMP_ROOM_H_

#include "Config.h"
#include "TerrainRegistry.h"
#include "tools/AStarPathfinder.h"
#include <vector>

class Room {
 public:
  Room(int w, int h);
  int getWidth() const { return width_; }
  int getHeight() const { return height_; }

  TileId getBlocksType(int x, int y) const;
  bool setBlock(const TileId& typeId, int x, int y);;

  bool isPassable(int x, int y) const;

  const Pos& doorPos()  const { return _door;}

  void saveToFile(const std::string& filename = "room_map.json") const;
  void loadFromFile(const std::string& filename = "room_map.json");
  void initDefaultLayout();

  IPathfinder* getPathfinder() const { return pathfinder_.get(); }

 private:
  int width_;
  int height_;
  std::vector<TileId> _blocks;
  Pos _door;

  std::unique_ptr<AStarPathfinder> pathfinder_;


  void buildPathfinder();
};

#endif//ROOM_TEMP_ROOM_H_

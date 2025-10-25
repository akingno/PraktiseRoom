//
// Created by jacob on 25-9-25.
//

#ifndef ASTARGRID_H
#define ASTARGRID_H

#include "IPathfinder.h"

class AStarPathfinder : public IPathfinder {
public:
  AStarPathfinder(GridSize size, IsPassable passable);

  bool plan_path(int sx,int sy,int tx,int ty,
            std::vector<std::pair<int,int>>& out) override;

private:
  GridSize size_;
  IsPassable passable_;
  std::vector<int> g_, parent_;
  // 这里可以预分配 g、f、parent、open/closed 的数组与小顶堆
};

#endif //ASTARGRID_H

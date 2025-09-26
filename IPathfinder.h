//
// Created by jacob on 25-9-25.
//

#ifndef IPATHFINDER_H
#define IPATHFINDER_H

#include <vector>
#include <functional>
#include <utility>

struct GridSize { int w, h; };

class IPathfinder {
public:
  using IsPassable = std::function<bool(int,int)>;
  virtual ~IPathfinder() = default;

  // 规划从 (sx,sy) 到 (tx,ty)，写入 out（含起点和终点）
  virtual bool plan(int sx,int sy,int tx,int ty,
                    std::vector<std::pair<int,int>>& out) = 0;
};

#endif //IPATHFINDER_H

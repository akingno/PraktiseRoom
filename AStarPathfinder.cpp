//
// Created by jacob on 25-9-25.
//
// AStarGrid.cpp
#include "AStarPathfinder.h"
#include <cmath>
#include <limits>
#include <queue>

namespace {
  inline int idx(int x,int y,int w){ return y*w + x; }
  inline int manh(int x0,int y0,int x1,int y1){ return std::abs(x0-x1) + std::abs(y0-y1); }

  struct OpenNode {
    int i;   // 线性索引
    int f;   // f=g+h，min-heap
    int g;   // g for tie-break
    bool operator<(const OpenNode& o) const {
      if (f != o.f) return f > o.f;      // 反向：小 f 优先
      return g > o.g;                    // 次序：小 g 优先
    }
  };
}

AStarPathfinder::AStarPathfinder(GridSize size, IsPassable passable)
  : size_(size), passable_(std::move(passable)) {}

bool AStarPathfinder::plan_path(int sx,int sy,int tx,int ty,
                     std::vector<std::pair<int,int>>& out) {
  const int W = size_.w, H = size_.h;
  if (sx<0||sx>=W||sy<0||sy>=H||tx<0||tx>=W||ty<0||ty>=H) return false;
  if (!passable_(tx,ty) && !(sx==tx && sy==ty)) return false; // 终点不可达（除非已在终点）

  const int N = W*H;
  g_.assign(N, std::numeric_limits<int>::max());
  parent_.assign(N, -1);


  std::priority_queue<OpenNode> open;
  const int s = idx(sx,sy,W);
  const int t = idx(tx,ty,W);

  g_[s] = 0;
  open.push(OpenNode{ s, manh(sx,sy,tx,ty), 0 });

  auto try_relax = [&](int cx,int cy,int nx,int ny){
    if (nx<0||nx>=W||ny<0||ny>=H) return;
    if (!passable_(nx,ny) && !(nx==tx && ny==ty)) return; // 邻居不可走（允许终点）
    const int ci = idx(cx,cy,W), ni = idx(nx,ny,W);
    const int ng = g_[ci] + 1; // 单步代价
    if (ng < g_[ni]) {
      g_[ni] = ng;
      parent_[ni] = ci;
      const int h = manh(nx,ny,tx,ty);
      open.push(OpenNode{ni, ng + h, ng});
    }
  };

  while (!open.empty()) {
    auto cur = open.top(); open.pop();
    const int ci = cur.i;
    if (ci == t) break;
    const int cx = ci % W, cy = ci / W;

    try_relax(cx,cy, cx+1,cy);
    try_relax(cx,cy, cx-1,cy);
    try_relax(cx,cy, cx,cy+1);
    try_relax(cx,cy, cx,cy-1);
  }

  if (parent_[t] == -1 && s != t) return false; // 无路径

  // 回溯路径（含起点与终点），起点在前
  out.clear();
  for (int p=t; p!=-1; p=parent_[p]) {
    out.emplace_back(p % W, p / W);
    if (p == s) break;
  }
  std::reverse(out.begin(), out.end());
  return true;
}

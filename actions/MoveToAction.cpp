//
// Created by jacob on 26-1-2.
//
#include "MoveToAction.h"
#include "../ActionExecutor.h"
#include "../Agent.h"

    void MoveToAction::onEnter(ActExecutorCtx& ctx, Blackboard& bb){
        // 1. 确定我们要去哪
        std::pair<int, int> targetPos = {-1, -1};
        bool found = false;

        switch (_targetKind) {
          case TargetKind::Character:
            bb.target = bb.target_agent->getCharacter().getLoc();
            targetPos = bb.target;
            found = true;
            break;
          case TargetKind::Food:
            if (auto p = ctx.items.foodPos()) {
              targetPos = {p->x, p->y}; found = true;
            }
            break;
          case TargetKind::Bed:
            if (auto p = ctx.items.bedPos()) {
              targetPos = {p->x, p->y}; found = true;
            }
            break;
          case TargetKind::Computer:
            if (auto p = ctx.items.computerPos()) {
              targetPos = {p->x, p->y}; found = true;
            }
            break;
          case TargetKind::WanderPt: {
            if (ctx.ch.isSleeping()) {
              ctx.ch.setSleeping(false);
          }
            auto startPos = ctx.ch.getLoc();
            int max_tries = 20;  // 尝试次数

            for (int t = 0; t < max_tries; ++t) {
              // 1. 随机取点
              int rx = AkRandom::randint(1, VIEW_W - 2);
              int ry = AkRandom::randint(1, VIEW_H - 2);

              // 2. 检查地形 (是否是墙)
              auto tt = ctx.room.getBlocksType(rx, ry);
              if (tt == TileType::WallV || tt == TileType::WallH) continue;

              // 3. 用 A* 验证可达性 (注意这里用 ctx.pf)
              std::vector<std::pair<int, int>> tmp;
              if (ctx.pf.plan_path(startPos.first, startPos.second, rx, ry, tmp)) {
                targetPos = {rx, ry};
                found = true;
                break;
              }
            }
            break;
          }
            default: break;
        }

        // 2. 设置 Blackboard (类似于原 set_target_and_invalidate)
        if (found) {
            // 如果目标变了，或者原路径无效，重置路径
            if (bb.target != targetPos || !bb.target_valid) {
                bb.target = targetPos;
                bb.target_valid = true;
                bb.target_kind = _targetKind;
                bb.clear_path(); // 目标变了，路径作废，tick里会重算
            }
        } else {
            bb.target_valid = false; // 没找到目标（比如没食物了）
            bb.clear_path();
        }
    }

    Action::Status MoveToAction::tick(ActExecutorCtx& ctx, Blackboard& bb) {
        // 1. 检查目标是否还存在/有效
        if (!bb.target_valid) return Status::Failure;

        // 2. 检查是否到达
        auto cur = ctx.ch.getLoc();
        if (cur == bb.target) return Status::Success;

        // 3. 规划路径
        bool needPlan = bb.path.empty() || bb.path_invalid || bb.path.back() != bb.target;

        if (needPlan) {
            bb.path.clear();
            // 调用寻路算法
            if (!ctx.pf.plan_path(cur.first, cur.second, bb.target.first, bb.target.second, bb.path)) {
                return Status::Failure; // 无法抵达
            }
            bb.init_path_after_planned(); // 重置 path_i
        }

        // 4. 走一步 (Follow one step)
        if (bb.path_i >= bb.path.size()) return Status::Success; // 双重检查

        auto [nx, ny] = bb.path[bb.path_i];
        if (ctx.room.isPassable(nx, ny) && ctx.ch.tryStepTo(nx, ny)) {
            bb.path_i++;
            // 再次检查是否到达（避免多等一帧）
            if (ctx.ch.getLoc() == bb.target) return Status::Success;
            return Status::Running;
        } else {
            // 被堵住了
            bb.path_invalid = true; // 下一帧重算
            return Status::Running; // 或者返回 Failure 放弃
        }
    }
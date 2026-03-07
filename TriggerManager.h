//
// Created by jacob on 26-3-4.
//

#ifndef TRIGGERMANAGER_H
#define TRIGGERMANAGER_H

#include <string>
#include <unordered_map>

// 触发器类型枚举 (目前只有你说的 NotifyBind)
enum class TriggerType {
  NotifyBind
};

// 触发器数据定义
struct TriggerDef {
  std::string id;
  int level; // 所在房间ID
  int x;
  int y;
  TriggerType type;
  std::string target_id;   // 绑定的Static AI或Item的id
};

class TriggerManager {
public:
  static TriggerManager& inst() {
    static TriggerManager instance;
    return instance;
  }

  void clear() { triggers_.clear(); }

  void addTrigger(const TriggerDef& def) {
    triggers_[def.id] = def;
  }

  // 获取指定坐标位置的触发器
  const TriggerDef* getTriggerAt(int level, int x, int y) const {
    for (const auto& [id, trg] : triggers_) {
      if (trg.level == level && trg.x == x && trg.y == y) {
        return &trg;
      }
    }
    return nullptr; // 没踩到返回空
  }

  const std::unordered_map<std::string, TriggerDef>& getAllTriggers() const {
    return triggers_;
  }

private:
  std::unordered_map<std::string, TriggerDef> triggers_;
};

#endif //TRIGGERMANAGER_H

//
// Created by jacob on 26-2-22.
//

#ifndef SMARTITEM_H
#define SMARTITEM_H
#include "Item.h"
#include <string>
#include <vector>

// 定义效果类型
enum class EffectType {
  ModifyStat,
  SetState,
  AddMemory
};

// 单个效果的结构
struct ItemEffect {
  EffectType type;
  std::string target;// 比如hunger
  double value;      // 比如 -80.0
};

struct ActionDescriptor {
  std::string name;    // 动作名
  int intParam = 0;    // 整型参数
  std::string strParam;//字符串参数，如特定的物品ID
};

class SmartItem : public Item {
 public:
  SmartItem(ItemId id, ItemProps props) : Item(std::move(id), props) {
  }

  // 添加效果
  void addEffect(const ItemEffect &effect) {
    effects_.push_back(effect);
  }

  // 物品关联的动作序列
  [[nodiscard]] std::vector<ItemEffect> getEffects() const { return effects_; }
  void setSequence(const std::vector<ActionDescriptor> &seq) { sequence_ = seq; }
  [[nodiscard]] const std::vector<ActionDescriptor> &getSequence() const { return sequence_; }

 private:
  std::vector<ItemEffect> effects_;
  std::vector<ActionDescriptor> sequence_;
};

#endif//SMARTITEM_H

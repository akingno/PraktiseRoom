//
// Created by jacob on 26-3-1.
//

#ifndef SIGNAL_H
#define SIGNAL_H

#include <functional>
#include <map>

template <typename... Args>
class Signal {
public:
  using SlotType = std::function<void(Args...)>;

  // 注册监听
  int connect(SlotType slot) {
    int id = ++current_id_;
    slots_.insert({id, slot});
    return id;
  }

  // 取消监听
  void disconnect(int id) {
    slots_.erase(id);
  }

  // 发射信号
  void emit(Args... args) {
    for (auto const& pair : slots_) {
      pair.second(args...);
    }
  }

private:
  std::map<int, SlotType> slots_; // 存储所有接收器
  int current_id_ = 0; // 生成唯一的监听器id
};

#endif //SIGNAL_H

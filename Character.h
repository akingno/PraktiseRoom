//
// Created by jacob on 2025/9/11.
//

#ifndef ROOM_TEMP__CHARACTER_H_
#define ROOM_TEMP__CHARACTER_H_

#include "Config.h"
#include "ShortMemory.h"
#include "unused/Directions.h"
#include <algorithm>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>

class Character {

 public:
  [[nodiscard]] double getStat(const std::string &name) const {
    auto it = _stats.find(name);
    if (it != _stats.end()) {
      return it->second;
    }
    return 0.0;
  }

  void setStat(const std::string &name, double value) {
    _stats[name] = std::clamp(value, 0.0, 100.0);
  }

  void modifyStat(const std::string &name, double delta) {
    setStat(name, getStat(name) + delta);
  }
  [[nodiscard]] const std::unordered_map<std::string, double> &getAllStats() const {
    return _stats;
  }

  enum class Act {
    Wander,
    Stop,
    Talk,
    WaitAlways,
    UseItem
  };

  //构造函数
  Character() {
    setLoc(1, 1);
    for (const auto &rule : Cfg::need_rules) {
      _stats[rule.name] = 0.0;
      _base_rates[rule.name] = rule.growth_rate;
    }
  }

  void setLoc(int x, int y) {
    _loc = {x, y};
  }

  // 小人需求随时间增长：目前：饥饿和疲劳
  void tickNeeds(double dt_sec) {
    std::unordered_map<std::string, double> current_rates = _base_rates;

    if (_sleeping) {
      current_rates["fatigue"] = -Cfg::speed::sleep_recover;// 睡觉时疲劳下降
    }

    // 3. 统一遍历应用
    for (const auto &[stat_name, rate] : current_rates) {
      modifyStat(stat_name, rate * dt_sec);
    }
  }

  /*
   *关于饥饿和进食的计算
   */

  //（站在食物上）吃饭，饥饿-进食卡路里数
  void eat(int calories) {
    modifyStat("hunger", -calories);
  }

  void play(int board_decrease) {
    modifyStat("boredom", -board_decrease);
  }

  /*
   * 关于疲劳fatigue和睡眠
   */
  bool isSleeping() const { return _sleeping; }

  void setSleeping(bool s) { _sleeping = s; }

  /*
   * 关于玩电脑和短期记忆
   */

  ShortMemory &short_memory() { return short_memory_; }
  //当前无聊程度

  [[nodiscard]] const ShortMemory &get_short_memory() const { return short_memory_; }

  /*
   * 抽象的动作设置和辅助函数
   */
  // 设置当前动作
  void setAct(Act a) { act_ = a; }

  Act act() const { return act_; }

  static const char *Act2Str(const Act a) {
    switch (a) {
      case Act::Wander:
        return "Wander";
      case Act::Stop:
        return "Stop";
      case Act::Talk:
        return "Talk";
      case Act::WaitAlways:
        return "WaitAlways";
      default:
        return "Unknown";
    }
  }

  [[nodiscard]] std::pair<int, int> getLoc() const { return _loc; }

  // 尝试走一步
  bool tryStepTo(int nx, int ny) {
    auto [x, y] = _loc;
    if (std::abs(nx - x) + std::abs(ny - y) != 1) return false;// 仅允许 4 邻接
    _loc = {nx, ny};
    return true;
  }

  /*
 * 成员变量
 */
 private:
  std::pair<int, int> _loc;
  Act act_ = Act::Wander;

  // 疲劳/睡眠相关成员变量
  bool _sleeping = false;// 是否正在睡

  ShortMemory short_memory_;
  std::unordered_map<std::string, double> _stats;
  std::unordered_map<std::string, double> _base_rates;
};

#endif//ROOM_TEMP__CHARACTER_H_

//
// Created by jacob on 2025/9/11.
//

#ifndef ROOM_TEMP__CHARACTER_H_
#define ROOM_TEMP__CHARACTER_H_

#include "Config.h"
#include "ShortMemory.h"
#include "unused/Directions.h"
#include <random>
#include <utility>

class Character {

public:
  enum class Act {
    Eat,
    Sleep,
    Wander,
    Stop,
    UseComputer,
    Talk,
    WaitAlways
  };

 //构造函数
  Character() {
    setLoc(1,1);
  }

  void setLoc(int x, int y) {
    _loc = {x, y};
  }

  // 小人需求随时间增长：目前：饥饿和疲劳
  void tickNeeds(double dt_sec) {
    // 饥饿程度被tick，增加一些
    _hunger = std::min(100.0, _hunger + _hunger_rate * dt_sec);
    if (_eat_cooldown > 0.0) _eat_cooldown = std::max(0.0, _eat_cooldown - dt_sec);

    // 疲劳： 如果不在睡眠则上升，在睡眠则下降
    if (_sleeping) {
      _fatigue = std::max(0.0, _fatigue - _sleep_recover_rate * dt_sec);
    } else {
      _fatigue = std::min(100.0, _fatigue + _fatigue_rate * dt_sec);
    }

    //娱乐
    if (act_ == Act::UseComputer) {
      _boredom = std::max(0.0, _boredom - _computer_recover_rate * dt_sec);
    } else {
      _boredom = std::min(100.0, _boredom + _boredom_rate * dt_sec);
    }
  }

  /*
   *关于饥饿和进食的计算
   */
  // 当前饥饿程度（只和时间有关）
  [[nodiscard]] double  get_hunger_inner() const { return _hunger; }

  // 是否进食到冷却了？
  bool eatAvailable() const {
    return _eat_cooldown <= 0.0;
  }

  //（站在食物上）吃饭，饥饿-进食卡路里数
  void eat(int calories) {
    _hunger = std::max(0.0, _hunger - calories);
    _eat_cooldown = _eat_cooldown_secs;
  }

  /*
   * 关于疲劳fatigue和睡眠
   */
  [[nodiscard]] double get_fatigue_score() const { return _fatigue; }

  bool isSleeping() const { return _sleeping; }

  void setSleeping(bool s){ _sleeping = s; }


  /*
   * 关于玩电脑和短期记忆
   */

  [[nodiscard]] double get_boredom() const { return _boredom; }
  ShortMemory& short_memory() { return short_memory_; }
  //当前无聊程度

  [[nodiscard]] const ShortMemory& get_short_memory() const { return short_memory_; }

  /*
   * 抽象的动作设置和辅助函数
   */
  // 设置当前动作
  void  setAct(Act a) { act_ = a; }

  Act   act() const { return act_; }

  static const char* Act2Str(const Act a) {
    switch (a) {
      case Act::Eat:
        return "Eat";
      case Act::Wander:
        return "Wander";
      case Act::Sleep:
        return "Sleep";
      case Act::Stop:
        return "Stop";
      case Act::UseComputer:
        return "UseComputer";
      case Act::Talk:
        return "Talk";
      case Act::WaitAlways:
        return "WaitAlways";
      default:
        return "Unknown";
    }
  }

  [[nodiscard]] std::pair<int, int> getLoc() const{return _loc; }


  // 尝试走一步
  bool tryStepTo(int nx, int ny) {
    auto [x, y] = _loc;
    if (std::abs(nx - x) + std::abs(ny - y) != 1) return false; // 仅允许 4 邻接
    _loc = {nx, ny};
    return true;
  }

/*
 * 成员变量
 */
private:
  std::pair<int, int> _loc;
  Act act_ = Act::Wander;

  // 饥饿/进食相关成员变量
  double _hunger = 0.0;          // 0=饱 100=极饿
  double _hunger_rate = HUNGER_SPEED;
  double _eat_cooldown = 0.0;   // 当前冷却剩余秒
  double _eat_cooldown_secs = 1.0;

  // 疲劳/睡眠相关成员变量
  double _fatigue = 0.0;           // 0=精力充沛，100=极困
  double _fatigue_rate = FATIGUE_SPEED;       // 清醒时每秒 +1.5
  double _sleep_recover_rate = SLEEP_RECOVER_RATE; // 睡眠时每秒 -8.0
  bool   _sleeping = false;         // 是否正在睡

  // 娱乐/玩电脑
  double _boredom = 0.0;
  double _boredom_rate = BOREDOM_SPEED;
  double _computer_recover_rate = COMPUTER_RECOVER_RATE;

  ShortMemory short_memory_;
};

#endif//ROOM_TEMP__CHARACTER_H_

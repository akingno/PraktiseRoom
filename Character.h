//
// Created by jacob on 2025/9/11.
//

#ifndef ROOM_TEMP__CHARACTER_H_
#define ROOM_TEMP__CHARACTER_H_



#include <random>
#include <utility>
#include "Directions.h"
#include "Config.h"


class Character {

public:
  enum class Act {
    Eat,
    Sleep,
    Wander,
    Stop
  };

 //构造函数
  Character() : _loc(1,1) {

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
  }

  /*
   *
   *关于饥饿和进食的计算
   *
   */
  // 当前饥饿程度（只和时间有关）
  [[nodiscard]] double  get_hunger_inner() const { return _hunger; }



  // 是否进食到冷却了？
  bool eatAvailable() const {
    return _eat_cooldown <= 0.0;
  }

  //（站在食物上）吃饭，饥饿-进食卡路里数
  void eat(int calories) {                        // 简化：卡路里直接换成饱腹度
    _hunger = std::max(0.0, _hunger - calories);
    _eat_cooldown = _eat_cooldown_secs;
  }

  /*
   *
   * 关于疲劳fatigue和睡眠
   *
   *
   */
  [[nodiscard]] double get_fatigue_score() const { return _fatigue; }
  bool isSleeping() const { return _sleeping; }
  void setSleeping(bool s){ _sleeping = s; }


  /*
   *
   * 抽象的动作设置和辅助函数
   *
   *
   */
  // 设置当前动作
  void  setAct(Act a) {
    act_ = a;
  }

  Act   act() const {
    return act_;
  }

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
      default:
        return "Unknown";
    }
  }

  [[nodiscard]] std::pair<int, int> getLoc() const{return _loc; }

  /*
   *
   * 游荡
   * Wander的相关函数
   *
   *
   */
  // 尝试走一步
  bool tryStepTo(int nx, int ny) {
    auto [x, y] = _loc;
    if (std::abs(nx - x) + std::abs(ny - y) != 1) return false; // 仅允许 4 邻接
    _loc = {nx, ny};
    return true;
  }

/*
 *
 * 成员变量
 *
 *
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


};

#endif//ROOM_TEMP__CHARACTER_H_

//
// Created by jacob on 25-10-19.
//

#ifndef CONFIG_H
#define CONFIG_H

// tile 像素大小
constexpr int TILE_PX = 32;

constexpr double TICK_MILLI = 50.0;
inline int TICK_MILLI_INT = 50;
constexpr int TICKS_PER_SEC = 20;

// 方向（已弃用）， 启用后请把此删除
constexpr int MAX_DIR = 5;


// 房间相关
constexpr int VIEW_W = 40;
constexpr int VIEW_H = 30;
constexpr int DOOR_X = 20;
constexpr int DOOR_Y = 0;
constexpr int FOOD_X = 20;
constexpr int FOOD_Y = 1;
constexpr int BED_X  = 10;
constexpr int BED_Y  = 28;

// 角色状态速度
constexpr double HUNGER_SPEED = 2.0;
constexpr double FATIGUE_SPEED = 0.5;
constexpr int SLEEP_RECOVER_RATE = 6;

// 阈值
constexpr double HUNGER_ENTER = 60.0;
constexpr double TIRED_ENTER = 50.0;
constexpr double RESTED_EXIT = 30.0;

constexpr double BASE_WANDER = 0.05;
constexpr double BASE_STOP = 0.06;

// Stop的时间
constexpr int MIN_STOP_TIME = 2;
constexpr int MAX_STOP_TIME = 6; // seconds

// wander完进入stop的概率
constexpr double ENTER_STOP_POSSI = 0.9;

// 食物
constexpr int FOOD_CALORIES = 80;



#endif //CONFIG_H

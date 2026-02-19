//
// Created by jacob on 25-10-19.
//


// Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// 辅助结构体保持不变
struct Pos { int x, y; };

namespace Cfg {
void Load(const std::string& filename = "config.json");
void Save(const std::string& filename = "config.json");

namespace core {
inline int tile_px = 32;
inline double tick_milli = 50.0;
inline int tick_milli_int = 50;
inline int ticks_per_sec = 20;
}

namespace room {
inline int view_w = 40;
inline int view_h = 30;
inline int door_x = 20;
inline int door_y = 0;
inline int food_x = 20;
inline int food_y = 1;
inline int bed_x  = 10;
inline int bed_y  = 28;
inline int computer_x = 10;
inline int computer_y = 1;
}

namespace speed {
inline double hunger = 2.0;
inline double fatigue = 0.5;
inline double boredom = 0.8;
inline double sleep_recover = 6.0;
inline double computer_recover = 10.0;
}

namespace threshold {
inline double hunger_enter = 60.0;
inline double tired_enter = 60.0;
inline double rested_exit = 25.0;
inline double bored_enter = 40.0;
inline double bored_exit = 5.0;
}

namespace score {
inline double base_wander = 0.05;
inline double base_stop = 0.06;
inline double base_use_computer = 0.055;
inline double base_talk = 99.0;
}

namespace time {
inline int min_stop = 2;
inline int max_stop = 6;
inline int min_use_computer = 4;
inline int max_use_computer = 8;
}

namespace prob {
inline double change_action = 0.9;
inline double change_talk = 0.4;
}

namespace item {
inline int food_calories = 80;
inline int play_computer_entertain = 70;
}
}

#endif //CONFIG_H
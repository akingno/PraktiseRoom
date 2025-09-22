#include "Block.h"
#include "Character.h"
#include "Room.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <windows.h>

void enableAnsi() {
  //用于windwos终端使用
  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE) return;
  DWORD mode = 0;
  if (!GetConsoleMode(hOut, &mode)) return;
  mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; // 关键
  SetConsoleMode(hOut, mode);
}


/**
 *
 * A Rectangle with 4 coordinate:
 * [x0,x1), [y0,y1)
 *
 * */
struct Rect4 {
  int x0, y0, x1, y1;
};


/**
 *
 * Output information at the last line
 *
 * */
inline void print_status_line(int row, const std::string& s) {
  std::cout << "\x1b[" << row << ";1H"   // 光标定位到 row 行 1 列
            << "\x1b[2K"                 // 清除整行
            << s                         // 输出内容
            << std::flush;
}

void render_frame(const Character&character, Room& room) {

  // 2) 准备帧缓冲（W 行，每行 W 字符 + '\n'）
  std::string frame;
  frame.resize(VIEW_H * (VIEW_W + 1));

  auto buf_at = [&](int sx, int sy) -> char& {
    //这个消除每行结尾的\n的影响
    return frame[ sy * (VIEW_W + 1) + sx ];
  };

  for(int y = 0; y < VIEW_H; ++y){
    for(int x = 0; x < VIEW_W; ++x){
      buf_at(x, y) = Block::tile_glyph(room.getBlocksType(x, y));
    }
  }

  //每行最后一个是换行(其实在屏幕宽度外)
  for (int y = 0; y < VIEW_H; ++y) {
    frame[y*(VIEW_W+1) + VIEW_W] = '\n';
  }

  // 4) 覆盖玩家 '@'
  int sx = static_cast<int>(std::floor(character.getLoc().first));
  int sy = static_cast<int>(std::floor(character.getLoc().second));
  if (0 <= sx && sx < VIEW_W && 0 <= sy && sy < VIEW_H) {
    buf_at(sx, sy) = '@';
  }

  // 或首次渲染前清屏一次
  static bool first = true;
  if (first) { std::cout << "\x1b[2J\x1b[H"; first = false; }

  std::cout << "\x1b[H" << frame;
  print_status_line(VIEW_H + 1,
                    "Character location: (" + std::to_string(character.getLoc().first) + ", " +
                        std::to_string(character.getLoc().second) + ")");

  print_status_line(VIEW_H + 2,
                    "Character Current Direction: " + Character::Dir2Str(character.getLastDir()));

}


int main() {

  enableAnsi();
  uint64_t seed = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  Random::init(seed);

  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);

  Room room;

  bool running = true;
  Character character{};

  auto is_passable = [&](int wx,int wy){
    auto tile_type = room.getBlocksType(wx, wy);
    return tile_type != TileType::WallV && tile_type != TileType::WallH;
  };

  using clock = std::chrono::steady_clock;
  auto next_tick = clock::now();
  const auto dt = std::chrono::milliseconds(33);
  while(running){
    //移动角色
    if(character.tryMove(is_passable, Character::KEEP_LAST_DIR_PROB)){

    }

    //根据cachechunk找到其chunk, 再找到其block
    //渲染
    //渲染其他物品
    //渲染角色
    render_frame(character, room);
    next_tick += dt;
    std::this_thread::sleep_until(next_tick);

  }

}


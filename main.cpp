#include "Block.h"
#include "Character.h"
#include "Room.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <windows.h>
#include "Utils.h"

const int FOOD_CALORIES = 25;
const double BASE_WANDER = 0.1;

struct ActExecutorCtx {
  Room& room;
  Character& ch;
};

/**
 *
 * Node of BT
 * @param ctx
 * @return
 */
bool step_eat(ActExecutorCtx& ctx) {
  const auto pos  = ctx.ch.getLoc();
  const auto fpos = std::pair<int,int>{ctx.room.foodPos().x, ctx.room.foodPos().y};
  // 当站在食物上就吃
  if (ctx.room.hasFood() && pos == fpos && ctx.ch.eatAvailable()) {
    ctx.ch.eat(FOOD_CALORIES);
    ctx.room.consumeFood();
    return true;
  }
  return false;
}

bool step_wander(ActExecutorCtx& ctx) {
  auto is_passable = [&](int x,int y){
    auto t = ctx.room.getBlocksType(x,y);
    return t != TileType::WallV && t != TileType::WallH;
  };
  ctx.ch.tryMove(is_passable, Character::KEEP_LAST_DIR_PROB);
  return true;
}


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


struct ActionCandidate {
  Character::Act kind;
  double score = 0.0;
  bool on_cooldown = false;

  // todo:指向行为树的指针/函数对象
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

  // 准备帧缓冲（W 行，每行 W 字符 + '\n'）
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

  // 每行最后一个是换行(其实在屏幕宽度外)
  for (int y = 0; y < VIEW_H; ++y) {
    frame[y*(VIEW_W+1) + VIEW_W] = '\n';
  }
  // 覆盖 门 和 食物
  const auto& d = room.doorPos();
  if (0<=d.x && d.x<VIEW_W && 0<=d.y && d.y<VIEW_H) buf_at(d.x, d.y) =
    Block::tile_glyph(room.getBlocksType(d.x,d.y));

  if (room.hasFood()) {
    const auto& f = room.foodPos();
    if (0<=f.x && f.x<VIEW_W && 0<=f.y && f.y<VIEW_H) buf_at(f.x, f.y) =
      Block::tile_glyph(room.getBlocksType(f.x,f.y));
  }

  // 覆盖玩家 '@'
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

  print_status_line(VIEW_H + 3,
    std::string("Action: ") + Character::Act2Str(character.act())
    +" | Hunger=" + std::to_string(character.hunger()));

}


int main() {
  // 初始化
  enableAnsi();
  uint64_t seed = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  Random::init(seed);
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  //初始化结束

  bool running = true;
  Room room;
  Character character{};


  //计时器
  using clock = std::chrono::steady_clock;
  auto next_tick = clock::now();
  const auto dt = std::chrono::milliseconds(33);


  while(running){

    //tick一下需求的更新
    character.tickNeeds(33.0/1000.0);
    //固定刷新食物
    room.ensureFoodSpawned();
    // ===== 3) Perception & Utility =====
    bool hasFood = room.hasFood();
    auto pos  = character.getLoc();
    std::pair fpos = {room.foodPos().x, room.foodPos().y};

    const double scoreEat = CalcScoreEat(
        character.hunger(), pos, hasFood, fpos,
        !character.eatAvailable(),              // onCooldown: true=禁止
        character.act() == Character::Act::Eat  // sticky
    );
    const double scoreWander = BASE_WANDER;

    Character::Act chosen = (scoreEat > scoreWander) ? Character::Act::Eat
                                                     : Character::Act::Wander;
    character.setAct(chosen);

    ActExecutorCtx ex{room, character};
    if (character.act() == Character::Act::Eat) {
      step_eat(ex);            // 现在是“站上去就吃”，以后直接换成 BT 的 tick
    } else {
      step_wander(ex);
    }


    //根据cachechunk找到其chunk, 再找到其block
    //渲染
    //渲染其他物品
    //渲染角色
    render_frame(character, room);

    print_status_line(VIEW_H + 4,
    std::string("Hunger Score: ") + std::to_string(scoreEat));
    double desire   = desire_from_hunger(character.hunger(), 60);
    bool   avail    = room.hasFood();
    int    dist     = manhattan(pos, fpos);
    double feas     = 1.0 / (1.0 + 0.2 * dist);
    bool   cooldown = !character.eatAvailable();

    print_status_line(VIEW_H + 5,
      "DBG desire=" + std::to_string(desire)
      + " avail=" + std::to_string(avail)
      + " dist=" + std::to_string(dist)
      + " feas=" + std::to_string(feas)
      + " cooldown=" + std::to_string(cooldown));

    next_tick += dt;
    std::this_thread::sleep_until(next_tick);

  }

}


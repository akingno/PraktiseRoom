#include "ASCIIRender.h"
#include "AStarGrid.h"
#include "ActionExecutor.h"
#include "Block.h"
#include "Character.h"
#include "IRender.h"
#include "Room.h"
#include "SDL3Render.h"
#include "Utils.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

const double BASE_WANDER = 0.05;



/**
 *
 * A Rectangle with 4 coordinate:
 * [x0,x1), [y0,y1)
 *
 * */
struct Rect4 {
  int x0, y0, x1, y1;
};



int main() {
  // 初始化
  //ASCIIRender::initialTerminal();
  uint64_t seed = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  Random::init(seed);
  //HWND console = GetConsoleWindow();
  //ShowWindow(console, SW_MAXIMIZE);
  //初始化结束


  bool running = true;
  Room room;
  Character character{};


  auto passable = [&](int x,int y){
    auto t = room.getBlocksType(x,y);
    return t != TileType::WallV && t != TileType::WallH;
  };
  AStarGrid path_finder({VIEW_W, VIEW_H}, passable);

  ActionExecutor executor(path_finder);
  Blackboard bb;

  //std::unique_ptr<IRender> render = std::make_unique<ASCIIRender>(VIEW_H * (VIEW_W + 0));
  //SDL3渲染器
  const int TILE_PX = 32;
  std::unique_ptr<IRender> render = std::make_unique<SDL3Render>(VIEW_W, VIEW_H, TILE_PX, "Little Room (SDL3)");

  //计时器
  using clock = std::chrono::steady_clock;
  auto next_tick = clock::now();
  const auto dt = std::chrono::milliseconds(33);
  uint64_t tick_index = 0;

  while(running){

    if (render->poll_quit()) break;
    //tick一下需求的更新
    character.tickNeeds(33.0/1000.0);
    //固定刷新食物
    room.ensureFoodSpawned();
    // ===== 3) Perception & Utility =====
    bool hasFood = room.hasFood();


    const double scoreEat = CalcScoreEat(
        character.get_hunger_inner(), hasFood,
        !character.eatAvailable(),              // onCooldown: true=禁止
        character.act() == Character::Act::Eat  // sticky
    );
    const double scoreWander = BASE_WANDER;
    const double scoreSleep = CalcScoreSleep(
    character.get_fatigue_score(),  room.hasBed(),
    character.act() == Character::Act::Sleep
    );


    Character::Act chosen_action = Character::Act::Wander;
    double best = scoreWander;
    if (scoreEat   > best) { best = scoreEat;   chosen_action = Character::Act::Eat; }
    if (scoreSleep > best) { best = scoreSleep; chosen_action = Character::Act::Sleep; }

    character.setAct(chosen_action);

    RenderStats stats{scoreEat, scoreWander, scoreSleep};

    ActExecutorCtx ctx{room, character, tick_index};
    executor.tick(chosen_action, ctx, bb);

    //根据cachechunk找到其chunk, 再找到其block
    //渲染
    //渲染其他物品
    //渲染角色
    render->render_frame(character, room, stats);


    next_tick += dt;
    std::this_thread::sleep_until(next_tick);
    ++tick_index;


  // End of Loop
  }

}


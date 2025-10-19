#include "ASCIIRender.h"
#include "AStarPathfinder.h"
#include "ActionExecutor.h"
#include "Character.h"
#include "IRender.h"
#include "Room.h"
#include "SDL3Render.h"
#include "Utils.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>


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
  uint64_t seed = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  Random::init(seed);
  //初始化结束

  bool running = true;
  Room room;
  Character character{};

  AStarPathfinder path_finder(
    {VIEW_W, VIEW_H},
    [&](int x, int y) { return room.isPassable(x, y); }
);

  ActionExecutor executor(path_finder);

  Blackboard bb;

  //SDL3渲染器
  std::unique_ptr<IRender> render = std::make_unique<SDL3Render>(VIEW_W, VIEW_H, TILE_PX, "Little Room (SDL3)");

  //计时器
  using clock = std::chrono::steady_clock;
  auto next_tick = clock::now();
  const auto dt = std::chrono::milliseconds(TICK_MILLI_INT);
  uint64_t tick_index = 0;

  const double scoreWander = BASE_WANDER;


  while(running){

    if (render->poll_quit()) break;
    //tick一下需求的更新
    character.tickNeeds(TICK_MILLI/1000.0);
    //固定刷新食物
    room.ensureFoodSpawned();
    // ===== 3) Perception & Utility =====
    bool hasFood = room.hasFood();

    double scoreStop = 0.0; //用于确认目前是否在Stop
    if (bb.in_stop(tick_index)) {
      scoreStop = BASE_STOP; // > BASE_WANDER(0.05)，保证到点之前维持 Stop
    }

    const double scoreEat = CalcScoreEat(
        character.get_hunger_inner(), hasFood,
        !character.eatAvailable(),              // onCooldown: true=禁止
        character.act() == Character::Act::Eat,  // sticky
        HUNGER_ENTER
    );

    const double scoreSleep = CalcScoreSleep(
    character.get_fatigue_score(),  room.hasBed(),
    character.act() == Character::Act::Sleep,
    TIRED_ENTER, RESTED_EXIT
    );


    Character::Act chosen_action = Character::Act::Wander;
    double best = scoreWander;
    if (scoreEat   > best) { best = scoreEat;   chosen_action = Character::Act::Eat; }
    if (scoreSleep > best) { best = scoreSleep; chosen_action = Character::Act::Sleep; }
    if (scoreStop  > best) { best = scoreStop;  chosen_action = Character::Act::Stop; }

    character.setAct(chosen_action);

    RenderStats stats{scoreEat, scoreWander, scoreSleep};

    ActExecutorCtx ctx{room, character, tick_index};
    executor.tick(chosen_action, ctx, bb);

    //渲染
    render->render_frame(character, room, stats);


    next_tick += dt;
    std::this_thread::sleep_until(next_tick);
    ++tick_index;


  // End of Loop
  }

}


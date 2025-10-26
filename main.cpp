#include "ActionExecutor.h"
#include "Character.h"
#include "GameContentInit.h"
#include "Room.h"
#include "renders/ASCIIRender.h"
#include "renders/IRender.h"
#include "renders/SDL3Render.h"
#include "tools/AStarPathfinder.h"
#include "tools/Utils.h"
#include <chrono>
#include <iostream>
#include <thread>

int main() {
  // 初始化
  uint64_t seed = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  Random::init(seed);
  //初始化结束

  bool running = true;
  Room room;
  ItemLayer items;
  Character character;
  register_default_items();
  
  items.ensureBedPlaced();
  items.ensureFoodSpawned();
  items.ensureComputerPlaced();

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

    // 每循环固定刷新一下食物
    items.ensureFoodSpawned();
    // ===== 3) Perception & Utility =====

    double scoreStop = 0.0; //用于确认目前是否在Stop
    if (bb.in_stop(tick_index)) {
      scoreStop = BASE_STOP; // > BASE_WANDER(0.05)，保证到点之前维持 Stop
    }

    // 分数计算
    const double scoreEat = CalcScoreEat(
        character.get_hunger_inner(), items.hasFood(),
        !character.eatAvailable(),              // onCooldown: true=禁止
        character.act() == Character::Act::Eat,  // sticky
        HUNGER_ENTER
    );

    const double scoreSleep = CalcScoreSleep(
    character.get_fatigue_score(),  items.hasBed(),
    character.act() == Character::Act::Sleep,
    TIRED_ENTER, RESTED_EXIT
    );


    Character::Act chosen_action = Character::Act::Wander;
    double best = scoreWander;
    if (scoreEat   > best) { best = scoreEat;   chosen_action = Character::Act::Eat; }
    if (scoreSleep > best) { best = scoreSleep; chosen_action = Character::Act::Sleep; }
    if (scoreStop  > best) { best = scoreStop;  chosen_action = Character::Act::Stop; }

    character.setAct(chosen_action);

    // 渲染数据
    RenderStats stats{scoreEat, scoreWander, scoreSleep};

    ActExecutorCtx ctx{room, character, tick_index, path_finder, items};
    executor.tick(chosen_action, ctx, bb);

    //渲染
    render->render_frame(items, character, room, stats);

    std::cout<< std::string("Action: ") << Character::Act2Str(character.act())<<"\n"<<
    "Inner Hunger=" + std::to_string(character.get_hunger_inner())<<"\n"<<
    " Inner Fatigue=" + std::to_string(character.get_fatigue_score())<<"\n"<<
    " Sleeping Status=" + std::to_string(character.isSleeping())<<"\n\n";


    next_tick += dt;
    std::this_thread::sleep_until(next_tick);
    ++tick_index;


  // End of Loop
  }

}


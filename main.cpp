#include "ASCIIRender.h"
#include "AStarGrid.h"
#include "ActionExecutor.h"
#include "Block.h"
#include "Character.h"
#include "IRender.h"
#include "Room.h"
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
  ASCIIRender::initialTerminal();
  uint64_t seed = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  Random::init(seed);
  //初始化结束


  bool running = true;
  Room room;
  Character character{};
  IRender render;

  auto passable = [&](int x,int y){
    auto t = room.getBlocksType(x,y);
    return t != TileType::WallV && t != TileType::WallH;
  };
  AStarGrid pf({VIEW_W, VIEW_H}, passable);

  ActionExecutor executor(pf);
  Blackboard bb;

  //计时器
  using clock = std::chrono::steady_clock;
  auto next_tick = clock::now();
  const auto dt = std::chrono::milliseconds(33);
  int tick_index = 0;

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
        character.get_hunger_score(), pos, hasFood, fpos,
        !character.eatAvailable(),              // onCooldown: true=禁止
        character.act() == Character::Act::Eat  // sticky
    );
    const double scoreWander = BASE_WANDER;


    Character::Act chosen = (scoreEat > scoreWander) ? Character::Act::Eat
                                                     : Character::Act::Wander;
    character.setAct(chosen);

    ActExecutorCtx ctx{room, character, tick_index};
    executor.tick(chosen, ctx, bb);

    //根据cachechunk找到其chunk, 再找到其block
    //渲染
    //渲染其他物品
    //渲染角色
    render.render_frame_ascii(character, room, scoreEat, scoreWander);


    next_tick += dt;
    std::this_thread::sleep_until(next_tick);
    ++tick_index;


  // End of Loop
  }

}


#include "ASCIIRender.h"
#include "Block.h"
#include "Character.h"
#include "IRender.h"
#include "Room.h"
#include "Utils.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>


const int FOOD_CALORIES = 25;
const double BASE_WANDER = 0.05;

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






int main() {
  // 初始化
  ASCIIRender::initialTerminal();
  uint64_t seed = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
  Random::init(seed);
  IRender render;

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
        character.get_hunger_score(), pos, hasFood, fpos,
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
    render.render_frame_ascii(character, room, scoreEat, scoreWander);


    next_tick += dt;
    std::this_thread::sleep_until(next_tick);


  // End of Loop
  }

}


//
// Created by jacob on 25-9-24.
//

#include "ASCIIRender.h"

#include "Utils.h"
void ASCIIRender::render_room(const Room &room) {

  for(int y = 0; y < VIEW_H; ++y){
    for(int x = 0; x < VIEW_W; ++x){
      buf_at(x, y) = Block::tile_glyph(room.getBlocksType(x, y));
    }
  }

  // 每行最后一个是换行(其实在屏幕宽度外)
  for (int y = 0; y < VIEW_H; ++y) {
    frame[y*(VIEW_W+1) + VIEW_W] = '\n';
  }}
void ASCIIRender::render_items(const Room &room) {
  // 覆盖 门 和 食物
  const auto& d = room.doorPos();
  if (0<=d.x && d.x<VIEW_W && 0<=d.y && d.y<VIEW_H) {
    buf_at(d.x, d.y) = Block::tile_glyph(room.getBlocksType(d.x,d.y));
  }

  if (room.hasFood()) {
    const auto& f = room.foodPos();
    if (0<=f.x && f.x<VIEW_W && 0<=f.y && f.y<VIEW_H) {
      buf_at(f.x, f.y) = Block::tile_glyph(room.getBlocksType(f.x,f.y));
    }
  }
}
void ASCIIRender::render_character(const Character &character) {
  // 覆盖玩家 '@'
  int sx = static_cast<int>(std::floor(character.getLoc().first));
  int sy = static_cast<int>(std::floor(character.getLoc().second));
  if (0 <= sx && sx < VIEW_W && 0 <= sy && sy < VIEW_H) {
    buf_at(sx, sy) = '@';
  }
}
void ASCIIRender::render_info(const Room & room, const Character & character, double scoreEat, double scoreWander) {

  // 或首次渲染前清屏一次

  if (first) { std::cout << "\x1b[2J\x1b[H"; first = false; }

  std::cout << "\x1b[H" << frame;
  print_status_line(VIEW_H + 1,
                    "Character location: (" + std::to_string(character.getLoc().first) + ", " +
                        std::to_string(character.getLoc().second) + ")");

  print_status_line(VIEW_H + 2,
                    "Character Current Direction: " + Dir2Str(character.getLastDir()));

  double desire   = desire_from_hunger(character.get_hunger_score(), 60);

  print_status_line(VIEW_H + 3,
    std::string("Action: ") + Character::Act2Str(character.act())
    +" | Hunger=" + std::to_string(desire));

  print_status_line(VIEW_H + 4,
    std::string("Hunger Score: ") + std::to_string(scoreEat));

  print_status_line(VIEW_H + 5,
      std::string("Wander Score: ") + std::to_string(scoreWander));
  auto pos  = character.getLoc();
  std::pair fpos = {room.foodPos().x, room.foodPos().y};


  bool   avail    = room.hasFood();
  int    dist     = manhattan(pos, fpos);
  double feas     = 1.0 / (1.0 + 0.2 * dist);
  bool   cooldown = !character.eatAvailable();

  print_status_line(VIEW_H + 6,
    "DBG desire=" + std::to_string(desire)
    + " avail=" + std::to_string(avail)
    + " dist=" + std::to_string(dist)
    + " feas=" + std::to_string(feas)
    + " cooldown=" + std::to_string(cooldown));
}
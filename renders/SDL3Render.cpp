//
// Created by jacob on 25-10-18.
//
#include "SDL3Render.h"

#include <stdexcept>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "../ItemLayer.h"

static std::string RES(const char* name) {
  return std::string("res/") + name;
}

SDL3Render::SDL3Render(int viewW, int viewH, int tilePx, const std::string& title)
  : viewW_(viewW), viewH_(viewH), tilePx_(tilePx) {

  // SDL 基础
  if (!SDL_Init(SDL_INIT_VIDEO)) {  // 返回bool：true成功 / false失败
    throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());;
  }

  const int winW = viewW_ * tilePx_;
  const int winH = viewH_ * tilePx_;

  window_ = SDL_CreateWindow(title.c_str(), winW, winH, SDL_WINDOW_RESIZABLE);
  if (!window_) {
    throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
  }

  renderer_ = SDL_CreateRenderer(window_, nullptr);
  if (!renderer_) {
    throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
  }

  // 载入纹理
  tileTex_[TileType::Grass] = loadTexture(RES("grass.png"));
  tileTex_[TileType::WallH] = loadTexture(RES("wall_h.png"));
  tileTex_[TileType::WallV] = loadTexture(RES("wall_v.png"));
  tileTex_[TileType::FOOD]  = loadTexture(RES("food.png"));
  tileTex_[TileType::BED]   = loadTexture(RES("bed.png"));
  texDoor_                  = loadTexture(RES("door.png"));
  texCharacter_             = loadTexture(RES("character.png"));


}

SDL3Render::~SDL3Render() {
  for (auto& kv : tileTex_) if (kv.second) SDL_DestroyTexture(kv.second);
  if (texDoor_)       SDL_DestroyTexture(texDoor_);
  if (texCharacter_)  SDL_DestroyTexture(texCharacter_);
  if (font_)          TTF_CloseFont(font_);

  if (renderer_) SDL_DestroyRenderer(renderer_);
  if (window_)   SDL_DestroyWindow(window_);

  TTF_Quit();          // SDL3_ttf 仍需 Quit
  // IMG_Quit();       // SDL3_image 3.x 已无此函数，千万别调用
  SDL_Quit();
}

SDL_Texture* SDL3Render::loadTexture(const std::string& path) {
  SDL_Texture* tex = IMG_LoadTexture(renderer_, path.c_str());
  if (!tex) {
    // SDL3_image 统一用 SDL_GetError()
    throw std::runtime_error(std::string("IMG_LoadTexture failed for ")
                             + path + ": " + SDL_GetError());
  }
  return tex;
}

void SDL3Render::clear() {
  SDL_SetRenderDrawColor(renderer_, 16, 16, 18, 255);
  SDL_RenderClear(renderer_);
}

void SDL3Render::present() {
  SDL_RenderPresent(renderer_);
}

void SDL3Render::drawTile(int gx, int gy, SDL_Texture* tex) {
  if (!tex) return;
  SDL_FRect dst;
  dst.x = static_cast<float>(gx * tilePx_);
  dst.y = static_cast<float>(gy * tilePx_);
  dst.w = static_cast<float>(tilePx_);
  dst.h = static_cast<float>(tilePx_);
  SDL_RenderTexture(renderer_, tex, nullptr, &dst);
}


bool SDL3Render::poll_quit() {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    if (e.type == SDL_EVENT_QUIT) return true;

    if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) {
      return true;
    }
  }
  return false;
}

void SDL3Render::render_frame(const ItemLayer& items_, const Character& c, const Room& room, const RenderStats& stats) {
  clear();
  // 先铺一层地板
  SDL_Texture* tex = tileTex_[TileType::Grass];

  for (int y = 0; y < VIEW_H; ++y) {
    for (int x = 0; x < VIEW_W; ++x) {
      drawTile(x, y, tex);
    }
  }

  // 1) 画地面/墙/门
  for (int y=0; y<VIEW_H; ++y) {
    for (int x=0; x<VIEW_W; ++x) {
      TileType t = room.getBlocksType(x, y);
      auto tex = (t==TileType::DOOR ? texDoor_ : tileTex_[t]);
      if (tex) drawTile(x, y, tex);
    }
  }

  // 2) 画物品
  for (auto& [key, iid] : items_.items()) {
    int x = key % VIEW_W;
    int y = key / VIEW_W;
    SDL_Texture* tex = nullptr;
    if (iid == "food") tex = tileTex_[TileType::FOOD];
    else if (iid == "bed") tex = tileTex_[TileType::BED];
    // ... 未来更多
    if (tex) drawTile(x, y, tex);
  }

  // 2) 画角色（你这边是 pair<int,int> getLoc()）
  const int cx = c.getLoc().first;
  const int cy = c.getLoc().second;
  drawTile(cx, cy, texCharacter_);


  present();
}


//
// Created by jacob on 25-10-18.
//

#ifndef SDL3RENDER_H
#define SDL3RENDER_H



#include "IRender.h"
#include "Room.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <unordered_map>

class SDL3Render : public IRender {
public:
  SDL3Render(int viewW, int viewH, int tilePx, const std::string& title = "Little Room");
  ~SDL3Render() override;

  void render_frame(const Character& c, const Room& room, const RenderStats& stats) override;
  bool poll_quit() override;

private:
  SDL_Window*   window_   = nullptr;
  SDL_Renderer* renderer_ = nullptr;

  int viewW_;
  int viewH_;
  int tilePx_;

  // 纹理集合
  std::unordered_map<TileType, SDL_Texture*> tileTex_;
  SDL_Texture* texCharacter_ = nullptr;
  SDL_Texture* texDoor_      = nullptr; // 可与 tileTex_[TileType::DOOR] 合并

  // 字体（HUD）
  TTF_Font* font_ = nullptr;

private:
  SDL_Texture* loadTexture(const std::string& pngPath);
  void drawTile(int gridX, int gridY, SDL_Texture* tex);
  void clear();
  void present();

  // 禁拷贝
  SDL3Render(const SDL3Render&) = delete;
  SDL3Render& operator=(const SDL3Render&) = delete;
};;



#endif //SDL3RENDER_H

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
#include "spdlog/spdlog.h"
#include "../EditorUI.h"
#include "../TriggerManager.h"
#include "../WorldManager.h"

static std::string RES(const char* name) {
  return std::string("res/") + name;
}

SDL3Render::SDL3Render(int viewW, int viewH, int tilePx, const std::string& title)
  : viewW_(viewW), viewH_(viewH), tilePx_(tilePx) {

  // SDL 基础
  if (!SDL_Init(SDL_INIT_VIDEO)) {  // 返回bool：true成功 / false失败
    throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());;
  }
  const int gameW = viewW_ * tilePx_;
  const int gameH = viewH_ * tilePx_;

  const int winW = gameW + 350;
  const int winH = gameH + 250 + Cfg::room::menu_bar_h;

  window_ = SDL_CreateWindow(title.c_str(), winW, winH, SDL_WINDOW_RESIZABLE);
  if (!window_) {
    throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
  }

  renderer_ = SDL_CreateRenderer(window_, nullptr);
  if (!renderer_) {
    throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
  }

  texCharacter_ = loadTexture(RES("character.png"));
  texTrigger_ = loadTexture(RES("trigger.png"));

  //载入物品纹理
  for (const auto& [id, itemPtr] : ItemRegistry::inst().getAllItems()) {
    std::string texName = itemPtr->props().texture_name;
    if (!texName.empty()) {
      try {
        itemTextures_[id] = loadTexture(RES(texName.c_str()));
      } catch (const std::exception& e) {
        spdlog::error("SDL3Render: Missing texture for item: {}", id);
      }
    }
  }

  // 载入地形纹理
  for (const auto& [id, def] : TerrainRegistry::inst().getAllTerrains()) {
    if (!def.texture_name.empty()) {
      try {
        terrainTextures_[id] = loadTexture(RES(def.texture_name.c_str()));
      } catch (const std::exception& e) {
        spdlog::error("SDL3Render: Missing texture for terrain: {}", id);
      }
    }
  }

}

void SDL3Render::loadTerrainTexture(const std::string& tileId, const std::string& textureName) {
  if (textureName.empty()) return;
  try {
    if (terrainTextures_.find(tileId) != terrainTextures_.end()) {
      SDL_DestroyTexture(terrainTextures_[tileId]);
    }
    terrainTextures_[tileId] = loadTexture(RES(textureName.c_str()));
  } catch (const std::exception& e) {
    spdlog::error("SDL3Render: Failed to load terrain texture: {}", tileId);
  }
}

void SDL3Render::loadAgentTexture(const std::string& textureName) {
  if (textureName.empty()) return;
  try {
    if (agentTextures_.find(textureName) != agentTextures_.end()) return;
    agentTextures_[textureName] = loadTexture(RES(textureName.c_str()));
    spdlog::info("SDL3Render:  Loaded agent texture: {}", textureName);
  } catch (const std::exception& e) {
    spdlog::error("SDL3Render: Failed to load agent texture: {}", textureName);
  }
}

SDL3Render::~SDL3Render() {
  for (auto& kv : itemTextures_) if (kv.second) SDL_DestroyTexture(kv.second);

  if (texCharacter_)  SDL_DestroyTexture(texCharacter_);
  if (texTrigger_)    SDL_DestroyTexture(texTrigger_);
  if (font_)          TTF_CloseFont(font_);

  if (renderer_) SDL_DestroyRenderer(renderer_);
  if (window_)   SDL_DestroyWindow(window_);

  TTF_Quit();
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
  dst.y = static_cast<float>(gy * tilePx_ + Cfg::room::menu_bar_h);
  dst.w = static_cast<float>(tilePx_);
  dst.h = static_cast<float>(tilePx_);
  SDL_RenderTexture(renderer_, tex, nullptr, &dst);
}

void SDL3Render::loadDynamicTexture(const std::string& itemId, const std::string& textureName) {
  if (textureName.empty()) return;
  try {
    //如果已经有了先删掉
    if (itemTextures_.find(itemId) != itemTextures_.end()) {
      SDL_DestroyTexture(itemTextures_[itemId]);
    }
    itemTextures_[itemId] = loadTexture(RES(textureName.c_str()));
    spdlog::info("SDL3Render: Loaded texture: {}", itemId);
  } catch (const std::exception& e) {
    spdlog::error("SDL3Render: Failed to load texture: {}", itemId);
  }
}

void SDL3Render::render_frame(
  const ItemLayer& items_,
  const std::vector<std::unique_ptr<Agent>>& agents,
  const Room& room,
  const std::string& preview_item_id,
  int preview_x,
  int preview_y,
  bool show_triggers)
{

  clear();
  for (int y = 0; y < room.getHeight(); ++y) {
    for (int x = 0; x < room.getWidth(); ++x) {
      std::string tId = room.getBlocksType(x, y);
      auto it = terrainTextures_.find(tId);
      if (it != terrainTextures_.end()) {
        drawTile(x, y, it->second);
      }
    }
  }

  // 2. 画物品
  for (auto& [key, iid] : items_.items()) {
    int x = key % items_.getWidth();
    int y = key / items_.getWidth();

    auto it = itemTextures_.find(iid);
    if (it != itemTextures_.end()) {
      drawTile(x, y, it->second);
    }
  }

  // 3. 画小人
  for (const auto& agent : agents) {
    if (agent->getCharacter().getLevel() != WorldManager::inst().current_active_level) continue;
    const auto& c = agent->getCharacter();
    const int cx = c.getLoc().first;
    const int cy = c.getLoc().second;

    std::string texName = agent->getTextureName();
    auto it = agentTextures_.find(texName);
    if (it != agentTextures_.end()) {
      drawTile(cx, cy, it->second);
    } else {
      drawTile(cx, cy, texCharacter_);
    }
  }

  // 放置模式的绘制
  if (!preview_item_id.empty() && preview_x >= 0 && preview_y >= 0) {
    auto it = itemTextures_.find(preview_item_id);
    if (it != itemTextures_.end()) {
      SDL_Texture* tex = it->second;
      //设置纹理透明度,255:不透明,128:半透明
      SDL_SetTextureAlphaMod(tex, 128);
      drawTile(preview_x, preview_y, tex);
      // 画完后把透明度恢复
      SDL_SetTextureAlphaMod(tex, 255);
    }
  }

  // triggers绘制
  if (show_triggers && texTrigger_) {
    SDL_SetTextureAlphaMod(texTrigger_, 128);

    for (const auto& [id, trg] : TriggerManager::inst().getAllTriggers()) {
      if (trg.level != WorldManager::inst().current_active_level) continue;
      drawTile(trg.x, trg.y, texTrigger_);
    }

    SDL_SetTextureAlphaMod(texTrigger_, 255);
  }

}


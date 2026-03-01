//
// Created by jacob on 25-9-24.
//

#ifndef IRENDER_H
#define IRENDER_H

#include "../Character.h"
#include "../Item.h"
#include "../Room.h"
#include <memory>

struct SDL_Window;
struct SDL_Renderer;

class Agent;

class IRender {
  public:
  virtual void render_frame(
    const ItemLayer&,
    const std::vector<std::unique_ptr<Agent>>&,
    const Room&,
    const std::string& preview_item_id = "",
    int preview_x = -1,
    int preview_y = -1
    ) = 0;

  virtual ~IRender() = default;

  virtual SDL_Window* getWindow() = 0;
  virtual SDL_Renderer* getRenderer() = 0;
  virtual void loadDynamicTexture(const std::string& itemId, const std::string& textureName) = 0;
  virtual void loadTerrainTexture(const std::string& tileId, const std::string& textureName) = 0;
  virtual void loadAgentTexture(const std::string& textureName) = 0;
};



#endif //IRENDER_H

//
// Created by jacob on 26-2-25.
//

#include "InputController.h"
#include <iostream>

void InputController::handleEvents(bool &running,bool& is_paused, EditorUI &editorUI, const Room &room, ItemLayer &items, const std::vector<std::unique_ptr<Agent>> &agents) {
  SDL_Event e;
  while (SDL_PollEvent(&e)) {
    // 优先喂给 UI，如果被截获则跳过逻辑
    if (editorUI.processEvent(&e)) {
      continue;
    }

    // 基础控制
    if (e.type == SDL_EVENT_QUIT) {
      running = false;
    }
    if (e.type == SDL_EVENT_KEY_DOWN) {
      if (e.key.key == SDLK_ESCAPE) running = false;
      if (e.key.key == SDLK_SPACE) {
        is_paused = !is_paused;
        std::cout << (is_paused ? "System: Game Paused." : "System: Game Resumed.") << std::endl;
      }
    }

    // 鼠标网格坐标追踪,用于半透明预览
    if (e.type == SDL_EVENT_MOUSE_MOTION) {
      int gameW = Cfg::room::view_w * Cfg::core::tile_px;
      int gameH = Cfg::room::view_h * Cfg::core::tile_px;

      float mx = e.motion.x;
      float my = e.motion.y - Cfg::room::menu_bar_h;

      if (mx >= 0 && mx < gameW && my >= 0 && my < gameH) {
        mouse_gx = static_cast<int>(mx) / Cfg::core::tile_px;
        mouse_gy = static_cast<int>(my) / Cfg::core::tile_px;
      } else {
        mouse_gx = -1;
      }
    }

    // 左/右键点击逻辑分发
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
      int gameW = Cfg::room::view_w * Cfg::core::tile_px;
      int gameH = Cfg::room::view_h * Cfg::core::tile_px;
      float mx = e.button.x;
      float my = e.button.y - Cfg::room::menu_bar_h;
      bool in_game_view = (mx >= 0 && mx < gameW && my >= 0 && my < gameH);

      if (e.button.button == SDL_BUTTON_RIGHT) {
        if (current_mode == EditorMode::Placement) {
          current_mode = EditorMode::Observation;
          selected_placement_item = "";
        }
      } else if (e.button.button == SDL_BUTTON_LEFT && in_game_view) {
        int gx = static_cast<int>(mx) / Cfg::core::tile_px;
        int gy = static_cast<int>(my) / Cfg::core::tile_px;

        if (current_mode == EditorMode::Observation) {
          Agent *clicked_agent = nullptr;
          for (auto &a : agents) {
            if (a->getCharacter().getLoc() == std::make_pair(gx, gy)) {
              clicked_agent = a.get();
              break;
            }
          }
          editorUI.setSelectedAgent(clicked_agent);
        } else if (current_mode == EditorMode::Placement) {
          if (!selected_placement_item.empty() && room.getBlocksType(gx, gy) != TileType::WallH && room.getBlocksType(gx, gy) != TileType::WallV) {

            items.place(selected_placement_item, gx, gy);
            std::cout << "[Editor] Placed " << selected_placement_item << " at " << gx << "," << gy << std::endl;
          }
        }
      }
    }
  }
}
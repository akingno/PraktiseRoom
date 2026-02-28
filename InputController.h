//
// Created by jacob on 26-2-25.
//

#ifndef INPUTCONTROLLER_H
#define INPUTCONTROLLER_H

#include "Agent.h"
#include "EditorUI.h"
#include "Room.h"
#include <memory>
#include <string>
#include <vector>

class InputController {
 public:
  InputController() = default;

  EditorMode current_mode = EditorMode::Observation;
  std::string selected_placement_item = "";
  std::string selected_terrain_id = "grass"; // 当前笔刷
  bool is_left_mouse_down = false; // 左键是否按着了
  int mouse_gx = -1;
  int mouse_gy = -1;

  void handleEvents(bool &running, bool& is_paused, EditorUI &editorUI, Room &room, ItemLayer &items, const std::vector<std::unique_ptr<Agent>> &agents);
};

#endif//INPUTCONTROLLER_H

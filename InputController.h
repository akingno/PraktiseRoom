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
  int mouse_gx = -1;
  int mouse_gy = -1;

  void handleEvents(bool &running, bool& is_paused, EditorUI &editorUI, const Room &room, ItemLayer &items, const std::vector<std::unique_ptr<Agent>> &agents);
};

#endif//INPUTCONTROLLER_H

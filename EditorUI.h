//
// Created by jacob on 26-2-24.
//

#ifndef EDITORUI_H
#define EDITORUI_H
#include "Agent.h"
#include "ItemLayer.h"
#include <SDL3/SDL.h>
#include <vector>
#include <memory>
#include <string>

class IRender;

class EditorUI {
public:
  EditorUI() = default;
  ~EditorUI() = default;

  // 初始化与清理
  void init(IRender* render);
  void closeClear();

  // 接收事件
  // if return true: UI process了事件，游戏逻辑应该忽略
  bool processEvent(const SDL_Event* event);

  // 设置当前选中的小人
  void setSelectedAgent(Agent* agent) { selected_agent_ = agent; }

  // 主渲染逻辑
  void render(bool& is_paused,SDL_Renderer *renderer);
private:
  Agent* selected_agent_ = nullptr; // 当前被选中的小人
};



#endif //EDITORUI_H

//
// Created by jacob on 26-2-24.
//

#ifndef EDITORUI_H
#define EDITORUI_H
#include "Agent.h"
#include "GameContentInit.h"
#include "event_bus/EventBus.h"
#include <SDL3/SDL.h>
#include <string>

enum class EditorMode {
  Observation,
  Placement,
  TerrainPaint
};

class IRender;

class EditorUI {
public:
  EditorUI() = default;
  ~EditorUI() = default;

  // 初始化与清理
  void init(IRender* render);
  void closeClear();

  // 接收事件
  // if return true: UI process了事件，逻辑应该忽略
  bool processEvent(const SDL_Event* event);

  // 设置当前选中的小人
  void setSelectedAgent(Agent* agent) { selected_agent_ = agent; }

  // 主渲染逻辑
  void render(
    bool is_paused,
    SDL_Renderer *renderer,
    EditorMode& mode,
    std::string& selected_item_id,
    std::string& selected_terrain_id
    );

private:
  Agent* selected_agent_ = nullptr; // 当前被选中的小人

  // 物品创建器临时数据
  char new_item_id_[64] = "";
  char new_item_tex_[64] = "";
  bool new_item_useable_ = true;
  bool new_item_blocks_ = false;

  // 临时存放效果的数组
  char new_eff_target_[32] = "";
  float new_eff_value_ = 0.0f;

  // 需求创建器临时数据
  char new_need_name_[64] = "";
  float new_need_growth_ = 1.0f;
  float new_need_enter_ = 50.0f;
  float new_need_exit_ = 0.0f;
  float new_need_weight_ = 1.0f;

  char new_terrain_id_[64] = "";
  char new_terrain_tex_[64] = "";
  bool new_terrain_blocks_ = false;

  char new_agent_name_[64] = "";
  char new_agent_id_[64] = "";
  int new_agent_x_ = 0;
  int new_agent_y_ = 0;
  int new_agent_type_idx_ = 1;
  char new_agent_tex_[64] = "";


  void renderMenuBar();
  void renderRightPanel(bool is_paused, EditorMode& mode);
  void renderBottomPanel(
    EditorMode& mode,
    std::string& selected_item_id,
    std::string& selected_terrain_id);
};



#endif //EDITORUI_H

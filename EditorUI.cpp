//
// Created by jacob on 26-2-24.
//

#include "EditorUI.h"
#include "Config.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "renders/IRender.h"
#include <iostream>

void EditorUI::init(IRender *render) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void) io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  ImGui::StyleColorsDark();

  ImGui_ImplSDL3_InitForSDLRenderer(render->getWindow(), render->getRenderer());
  ImGui_ImplSDLRenderer3_Init(render->getRenderer());

  ImFont *font = io.Fonts->AddFontFromFileTTF("res/msyh.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
  if (font == nullptr) {
    std::cerr << "EditorUI: 字体加载失败！请检查res/msyh.ttc" << std::endl;
  }
}

void EditorUI::closeClear() {
  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();
}

bool EditorUI::processEvent(const SDL_Event *event) {
  ImGui_ImplSDL3_ProcessEvent(event);
  ImGuiIO &io = ImGui::GetIO();

  // 如果鼠标或键盘在UI上，返回true告诉主循环拦截它
  if (io.WantCaptureMouse && (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN || event->type == SDL_EVENT_MOUSE_BUTTON_UP || event->type == SDL_EVENT_MOUSE_MOTION)) {
    return true;
  }
  if (io.WantCaptureKeyboard && (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP)) {
    return true;
  }
  return false;
}

void EditorUI::renderMenuBar(ItemLayer& items, std::vector<std::unique_ptr<Agent>>& agents, IPathfinder* pf, IRender* irender, Room& room) {
  // 顶部菜单栏
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu(u8"文件 (File)")) {
      if (ImGui::MenuItem(u8"保存全部 (Save All World & Config)")) {
        saveItems("items.json");
        Cfg::save("config.json");
        items.saveToFile("world.json");
        saveTerrains("terrains.json");
        room.saveToFile("room_map.json");
        saveAgents(agents, "agents.json");
        std::cout << "EditorUI: 保存了所有更改！" << std::endl;
      }

      ImGui::Separator();

      if (ImGui::MenuItem(u8"读取物品")) {
        loadItems("items.json");
      }
      if (ImGui::MenuItem(u8"读取配置")) {
        Cfg::load("config.json");
      }
      if (ImGui::MenuItem(u8"读取世界")) {
        items.loadFromFile("world.json");
      }
      if (ImGui::MenuItem(u8"读取地形")) {
        loadTerrains("terrains.json");
      }
      if (ImGui::MenuItem(u8"读取房间地图")) {
        room.loadFromFile("room_map.json");
      }
      if (ImGui::MenuItem(u8"读取实体")) {
        loadAgents(agents, pf, irender, "agents.json");
      }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
}
void EditorUI::renderRightPanel(bool& is_paused, EditorMode& mode, const std::vector<std::unique_ptr<Agent>>& agents) {
  int gameW = Cfg::room::view_w * Cfg::core::tile_px;
  int gameH = Cfg::room::view_h * Cfg::core::tile_px;
  float menuHeight = static_cast<float>(Cfg::room::menu_bar_h);

  ImGui::SetNextWindowPos(ImVec2(gameW, menuHeight), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(350, gameH + 250), ImGuiCond_Always);
  ImGui::Begin(u8"属性检视", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

  // 状态显示
  if (mode == EditorMode::Observation) {
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), u8"当前模式: 观察 (左键查看小人)");
  } else if (mode == EditorMode::Placement) {
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), u8"当前模式: 放置 (左键放置, 右键取消)");
  }
  ImGui::Separator();

  // 系统控制区
  if (is_paused) {
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), u8" ⏸已暂停 (按空格恢复)");
  } else {
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), u8" ▶ 运行中");
  }
  ImGui::Separator();
  ImGui::Spacing();

  // 实体检视区
  if (selected_agent_) {
    const Character &ch = selected_agent_->getCharacter();
    ImGui::Text(u8"当前选中: %s", selected_agent_->getName().c_str());
    ImGui::Text(u8"当前动作: %s", Character::Act2Str(ch.act()));
    ImGui::Spacing();

    ImGui::Text(u8"【内部需求状态】");
    for (const auto &[stat_name, value] : ch.getAllStats()) {
      float ratio = static_cast<float>(value) / 100.0f;
      char buf[32];
      sprintf(buf, "%s: %.1f", stat_name.c_str(), value);
      ImGui::ProgressBar(ratio, ImVec2(-1.0f, 0.0f), buf);
    }
  } else {
    ImGui::TextDisabled(u8"点击地图上的小人以查看其属性");
  }

  ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
  ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), u8"【全局需求定义】");
  ImGui::InputText(u8"需求名(如:thirst)", new_need_name_, IM_ARRAYSIZE(new_need_name_));
  ImGui::InputFloat(u8"每秒增长率", &new_need_growth_, 0.1f, 1.0f, "%.1f");
  ImGui::InputFloat(u8"触发进入阈值", &new_need_enter_, 1.0f, 10.0f, "%.1f");
  ImGui::InputFloat(u8"满足退出阈值", &new_need_exit_, 1.0f, 10.0f, "%.1f");
  ImGui::InputFloat(u8"需求权重", &new_need_weight_, 0.1f, 1.0f, "%.1f");

  if (new_need_growth_ < 0.0f) new_need_growth_ = 0.0f;
  if (new_need_enter_ < 0.0f) new_need_enter_ = 0.0f;
  if (new_need_enter_ > 100.0f) new_need_enter_ = 100.0f;
  if (new_need_exit_ < 0.0f) new_need_exit_ = 0.0f;
  if (new_need_exit_ > 100.0f) new_need_exit_ = 100.0f;
  if (new_need_weight_ < 0.0f) new_need_weight_ = 0.0f;
  if (new_need_exit_ > new_need_enter_) { new_need_exit_ = new_need_enter_; }

  ImGui::Spacing();
  if (ImGui::Button(u8"应用新需求", ImVec2(-1.0f, 30))) {
    std::string needStr = new_need_name_;
    if (!needStr.empty()) {
      bool exists = false;
      for (const auto &r : Cfg::need_rules) {
        if (r.name == needStr) { exists = true; break; }
      }
      if (!exists) {
        Cfg::NeedRule rule;
        rule.name = needStr; rule.growth_rate = new_need_growth_;
        rule.enter_threshold = new_need_enter_; rule.exit_threshold = new_need_exit_;
        rule.weight = new_need_weight_;
        Cfg::need_rules.push_back(rule);
        for (auto &agent : agents) {
          agent->getCharacter().registerNewStat(needStr, new_need_growth_);
        }
        memset(new_need_name_, 0, sizeof(new_need_name_));
      } else {
        std::cout << "EditorUI: 需求" << needStr << "已经存在，无需重复添加。" << std::endl;
      }
    }
  }
  ImGui::End();
}


void EditorUI::renderBottomPanel(IRender* irender, EditorMode& mode, std::string& selected_item_id, std::string& selected_terrain_id,ItemLayer& items,std::vector<std::unique_ptr<Agent>> &agents, IPathfinder *pf) {
  int gameW = Cfg::room::view_w * Cfg::core::tile_px;
  int gameH = Cfg::room::view_h * Cfg::core::tile_px;

  float startY = static_cast<float>(gameH + Cfg::room::menu_bar_h);

  ImGui::SetNextWindowPos(ImVec2(0, startY), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(gameW, 250), ImGuiCond_Always);
  ImGui::Begin(u8"物品创建和放置/需求创建", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

  if (ImGui::BeginTabBar("BottomTabs")) {
    if (ImGui::BeginTabItem(u8" 物品放置")) {
      ImGui::Spacing();
      std::string combo_preview = selected_item_id.empty() ? u8"请选择物品..." : selected_item_id;
      if (ImGui::BeginCombo(u8"选择要放置的物品", combo_preview.c_str())) {
        for (const auto &[id, item] : ItemRegistry::inst().getAllItems()) {
          bool is_selected = (selected_item_id == id);
          if (ImGui::Selectable(id.c_str(), is_selected)) {
            selected_item_id = id; mode = EditorMode::Placement;
          }
          if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem(u8" 创造物品")) {
      ImGui::InputText(u8"物品 ID (如: water)", new_item_id_, IM_ARRAYSIZE(new_item_id_));
      ImGui::InputText(u8"贴图名 (如: water.png)", new_item_tex_, IM_ARRAYSIZE(new_item_tex_));
      ImGui::Checkbox(u8"可被使用", &new_item_useable_);
      ImGui::SameLine();
      ImGui::Checkbox(u8"阻挡寻路", &new_item_blocks_);
      ImGui::Separator();
      ImGui::Text(u8"添加效果");
      ImGui::InputText(u8"目标属性 (如: thirst)", new_eff_target_, IM_ARRAYSIZE(new_eff_target_));
      ImGui::InputFloat(u8"属性变化数值 (如: -50)", &new_eff_value_);

      ImGui::Spacing();
      if (ImGui::Button(u8"创建", ImVec2(200, 30))) {
        std::string idStr = new_item_id_;
        if (!idStr.empty()) {
          ItemProps props{false, new_item_blocks_, new_item_useable_, new_item_tex_};
          auto smartItem = std::make_unique<SmartItem>(idStr, props);
          std::string targetStr = new_eff_target_;
          if (!targetStr.empty()) {
            smartItem->addEffect({EffectType::ModifyStat, targetStr, static_cast<double>(new_eff_value_)});
          }
          smartItem->setSequence({{"MoveToTarget", 0, ""}, {"Interact", 0, ""}});
          ItemRegistry::inst().register_item(std::move(smartItem));
          irender->loadDynamicTexture(idStr, new_item_tex_);
          memset(new_item_id_, 0, sizeof(new_item_id_));
        }
      }
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(u8"地形画笔")) {
      ImGui::Spacing();
      std::string combo_preview = selected_terrain_id.empty() ? u8"请选择笔刷..." : selected_terrain_id;
      if (ImGui::BeginCombo(u8"地形笔刷", combo_preview.c_str())) {
        for (const auto &[id, def] : TerrainRegistry::inst().getAllTerrains()) {
          bool is_selected = (selected_terrain_id == id);
          if (ImGui::Selectable(id.c_str(), is_selected)) {
            selected_terrain_id = id;
            mode = EditorMode::TerrainPaint;
          }
          if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(u8"创造地形")){
      ImGui::Spacing();
      ImGui::InputText(u8"地形 ID (如: lava)", new_terrain_id_, IM_ARRAYSIZE(new_terrain_id_));
      ImGui::InputText(u8"地形贴图 (如: lava.png)", new_terrain_tex_, IM_ARRAYSIZE(new_terrain_tex_));
      ImGui::Checkbox(u8"阻挡寻路 (无法通行)", &new_terrain_blocks_);

      ImGui::Spacing();
      if (ImGui::Button(u8"生成地形并热加载", ImVec2(200, 30))) {
        std::string idStr = new_terrain_id_;
        if (!idStr.empty()) {
          TerrainDef def{idStr, new_terrain_tex_, new_terrain_blocks_};
          TerrainRegistry::inst().register_terrain(def);
          irender->loadTerrainTexture(idStr, new_terrain_tex_);
          memset(new_terrain_id_, 0, sizeof(new_terrain_id_));
        }
      }
      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem(u8"创造实体")) {
      ImGui::Spacing();
      ImGui::InputText(u8"实体名字 (如: 王五)", new_agent_name_, IM_ARRAYSIZE(new_agent_name_));
      ImGui::InputText(u8"实体id (如: npc_01)", new_agent_id_, IM_ARRAYSIZE(new_agent_id_));
      ImGui::InputText(u8"实体贴图 (如: character.png)", new_agent_tex_, IM_ARRAYSIZE(new_agent_tex_));
      ImGui::InputInt(u8"坐标X", &new_agent_x_);
      ImGui::InputInt(u8"坐标Y", &new_agent_y_);

      const char* ai_types[] = { "Static(静态NPC)", "Utility(效用AI)", "LLM(大模型驱动)", "Player(玩家控制)"};
      ImGui::Combo(u8"大脑类型", &new_agent_type_idx_, ai_types, IM_ARRAYSIZE(ai_types));

      ImGui::Spacing();
      if (ImGui::Button(u8"在指定位置创造该实体", ImVec2(200, 30))) {
        std::string nameStr = new_agent_name_;
        std::string idStr = new_agent_id_;
        std::string texStr = new_agent_tex_;
        if (!nameStr.empty() && pf) {
          AIType type = static_cast<AIType>(new_agent_type_idx_);
          // TODO：只有效用ai放agent里，或者进行agent的通用化
          agents.push_back(std::make_unique<Agent>(nameStr, idStr, new_agent_x_, new_agent_y_, pf, type, texStr));
          irender->loadAgentTexture(texStr);
          std::cout << "[Editor] Spawned Agent: " << nameStr << " at " << new_agent_x_ << "," << new_agent_y_ << "\n";
          memset(new_agent_name_, 0, sizeof(new_agent_name_));
          memset(new_agent_id_, 0, sizeof(new_agent_id_));
        }
      }
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }
  ImGui::End();
}

void EditorUI::render(bool &is_paused,
  SDL_Renderer *renderer,
  IRender *irender,
  EditorMode &mode,
  std::string &selected_item_id,
  std::string& selected_terrain_id,
  std::vector<std::unique_ptr<Agent>> &agents,
  ItemLayer &items,
  IPathfinder *pf,
  Room& room
  ) {

  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  // 渲染顶部菜单、右边菜单和底部菜单
  renderMenuBar(items, agents, pf, irender, room);
  renderRightPanel(is_paused, mode, agents);
  renderBottomPanel(irender, mode, selected_item_id,selected_terrain_id, items, agents, pf);

  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}
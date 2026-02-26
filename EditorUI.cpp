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

void EditorUI::init(IRender* render) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(render->getWindow(), render->getRenderer());
    ImGui_ImplSDLRenderer3_Init(render->getRenderer());

    ImFont* font = io.Fonts->AddFontFromFileTTF("res/msyh.ttc", 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    if (font == nullptr) {
        std::cerr << "EditorUI: 字体加载失败！请检查res/msyh.ttc" << std::endl;
    }
}

void EditorUI::closeClear() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

bool EditorUI::processEvent(const SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);
    ImGuiIO& io = ImGui::GetIO();

    // 如果鼠标或键盘在UI上，返回true告诉主循环拦截它
    if (io.WantCaptureMouse && (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN || event->type == SDL_EVENT_MOUSE_BUTTON_UP || event->type == SDL_EVENT_MOUSE_MOTION)) {
        return true;
    }
    if (io.WantCaptureKeyboard && (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP)) {
        return true;
    }
    return false;
}

void EditorUI::render(bool& is_paused, SDL_Renderer *renderer, IRender* irender, EditorMode& mode, std::string& selected_item_id, const std::vector<std::unique_ptr<Agent>>& agents) {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  int gameW = Cfg::room::view_w * Cfg::core::tile_px;
  int gameH = Cfg::room::view_h * Cfg::core::tile_px;

  //属性检视
  ImGui::SetNextWindowPos(ImVec2(gameW, 0), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(350, gameH + 250), ImGuiCond_Always);
  ImGui::Begin(u8"属性检视", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
  //状态显示
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
    const Character& ch = selected_agent_->getCharacter();
    ImGui::Text(u8"当前选中: %s", selected_agent_->getName().c_str());
    ImGui::Text(u8"当前动作: %s", Character::Act2Str(ch.act()));
    ImGui::Spacing();

    ImGui::Text(u8"【内部需求状态】");
    for (const auto& [stat_name, value] : ch.getAllStats()) {
        float ratio = static_cast<float>(value) / 100.0f;
        char buf[32];
        sprintf(buf, "%s: %.1f", stat_name.c_str(), value);
        ImGui::ProgressBar(ratio, ImVec2(-1.0f, 0.0f), buf);
    }
  } else {
    ImGui::TextDisabled(u8"点击地图上的小人以查看其属性");
  }

  ImGui::End();

  //物品区
  ImGui::SetNextWindowPos(ImVec2(0, gameH), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(gameW, 250), ImGuiCond_Always);
  ImGui::Begin(u8"物品创建和放置/需求创建", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

  if (ImGui::BeginTabBar("BottomTabs")) {

      // --- Tab 1: 物品放置 ---
      if (ImGui::BeginTabItem(u8"🛠️ 物品放置")) {
          ImGui::Spacing();
          std::string combo_preview = selected_item_id.empty() ? u8"请选择物品..." : selected_item_id;
          if (ImGui::BeginCombo(u8"选择要放置的物品", combo_preview.c_str())) {
              for (const auto& [id, item] : ItemRegistry::inst().getAllItems()) {
                  bool is_selected = (selected_item_id == id);
                  if (ImGui::Selectable(id.c_str(), is_selected)) {
                      selected_item_id = id;
                      mode = EditorMode::Placement;
                  }
                  if (is_selected) ImGui::SetItemDefaultFocus();
              }
              ImGui::EndCombo();
          }
          ImGui::EndTabItem();
      }

      // --- Tab 2: 创造新物品 ---
      if (ImGui::BeginTabItem(u8"✨ 创造物品")) {
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
          if (ImGui::Button(u8"生成并热加载", ImVec2(200, 30))) {
              std::string idStr = new_item_id_;
              if (!idStr.empty()) {
                  ItemProps props{false, new_item_blocks_, new_item_useable_, new_item_tex_};
                  auto smartItem = std::make_unique<SmartItem>(idStr, props);

                  // 效果
                  std::string targetStr = new_eff_target_;
                  if (!targetStr.empty()) {
                      smartItem->addEffect({EffectType::ModifyStat, targetStr, static_cast<double>(new_eff_value_)});
                  }
                  // 默认配一个Interact动作保证能被使用
                  smartItem->setSequence({ {"MoveToTarget", 0, ""}, {"Interact", 0, ""} });

                  // 注册
                  ItemRegistry::inst().register_item(std::move(smartItem));

                  // 渲染器热加载贴图
                  irender->loadDynamicTexture(idStr, new_item_tex_);

                  // 持久化
                  saveItems("items.json");

                  // 清空输入框
                  memset(new_item_id_, 0, sizeof(new_item_id_));
              }
          }
          ImGui::EndTabItem();
      }

      // --- Tab 3: 创造新需求 ---
      if (ImGui::BeginTabItem(u8"定义需求")) {
          ImGui::InputText(u8"需求名称 (如: thirst)", new_need_name_, IM_ARRAYSIZE(new_need_name_));
          ImGui::SliderFloat(u8"每秒增长率0-10", &new_need_growth_, 0.1f, 10.0f);
          ImGui::SliderFloat(u8"触发动作阈值0-100", &new_need_enter_, 10.0f, 100.0f);
          ImGui::SliderFloat(u8"满足退出阈值0-100", &new_need_exit_, 0.0f, 90.0f);
          ImGui::SliderFloat(u8"权重,紧急:1.5,普通:1.0", &new_need_weight_, 0.1f, 5.0f);

          ImGui::Spacing();
          if (ImGui::Button(u8"应用新需求并保存！", ImVec2(200, 30))) {
              std::string needStr = new_need_name_;
              if (!needStr.empty()) {
                  Cfg::NeedRule rule;
                  rule.name = needStr; rule.growth_rate = new_need_growth_;
                  rule.enter_threshold = new_need_enter_; rule.exit_threshold = new_need_exit_;
                  rule.weight = new_need_weight_;
                  Cfg::need_rules.push_back(rule);

                  for (auto& agent : agents) {
                      agent->getCharacter().registerNewStat(needStr, new_need_growth_);
                  }

                  Cfg::save("config.json");
              }
          }
          ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
  }
  ImGui::End();

  // 渲染指令下发
  ImGui::Render();
  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),renderer);
}
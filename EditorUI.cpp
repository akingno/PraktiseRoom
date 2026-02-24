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

void EditorUI::render(bool& is_paused,SDL_Renderer *renderer) {
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    int gameW = Cfg::room::view_w * Cfg::core::tile_px;
    int gameH = Cfg::room::view_h * Cfg::core::tile_px;

    //属性检视
    ImGui::SetNextWindowPos(ImVec2(gameW, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(350, gameH + 250), ImGuiCond_Always);
    ImGui::Begin(u8"属性检视", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

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

    //物品创建和放置面板
    ImGui::SetNextWindowPos(ImVec2(0, gameH), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(gameW, 250), ImGuiCond_Always);
    ImGui::Begin(u8"物品创建和放置面板", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::Text(u8"这里是物品创建和放置面板");
    ImGui::End();

    // 渲染指令下发
    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(),renderer);
}
//
// Created by jacob on 26-3-1.
//

#ifndef EVENTBUS_H
#define EVENTBUS_H

#include "../Agent.h"
#include "Signal.h"
#include <string>

struct EventBus {
  inline static Signal<> onUI_SaveAllRequested;
  inline static Signal<> onUI_LoadItemsRequested;
  inline static Signal<> onUI_LoadConfigRequested;
  inline static Signal<> onUI_LoadWorldRequested;
  inline static Signal<> onUI_LoadTerrainsRequested;
  inline static Signal<> onUI_LoadRoomMapRequested;
  inline static Signal<> onUI_LoadAgentsRequested;
  inline static Signal<std::string, std::string> onTriggerStepped;

  // signal1: UI请求创建一个新的agent
  // args: 名字, ID, X, Y, ai类型, 贴图名字
  inline static Signal<std::string, std::string, int, int, AIType, std::string> onUI_CreateAgent;

  // sigal2: UI请求创建新物品
  //args: id, texture, usable, blocks, target_state, effect_value
  inline static Signal<std::string, std::string, bool, bool, std::string, float> onUI_CreateItem;

  //signal 3: UI请求创造新地形
  //args: id, texture, blocks
  inline static Signal<std::string, std::string, bool> onUI_CreateTerrain;

  // signal 4: UI请求创建新需求
  // args: name, growth, enter, exit, weight
  inline static Signal<std::string, float, float, float, float> onUI_ApplyNewNeed;

  // signal 5: UI请求增加新动作序列
  inline static Signal<std::string, std::vector<ActionDescriptor>> onStaticSequenceUpdated;

  // signal 6 UI请求增加新trigger
  // args: x, y, target id
  inline static Signal<int, int, std::string> onUI_CreateTrigger;


};

#endif //EVENTBUS_H

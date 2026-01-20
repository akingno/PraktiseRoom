//
// Created by jacob on 26-1-19.
//

#ifndef BRAIN_H
#define BRAIN_H

#include <future>
#include "Blackboard.h"

// 打开后将走 HTTP；不开则走本地 Utility 逻辑。
#define USE_LLM_HTTP_SERVER

#ifdef USE_LLM_HTTP_SERVER
#include <nlohmann/json.hpp>
#include <httplib/httplib.h>
#endif

struct BrainResult {
  Character::Act act;
  std::string thought; // 新的记忆/思考内容
};

class Brain {
public:
  Brain() = default;
  ~Brain();

  // 在主线程调用：快照参数全部按值传入，避免竞态
  void requestDecision(const Character& chSnap,
                       bool is_being_called,
                       const std::string& name,
                       uint64_t   nowTick,
                       bool       hasFood,
                       bool       hasBed,
                       bool       hasComputer);

  // 主线程轮询：若结果就绪则写入 bb.actionQueue
  void poll(Blackboard& bb, Character& ch);

  bool isThinking() const;

private:
  std::future<BrainResult> _fut;

  BrainResult localUtility(const Character& ch,
                              bool is_being_called,
                              uint64_t nowTick,
                              bool f, bool b, bool c);
};



#endif //BRAIN_H

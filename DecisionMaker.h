//
// Created by jacob on 26-1-20.
//

#ifndef DECISIONMAKER_H
#define DECISIONMAKER_H


#include <future>
#include <vector>
#include <map>
#include <string>
#include "Character.h"

class Agent;

//打开则走http
#define USE_LLM_HTTP_SERVER

#ifdef USE_LLM_HTTP_SERVER
#include <nlohmann/json.hpp>
#include <httplib/httplib.h>
#endif

// 单个 Agent 的决策结果
struct DecisionResult {
  Character::Act act;
  std::string thought;
};

// 线程安全的快照数据结构
struct AgentSnapshot {
  std::string name;
  double hunger;
  double fatigue;
  double boredom;
  bool hasFood;
  bool hasBed;
  bool hasComputer;
  bool isBeingCalled;
  std::vector<std::string> memories;
  Character::Act currentAct;
};

class DecisionMaker {
public:
  DecisionMaker() = default;
  ~DecisionMaker();

  /**
   * @brief 发起一次批量请求 (主线程调用)
   * @param agents 所有需要参与决策的 Agent 指针
   * @param nowTick 当前时间 tick
   */
  void requestBatchDecision(const std::vector<Agent*>& agents, uint64_t nowTick);

  /**
   * @brief 轮询结果 (主线程调用)
   * @param agents 将根据名字匹配结果，并应用到 Agent 身上
   */
  void poll(std::vector<Agent*>& agents);

  bool isThinking() const;

private:
  // 返回值是一个 Map: Key=AgentName, Value=DecisionResult
  std::future<std::map<std::string, DecisionResult>> _fut;

  // 本地逻辑 (Fallback)
  std::map<std::string, DecisionResult> localUtilityBatch(const std::vector<AgentSnapshot>& snapshots);
};



#endif //DECISIONMAKER_H

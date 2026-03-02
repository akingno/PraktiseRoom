//
// Created by jacob on 26-1-20.
//

#include "DecisionMaker.h"
#include "Agent.h"
#include "Character.h"
#include "tools/Utils.h"

DecisionMaker::~DecisionMaker() {
  if (_fut.valid()) _fut.wait();
}

void DecisionMaker::requestBatchDecision(const std::vector<Agent *> &agents, uint64_t nowTick, const ItemLayer &items) {
  if (_fut.valid()) return;

  // 扫描地图，收集所有可用的物品
  std::vector<ItemSnapshot> itemSnapshots;
  for (const auto &[key, id] : items.items()) {
    if (Item *baseItem = ItemRegistry::inst().get(id)) {
      if (auto *smartItem = dynamic_cast<SmartItem *>(baseItem)) {
        int x = key % Cfg::room::view_w;
        int y = key / Cfg::room::view_w;
        itemSnapshots.push_back({id, {x, y}, smartItem});
      }
    }
  }

  std::vector<AgentSnapshot> snapshots;
  snapshots.reserve(agents.size());

  for (const auto *agent : agents) {
    const Character &ch = agent->getCharacter();
    AgentSnapshot snap;
    snap.name = agent->getName();
    snap.stats = ch.getAllStats();
    snap.isBeingCalled = agent->isBeingCalled();
    snap.currentAct = ch.act();
    snap.targetItemId = agent->getTargetItemId();

    for (const auto &mem : ch.get_short_memory().entries()) {
      snap.memories.push_back(mem.content);
    }
    snapshots.push_back(snap);
  }

  _fut = std::async(std::launch::async, [snapshots, itemSnapshots, nowTick]() {

#ifdef USE_LLM_HTTP_SERVER
    std::map<std::string, DecisionResult> results;
    using nlohmann::json;
    try {
      // A. 构建 Batch Request JSON
      json jBatch = json::array();// 这是一个数组

      for (const auto &snap : snapshots) {
        json j;
        j["name"] = snap.name;
        j["stats"] = snap.stats;
        j["nowTick"] = nowTick;
        j["hasFood"] = snap.hasFood;
        j["hasBed"] = snap.hasBed;
        j["hasComputer"] = snap.hasComputer;
        j["memories"] = snap.memories;
        jBatch.push_back(j);
      }

      // B. 发送请求
      httplib::Client cli("http://127.0.0.1:8000");
      cli.set_connection_timeout(0, 300000);
      cli.set_read_timeout(20, 0);

      // 发送给 /decide_batch 接口
      auto res = cli.Post("/decide_batch", jBatch.dump(), "application/json");

      if (res && res->status == 200) {
        // C. 解析 Batch Response
        auto rRoot = json::parse(res->body);

        if (rRoot.contains("decisions")) {
          auto decisionsMap = rRoot["decisions"];

          for (auto &[key, value] : decisionsMap.items()) {
            std::string agentName = key;
            std::string actStr = value.value("action", "Wander");
            std::string thought = value.value("thought", "");

            Character::Act finalAct = Character::Act::Wander;
            if (actStr == "Eat") finalAct = Character::Act::Eat;
            else if (actStr == "Sleep")
              finalAct = Character::Act::Sleep;
            else if (actStr == "UsePC")
              finalAct = Character::Act::UseComputer;
            else if (actStr == "Talk")
              finalAct = Character::Act::Talk;
            else if (actStr == "Stop")
              finalAct = Character::Act::Stop;
            else if (actStr == "WaitAlways")
              finalAct = Character::Act::WaitAlways;

            results[agentName] = {finalAct, thought};
          }
          return results;
        }
      }
    } catch (const std::exception &e) {
      printf("[DecisionMaker] HTTP Error: %s\n", e.what());
    }
#endif
    //本地逻辑
    DecisionMaker dm;
    return dm.localUtilityBatch(snapshots, itemSnapshots);
  });
}

void DecisionMaker::poll(std::vector<Agent *> &agents) {
  if (!_fut.valid()) return;
  using namespace std::chrono_literals;
  if (_fut.wait_for(0s) != std::future_status::ready) return;

  // 获取所有人的结果
  auto resultsMap = _fut.get();

  // 分发给agents
  for (auto *agent : agents) {
    std::string name = agent->getName();

    // 检查该是否有结果
    if (resultsMap.find(name) != resultsMap.end()) {
      const auto &res = resultsMap[name];

      // 写入记忆
      if (!res.thought.empty()) {
        agent->getCharacter().short_memory().add("[Thought] " + res.thought);
      }

      agent->applyDecision(res.act, res.targetItemId, res.targetPos);
    }
  }
}

bool DecisionMaker::isThinking() const {
  return _fut.valid();
}

std::map<std::string, DecisionResult> DecisionMaker::localUtilityBatch(
    const std::vector<AgentSnapshot> &snapshots,
    const std::vector<ItemSnapshot> &availableItems) {
  std::map<std::string, DecisionResult> results;

  for (const auto &agent : snapshots) {
    if (agent.isBeingCalled) {
      results[agent.name] = {Character::Act::WaitAlways, "", "", {-1, -1}};
      continue;
    }
    double scoreTalk = 0.0;
    if (agent.currentAct == Character::Act::Talk) {
      scoreTalk = Cfg::score::base_talk;
    }

    Character::Act chosen = Character::Act::Wander;
    double best = Cfg::score::base_wander;

    std::string targetId = "";
    std::pair<int, int> targetPos = {-1, -1};

    // 遍历所有需求 -> 寻找能满足该需求的物品
    for (const auto &rule : Cfg::need_rules) {
      double currentStat = agent.getStat(rule.name);

      // 如果还没达到阈值，就不找了
      if (currentStat <= rule.enter_threshold) continue;

      // 在全地图扫描能满足该rule.name的物品
      for (const auto &itemSnap : availableItems) {
        bool canSatisfy = false;
        // 检查物品的效果列表中，是否有降低该需求的设定
        for (const auto &eff : itemSnap.smartItemPtr->getEffects()) {
          if (eff.type == EffectType::ModifyStat && eff.target == rule.name && eff.value < 0.0) {
            canSatisfy = true;
            break;
          }
        }
        if (canSatisfy) {
          // 找到了则计算得分
          bool isDoingIt = (agent.currentAct == Character::Act::UseItem && agent.targetItemId == itemSnap.id);
          double score = CalcScoreGeneric(
              currentStat,
              rule.enter_threshold,
              rule.exit_threshold,
              true,// hasItem
              isDoingIt,
              rule.weight);

          if (score > best) {
            best = score;
            chosen = Character::Act::UseItem;
            targetId = itemSnap.id;
            targetPos = itemSnap.pos;
          }
        }
      }
    }

    // 最后比对一下说话的分数得到分数
    if (scoreTalk > best) {
      best = scoreTalk;
      chosen = Character::Act::Talk;
      targetId = "";
      targetPos = {-1, -1};
    }

    results[agent.name] = {chosen, "", targetId, targetPos};
  }
  return results;
}
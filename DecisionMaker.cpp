//
// Created by jacob on 26-1-20.
//

#include "DecisionMaker.h"
#include "Agent.h"
#include "actions/ActionFactory.h"
#include "tools/Utils.h"

DecisionMaker::~DecisionMaker() {
    if (_fut.valid()) _fut.wait();
}

void DecisionMaker::requestBatchDecision(const std::vector<Agent*>& agents, uint64_t nowTick) {
    if (_fut.valid()) return; // 上一轮还没结束

    // 先构建所有agent的快照并深拷贝放置agent的数据出现变化
    std::vector<AgentSnapshot> snapshots;
    snapshots.reserve(agents.size());

    for (const auto* agent : agents) {
        const Character& ch = agent->getCharacter();

        AgentSnapshot snap;
        snap.name = agent->getName();
        snap.hunger = ch.get_hunger_inner();
        snap.fatigue = ch.get_fatigue_score();
        snap.boredom = ch.get_boredom();
        snap.isBeingCalled = agent->isBeingCalled();
        snap.currentAct = ch.act();

        for(const auto& mem : ch.get_short_memory().entries()) {
            snap.memories.push_back(mem.content);
        }

        //环境感知，目前都是true
        snap.hasFood = true;
        snap.hasBed = true;
        snap.hasComputer = true;

        snapshots.push_back(snap);
    }

    // 2. 启动后台线程
    _fut = std::async(std::launch::async, [snapshots, nowTick]() {
        std::map<std::string, DecisionResult> results;

#ifdef USE_LLM_HTTP_SERVER
        using nlohmann::json;
        try {
            // A. 构建 Batch Request JSON
            json jBatch = json::array(); // 这是一个数组

            for(const auto& snap : snapshots) {
                json j;
                j["name"] = snap.name;
                j["hunger"] = snap.hunger;
                j["fatigue"] = snap.fatigue;
                j["boredom"] = snap.boredom;
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

                    for (auto& [key, value] : decisionsMap.items()) {
                        std::string agentName = key;
                        std::string actStr = value.value("action", "Wander");
                        std::string thought = value.value("thought", "");

                        Character::Act finalAct = Character::Act::Wander;
                        if (actStr == "Eat") finalAct = Character::Act::Eat;
                        else if (actStr == "Sleep") finalAct = Character::Act::Sleep;
                        else if (actStr == "UsePC") finalAct = Character::Act::UseComputer;
                        else if (actStr == "Talk") finalAct = Character::Act::Talk;
                        else if (actStr == "Stop") finalAct = Character::Act::Stop;
                        else if (actStr == "WaitAlways") finalAct = Character::Act::WaitAlways;

                        results[agentName] = {finalAct, thought};
                    }
                    return results;
                }
            }
        } catch (const std::exception& e) {
            printf("[DecisionMaker] HTTP Error: %s\n", e.what());
        }
#endif
        //本地逻辑
        DecisionMaker dm;
        return dm.localUtilityBatch(snapshots);
    });
}

void DecisionMaker::poll(std::vector<Agent*>& agents) {
    if (!_fut.valid()) return;
    using namespace std::chrono_literals;
    if (_fut.wait_for(0s) != std::future_status::ready) return;

    // 获取所有人的结果
    auto resultsMap = _fut.get();

    // 分发给agents
    for (auto* agent : agents) {
        std::string name = agent->getName();

        // 检查该是否有结果
        if (resultsMap.find(name) != resultsMap.end()) {
            const auto& res = resultsMap[name];

            // 写入记忆
            if (!res.thought.empty()) {
                agent->getCharacter().short_memory().add("[Thought] " + res.thought);
            }

             agent->applyDecision(res.act);
        }
    }
}

bool DecisionMaker::isThinking() const {
    return _fut.valid();
}

std::map<std::string, DecisionResult> DecisionMaker::localUtilityBatch(const std::vector<AgentSnapshot>& snapshots) {
    std::map<std::string, DecisionResult> results;

    for (const auto& agent : snapshots) {
      if (agent.isBeingCalled) {
          results[agent.name] = {Character::Act::WaitAlways, ""};
          continue;
      }
        double scoreTalk = 0.0;
      if (agent.currentAct == Character::Act::Talk) {
        scoreTalk = BASE_TALK;
      }

      double scoreUseComputer = CalcScoreUseComputer(
           agent.boredom, agent.hasComputer, agent.currentAct == Character::Act::UseComputer,
           BORED_ENTER, BORED_EXIT);

      double scoreEat = CalcScoreEat(
          agent.hunger, agent.hasFood, true /*eatAvailable*/,
          agent.currentAct == Character::Act::Eat, HUNGER_ENTER);

      double scoreSleep = CalcScoreSleep(
          agent.fatigue, agent.hasBed, agent.currentAct == Character::Act::Sleep,
          TIRED_ENTER, RESTED_EXIT);

      Character::Act chosen = Character::Act::Wander;
      double best = BASE_WANDER;

      if (scoreEat > best) { best = scoreEat; chosen = Character::Act::Eat; }
      if (scoreSleep > best) { best = scoreSleep; chosen = Character::Act::Sleep; }
      if (scoreUseComputer > best) { best = scoreUseComputer; chosen = Character::Act::UseComputer; }
      if (scoreTalk > best) { best = scoreTalk; chosen = Character::Act::Talk; }

      results[agent.name] = {chosen, ""};
    }
    return results;
}
//
// Created by jacob on 26-1-19.
//

#include "Brain.h"
#include "actions/ActionFactory.h"
#include "tools/Utils.h"

Brain::~Brain()
{
    if (_fut.valid()) _fut.wait();
}

void Brain::requestDecision(const Character& chSnap,
                            const bool is_being_called,
                            const std::string& name,
                            uint64_t   nowTick,
                            bool       hasFood,
                            bool       hasBed,
                            bool       hasComputer)
{
    if (_fut.valid()) return; // 上一次还没取完结果
    _fut = std::async(std::launch::async, [=]() {

        /* ------------------------ 可插拔分支 ------------------------ */
#ifdef USE_LLM_HTTP_SERVER
      using nlohmann::json;
      try {
          json j;
          j["name"] = name;
          j["hunger"] = chSnap.get_hunger_inner();
          j["fatigue"] = chSnap.get_fatigue_score();
          j["boredom"] = chSnap.get_boredom();
          j["nowTick"] = nowTick;
          j["hasFood"] = hasFood;
          j["hasBed"] = hasBed;
          j["hasComputer"] = hasComputer;

          // 1. 发送记忆列表
          j["memories"] = json::array();
          for(const auto& mem : chSnap.get_short_memory().entries()) {
              j["memories"].push_back(mem.content);
          }

          httplib::Client cli("http://127.0.0.1:8000");
          // 设置超时，防止 C++ 卡死
          cli.set_connection_timeout(0, 300000); // 300ms 连接
          cli.set_read_timeout(20, 0); // 5s 读取 (给 LLM 留时间)

          auto res = cli.Post("/decide", j.dump(), "application/json");

          if (res && res->status == 200) {
              auto r = json::parse(res->body);

              std::string actStr = r.value("action", "Wander");
              std::string thought = r.value("thought", "");

              Character::Act finalAct = Character::Act::Wander;

              // 简单的字符串映射 (建议后续用 map 优化)
              if (actStr == "Eat")   finalAct = Character::Act::Eat;
              else if (actStr == "Sleep") finalAct = Character::Act::Sleep;
              else if (actStr == "UsePC") finalAct = Character::Act::UseComputer;
              else if (actStr == "Talk")  finalAct = Character::Act::Talk;
              else if (actStr == "Stop")  finalAct = Character::Act::Stop;

              // 返回结果
              return BrainResult{finalAct, thought};
          }
      } catch (const std::exception& e) {
          printf("HTTP Error: %s\n", e.what());
      }
      // 如果 HTTP 失败，回退到本地逻辑
#endif
        return localUtility(chSnap, is_being_called, nowTick, hasFood, hasBed, hasComputer);
    });
}

void Brain::poll(Blackboard& bb, Character& ch)
{
  if (!_fut.valid()) return;
  using namespace std::chrono_literals;
  if (_fut.wait_for(0s) != std::future_status::ready) return;

  BrainResult result = _fut.get();

  //思考写入记忆
  if (!result.thought.empty()) {
    ch.short_memory().add("[Thought] " + result.thought);
  }

  // action推入队列
  auto action = ActionFactory::createFromEnum(result.act);
  if (action) {
    std::lock_guard<std::mutex> lk(bb.queueMutex);
    bb.actionQueue.push_back(std::move(action));
    bb.actNow = result.act; // 更新当前意图状态
  }

  bb.is_thinking = false;
}

bool Brain::isThinking() const
{
    return _fut.valid();
}

// Old Decide action logic
BrainResult Brain::localUtility(const Character& _ch,
                                   const bool is_being_called,
                                   uint64_t tick_index,
                                   bool hasFood, bool hasBed, bool hasComputer)
{
  if (is_being_called) {
    return BrainResult{Character::Act::WaitAlways, ""};
  }

  // stop
  double scoreStop = 0.0; //用于确认目前是否在Stop
  /*if (_bb.in_stop(tick_index)) {
    scoreStop = BASE_STOP;
  }*/

  double scoreTalk = 0.0;
  if (_ch.act() == Character::Act::Talk) {
    scoreTalk = BASE_TALK;
  }

  // use computer
  const double scoreUseComputer = CalcScoreUseComputer(
          _ch.get_boredom(),
          hasComputer,
          _ch.act() == Character::Act::UseComputer,
          BORED_ENTER,
          BORED_EXIT
      );

  // eat
  const double scoreEat = CalcScoreEat(
      _ch.get_hunger_inner(), hasFood,
      !_ch.eatAvailable(),
      _ch.act() == Character::Act::Eat,  // sticky
      HUNGER_ENTER
  );

  // sleep
  const double scoreSleep = CalcScoreSleep(
  _ch.get_fatigue_score(),  hasBed,
  _ch.act() == Character::Act::Sleep,
  TIRED_ENTER, RESTED_EXIT
  );

  // 5. 计算
  Character::Act chosen_action = Character::Act::Wander;
  double best = BASE_WANDER;

  if (scoreEat > best) { best = scoreEat; chosen_action = Character::Act::Eat; }
  if (scoreSleep > best) { best = scoreSleep; chosen_action = Character::Act::Sleep; }
  if (scoreStop > best) { best = scoreStop; chosen_action = Character::Act::Stop; }
  if (scoreUseComputer > best) {best = scoreUseComputer; chosen_action = Character::Act::UseComputer;}
  if (scoreTalk > best) { best = scoreTalk; chosen_action = Character::Act::Talk; }

  return BrainResult{chosen_action, ""};
}
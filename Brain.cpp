//
// Created by jacob on 26-1-19.
//

#include "Brain.h"
#include "tools/Utils.h"
#include <chrono>

Brain::~Brain()
{
    if (_fut.valid()) _fut.wait();
}

void Brain::requestDecision(const Character& chSnap,
                            const Blackboard& bbSnap,
                            uint64_t   nowTick,
                            bool       hasFood,
                            bool       hasBed,
                            bool       hasComputer)
{
    if (_fut.valid()) return;               // 上一次还没取完结果
    _fut = std::async(std::launch::async, [=, &bbSnap]() {

        /* ------------------------ 可插拔分支 ------------------------ */
#ifdef USE_LLM_HTTP_SERVER
#include <nlohmann/json.hpp>
#include <httplib/httplib.h>
        using nlohmann::json;
        json j;
        j["hunger"]   = chSnap.get_hunger_inner();
        j["fatigue"]  = chSnap.get_fatigue_score();
        j["boredom"]  = chSnap.get_boredom();
        j["nowTick"]  = nowTick;

        httplib::Client cli("http://127.0.0.1:8000");
        auto res = cli.Post("/decide", j.dump(), "application/json");
        if (res && res->status == 200) {
            auto r = json::parse(res->body);
            std::string actStr = r.value("action", "Wander");
            if (actStr == "Eat")   return Character::Act::Eat;
            if (actStr == "Sleep") return Character::Act::Sleep;
            if (actStr == "UsePC") return Character::Act::UseComputer;
        }
        /* 若 HTTP 失败 fallthrough 到本地 */
#endif
        return localUtility(chSnap, bbSnap, nowTick, hasFood, hasBed, hasComputer);
    });
}

void Brain::poll(Blackboard& bb)
{
    if (!_fut.valid()) return;
    using namespace std::chrono_literals;
    if (_fut.wait_for(0s) != std::future_status::ready) return;

    Character::Act act = _fut.get();

    auto action = ActionFactory::createFromEnum(act);
    if (action) {
        std::lock_guard<std::mutex> lk(bb.queueMutex);
        bb.actionQueue.push_back(std::move(action));
    }
    bb.is_thinking = false;
}

bool Brain::isThinking() const
{
    return _fut.valid();
}

// Old Decide action logic
Character::Act Brain::localUtility(const Character& _ch,
                                   const Blackboard& _bb,
                                   uint64_t tick_index,
                                   bool hasFood, bool hasBed, bool hasComputer)
{
  if (_bb.is_being_called) {
    return Character::Act::WaitAlways;
  }

  // stop
  double scoreStop = 0.0; //用于确认目前是否在Stop
  if (_bb.in_stop(tick_index)) {
    scoreStop = BASE_STOP;
  }

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
  if (scoreUseComputer > best) {
    best = scoreUseComputer; chosen_action = Character::Act::UseComputer;
    std::cout<<"Using computer"<<std::endl;
  }
  if (scoreTalk > best) { best = scoreTalk; chosen_action = Character::Act::Talk; }

  return chosen_action;
}
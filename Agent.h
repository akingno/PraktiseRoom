//
// Created by jacob on 26-1-12.
//

#ifndef AGENT_H
#define AGENT_H

#include <string>
#include "Character.h"
#include "Blackboard.h"
#include "ActionExecutor.h"
#include "tools/Utils.h"
#include "Config.h"

class Room;
class ItemLayer;
class IPathfinder;

class Agent {
public:
  Agent(std::string& name, int start_x, int start_y, IPathfinder* pf )
         : _name(name), _pf(pf)  {
    _ch.setLoc(start_x, start_y);
    _executor = std::make_unique<ActionExecutor>();

  }
  Agent();

  [[nodiscard]] Character& getCharacter() { return _ch; }
  [[nodiscard]] const std::string& getName() const { return _name; }


  void Update(double dt_sec, uint64_t tick_index, Room& room, ItemLayer& items, std::vector<Agent *>& others) {

    _other_agents = others;

    // 需求更新
    _ch.tickNeeds(dt_sec);

    // 决策
    decideAction(items, tick_index);

    // 构建瞬时的context
    ActExecutorCtx ctx{room, _ch, tick_index, *_pf, items, this};

    // 执行
    _executor->tick(_ch.act(), ctx, _bb);
  }

  void receiveCall(Agent* agent) {
    _bb.caller_agent = agent;
    _bb.is_being_called = true;
  }

  void headMessage(const std::string &msg) {
    _ch.short_memory().add(msg);
  }

  void finishChat() {
    _bb.is_being_called = false;
    _bb.caller_agent = nullptr;
    _ch.setAct(Character::Act::Wander);
  }

  [[nodiscard]] std::vector<Agent *> get_other_agents(){
    return _other_agents;
  }
private:

  /*
   * Calculate scores of actions and set now action
   */
  void decideAction(const ItemLayer& items, uint64_t tick_index) {

    if (_bb.is_being_called) {
      _ch.setAct(Character::Act::WaitAlways);
      return;
    }

    // stop
    double scoreStop = 0.0; //用于确认目前是否在Stop,目前无用
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
            items.hasComputer(),
            _ch.act() == Character::Act::UseComputer,
            BORED_ENTER,
            BORED_EXIT
        );;

    // eat
    const double scoreEat = CalcScoreEat(
        _ch.get_hunger_inner(), items.hasFood(),
        !_ch.eatAvailable(),
        _ch.act() == Character::Act::Eat,  // sticky
        HUNGER_ENTER
    );

    // sleep
    const double scoreSleep = CalcScoreSleep(
    _ch.get_fatigue_score(),  items.hasBed(),
    _ch.act() == Character::Act::Sleep,
    TIRED_ENTER, RESTED_EXIT
    );

    // 5. 计算
    Character::Act chosen_action = Character::Act::Wander;
    double best = BASE_WANDER;

    if (scoreEat > best) { best = scoreEat; chosen_action = Character::Act::Eat; }
    if (scoreSleep > best) { best = scoreSleep; chosen_action = Character::Act::Sleep; }
    if (scoreStop > best) { best = scoreStop; chosen_action = Character::Act::Stop; }
    if (scoreUseComputer > best) { best = scoreUseComputer; chosen_action = Character::Act::UseComputer; }
    if (scoreTalk > best) { best = scoreTalk; chosen_action = Character::Act::Talk; }

    _ch.setAct(chosen_action);
  }


  std::string   _name;
  Character     _ch;
  Blackboard    _bb;
  IPathfinder*   _pf;
  std::unique_ptr<ActionExecutor> _executor;
  std::vector<Agent *> _other_agents;

};




#endif //AGENT_H

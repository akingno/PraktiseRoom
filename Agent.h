//
// Created by jacob on 26-1-12.
//

#ifndef AGENT_H
#define AGENT_H

#include "ActionExecutor.h"
#include "Blackboard.h"
#include "Brain.h"
#include "Character.h"
#include <string>

class Room;
class ItemLayer;
class IPathfinder;

class Agent {
public:
  Agent(std::string& name, int start_x, int start_y, IPathfinder* pf )
         : _name(name), _pf(pf){
    _ch.setLoc(start_x, start_y);
    _executor = std::make_unique<ActionExecutor>();
    _brain = std::make_unique<Brain>();

  }
  Agent();

  [[nodiscard]] Character& getCharacter() { return _ch; }
  [[nodiscard]] const std::string& getName() const { return _name; }


  void update(double dt_sec, uint64_t tick_index, Room& room, ItemLayer& items, std::vector<Agent *>& others) {
    _other_agents = others;
    // 需求更新
    _ch.tickNeeds(dt_sec);

    //取 Brain 结果
    _brain->poll(_bb, _ch);

    // 如果队列空且不在思考 → 发起思考
    bool queueEmpty;
    {
      std::lock_guard<std::mutex> lk(_bb.queueMutex);
      queueEmpty = _bb.actionQueue.empty();
    }

    if (queueEmpty && !_bb.currentAction && !_bb.is_thinking) {
      _bb.is_thinking = true;
      _brain->requestDecision(
        _ch, _bb.is_being_called,_name,
        tick_index,
        items.hasFood(),items.hasBed(),items.hasComputer());
    }

    // 构建瞬时的context
    ActExecutorCtx ctx{room, _ch, tick_index, *_pf, items, this};

    {
      std::lock_guard<std::mutex> lk(_bb.queueMutex);
      _ch.setAct(_bb.actNow);
    }
    // 执行
    _executor->tick(ctx, _bb);
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


  std::string   _name;
  Character     _ch;
  Blackboard    _bb;
  IPathfinder*   _pf;
  std::unique_ptr<ActionExecutor> _executor;
  std::vector<Agent *> _other_agents;
  std::unique_ptr<Brain> _brain;

};




#endif //AGENT_H

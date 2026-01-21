//
// Created by jacob on 26-1-12.
//

#ifndef AGENT_H
#define AGENT_H

#include "ActionExecutor.h"
#include "Blackboard.h"
#include "Character.h"
#include <string>
#include "actions/ActionFactory.h"
class Room;
class ItemLayer;
class IPathfinder;

class Agent {
public:
  Agent(std::string& name, int start_x, int start_y, IPathfinder* pf )
         : _name(name), _pf(pf){
    _ch.setLoc(start_x, start_y);
    _executor = std::make_unique<ActionExecutor>();

  }
  Agent();

  [[nodiscard]] const Character& getCharacter() const { return _ch; }
  [[nodiscard]] Character& getCharacter() { return _ch; }

  [[nodiscard]] const std::string& getName() const { return _name; }

  //是否在被呼叫？
  [[nodiscard]] bool isBeingCalled() const {
    return _bb.is_being_called;
  }

  //是否需要发起新的决策
  [[nodiscard]] bool needsNewDecision() const {
    //如果正在等待 Brain 回复，就不需要新决策
    if (_bb.is_thinking) return false;

    //如果正在被呼叫，也不需要主动决策
    if (_bb.is_being_called) return false;

    //检查队列是否为空且当前无动作
    return _bb.actionQueue.empty() && !_bb.currentAction;
  }

  void markThinking() {
    _bb.is_thinking = true;
  }

  // 应用得到的决策
  void applyDecision(Character::Act act) {
    // 将 Enum 转为 Action 对象
    auto action = ActionFactory::createFromEnum(act);

    if (action) {
      std::lock_guard<std::mutex> lk(_bb.queueMutex);
      _bb.actionQueue.push_back(std::move(action));
      _bb.actNow = act; // 更新意图
    }

    // 2. 思考结束，恢复标记
    _bb.is_thinking = false;
  }


  void update(double dt_sec, uint64_t tick_index, Room& room, ItemLayer& items, std::vector<Agent *>& others) {
    _other_agents = others;
    // 需求更新
    _ch.tickNeeds(dt_sec);

    // 构建瞬时的context
    ActExecutorCtx ctx{room, _ch, tick_index, *_pf, items, this};

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

};




#endif //AGENT_H

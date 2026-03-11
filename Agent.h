//
// Created by jacob on 26-1-12.
//

#ifndef AGENT_H
#define AGENT_H

#include "ActionExecutor.h"
#include "Blackboard.h"
#include "BrainImplement.h"
#include "Character.h"
#include "ItemRegistry.h"
#include "TriggerManager.h"
#include "actions/ActionFactory.h"
#include "event_bus/EventBus.h"
#include "spdlog/spdlog.h"

#include <iostream>
#include <string>

class Room;
class ItemLayer;
class IPathfinder;

class Agent {
 public:
  Agent(std::string name, std::string id, int start_x, int start_y, AIType ai_type = AIType::Utility, std::string texture_name = "character.png")
      : _name(name), _id(std::move(id)), _texture_name(std::move(texture_name)){
    _ch.setLoc(start_x, start_y);
    _executor = std::make_unique<ActionExecutor>();
    _brain = createBrain(ai_type);
  }
  Agent() = default;

  [[nodiscard]] const Character &getCharacter() const { return _ch; }
  [[nodiscard]] Character &getCharacter() { return _ch; }

  [[nodiscard]] const std::string &getName() const { return _name; }
  [[nodiscard]] const std::string &getId() const { return _id; }
  [[nodiscard]] AIType getAIType() const { return _brain->getType(); }
  [[nodiscard]] const std::string &getTextureName() const { return _texture_name; }

  [[nodiscard]] bool isReadyForAction() const {
    return _bb.actionQueue.empty() && !_bb.currentAction;
  }

  //是否在被呼叫？
  [[nodiscard]] bool isBeingCalled() const {
    return _bb.is_being_called;
  }

  //是否需要发起新的决策
  [[nodiscard]] bool needsNewDecision() const {
    //只有utility ai和llm ai要决策
    AIType type = getAIType();
    if (type == AIType::Static || type == AIType::Player) {
      return false;
    }
    //如果正在等待回复，不需要新决策
    if (_bb.is_thinking) return false;

    //如果正在被呼叫，不需要主动决策
    if (_bb.is_being_called) return false;

    //检查队列是否为空且当前无动作
    return isReadyForAction();
  }

  void markThinking() {
    _bb.is_thinking = true;
  }

  // 应用得到的决策
  void applyDecision(Character::Act act, const std::string &targetItemId = "", std::pair<int, int> targetPos = {-1, -1}) {
    if (getAIType() == AIType::Static) {
      return;
    }

    _bb.target_item_id = targetItemId;
    _bb.target = targetPos;
    _bb.target_valid = (targetPos.first != -1);

    std::shared_ptr<Action> action = nullptr;

    if (act == Character::Act::UseItem) {
      if (Item *baseItem = ItemRegistry::inst().get(targetItemId)) {
        if (auto *smartItem = dynamic_cast<SmartItem *>(baseItem)) {
          action = ActionFactory::createFromSmartItem(smartItem);
        }
      }
    }
    // 其他走老管线
    else {
      action = ActionFactory::createFromEnum(act);
    }

    // 3. 推入队列执行
    if (action) {
      std::lock_guard<std::mutex> lk(_bb.queueMutex);
      _bb.actionQueue.push_back(std::move(action));
      _bb.actNow = act;
    }

    _bb.is_thinking = false;
  }

  void update(double dt_sec, uint64_t tick_index, Room &room, ItemLayer &items, std::vector<Agent *> &others) {
    _other_agents = others;
    // 需求更新
    if (getAIType() != AIType::Static) {
      _ch.tickNeeds(dt_sec);
    }

    if (_brain) {
      _brain->think(this, dt_sec, tick_index, room, items, others);
    }

    auto pos = _ch.getLoc();
    int current_level = _ch.getLevel();

    const TriggerDef* trg = TriggerManager::inst().getTriggerAt(current_level, pos.first, pos.second); //踩到trigger了吗

    if (trg) {
      spdlog::debug("Player trigger le");
      // 当这次踩到的trigger和上一次记录的不同时，才发射信号
      if (_last_stepped_trigger != trg->id) {
        _last_stepped_trigger = trg->id;
        EventBus::onTriggerStepped.emit(trg->id, _id);
      }
    } else {
      // 脚下没有触发器，清空记录。
      _last_stepped_trigger = "";
    }


    // 构建瞬时的context
    ActExecutorCtx ctx{room, _ch, tick_index, *room.getPathfinder(), items, this};

    _executor->tick(ctx, _bb);
  }

  void receiveCall(Agent *agent) {
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

  void receiveTrigger(Agent* triggerer) {
    if (_brain) {
      _brain->onTriggerNotified(this, triggerer);
    }
  }

  [[nodiscard]] std::vector<Agent *> get_other_agents() {
    return _other_agents;
  }

  std::string getTargetItemId() const {
    return _bb.target_item_id;
  }

  void clearActionQueue() {
    std::lock_guard<std::mutex> lk(_bb.queueMutex);
    _bb.actionQueue.clear();
    _bb.currentAction = nullptr;
    _bb.is_thinking = false;
  }

  // 锁定一个目标，将整个动作序列推入自己的队列
  void castSequenceOnTarget(Agent* target, std::shared_ptr<Action> sequence) {
    clearActionQueue();
    _bb.target_agent = target; // 黑板锁定施法目标
    std::lock_guard<std::mutex> lk(_bb.queueMutex);
    _bb.actionQueue.push_back(std::move(sequence));
    _bb.actNow = Character::Act::WaitAlways;
  }

  void setStaticAISequence(const std::vector<ActionDescriptor>& seq) {
    if (getAIType() == AIType::Static) {
      if (auto* sb = dynamic_cast<StaticBrain*>(_brain.get())) {
        sb->setSequenceDef(seq);
      }
    }
  }

  std::vector<ActionDescriptor> getStaticAISequence() const {
    if (getAIType() == AIType::Static) {
      if (auto* sb = dynamic_cast<StaticBrain*>(_brain.get())) {
        return sb->getSequenceDef();
      }
    }
    return {};
  }


 private:
  std::string _name;
  std::string _id;
  Character _ch;
  std::unique_ptr<IBrain> _brain;
  Blackboard _bb;
  std::unique_ptr<ActionExecutor> _executor;
  std::vector<Agent *> _other_agents;
  std::string _texture_name;

  std::string _last_stepped_trigger = ""; //用于防抖
};

#endif//AGENT_H

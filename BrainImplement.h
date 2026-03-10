//
// Created by jacob on 26-3-3.
//

#ifndef BRAINIMPLEMENT_H
#define BRAINIMPLEMENT_H
#include "IBrain.h"
#include "actions/ActionDef.h"
#include <memory>

// weak ai triggered by triggers, no need, no complext action sequence, no decision
class StaticBrain : public IBrain {
 public:
  void think(Agent *body, double dt_sec, uint64_t tick_index, Room &room, ItemLayer &items, const std::vector<Agent *> &others) override;
  AIType getType() const override { return AIType::Static; }
  void onTriggerNotified(Agent *body, Agent *triggerer) override;

  void setSequenceDef(const std::vector<ActionDescriptor> &seq) { sequence_def_ = seq; }
  const std::vector<ActionDescriptor> &getSequenceDef() const { return sequence_def_; }

 private:
  std::vector<ActionDescriptor> sequence_def_;
};

// utility 计算
class UtilityBrain : public IBrain {
 public:
  void think(Agent *body, double dt_sec, uint64_t tick_index, Room &room, ItemLayer &items, const std::vector<Agent *> &others) override;
  AIType getType() const override { return AIType::Utility; }
};

// llm 控制
class LLMBrain : public IBrain {
 public:
  void think(Agent *body, double dt_sec, uint64_t tick_index, Room &room, ItemLayer &items, const std::vector<Agent *> &others) override;
  AIType getType() const override { return AIType::LLM; }
};

//player 控制大脑
class PlayerBrain : public IBrain {
 public:
  void think(Agent *body, double dt_sec, uint64_t tick_index, Room &room, ItemLayer &items, const std::vector<Agent *> &others) override;
  AIType getType() const override { return AIType::Player; }
};

std::unique_ptr<IBrain> createBrain(AIType type);
#endif//BRAINIMPLEMENT_H

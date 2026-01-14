//
// Created by jacob on 26-1-14.
//

#ifndef SELECTAGENTACTION_H
#define SELECTAGENTACTION_H

#include "../Agent.h"
#include "../tools/Random.h"
#include "Action.h"

class SelectAgentAction : public Action {
public:
  Status tick(ActExecutorCtx& ctx, Blackboard& bb) override {
    std::vector<Agent*> candidates;
    for (Agent* a : ctx.parent_agent->get_other_agents()) {
      if (a != ctx.parent_agent) {
        candidates.push_back(a);
      }
    }

    if (candidates.empty()) {
      return Status::Failure;
    }

    int idx = Random::randint(0, static_cast<int>(candidates.size()) - 1);
    bb.target_agent = candidates[idx];

    return Status::Success;
  }
};

#endif //SELECTAGENTACTION_H

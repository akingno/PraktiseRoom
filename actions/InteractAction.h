//
// Created by jacob on 26-1-2.
//

#ifndef INTERACTACTION_H
#define INTERACTACTION_H
#include "Action.h"

class InteractAction : public Action {


public:
  Status tick(ActExecutorCtx& ctx, Blackboard& bb) override;


};

#endif //INTERACTACTION_H

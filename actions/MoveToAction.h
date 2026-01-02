//
// Created by jacob on 26-1-2.
//

#ifndef MOVETOACTION_H
#define MOVETOACTION_H
// MoveToAction.cpp (或者写在头文件里)

#include "Action.h"




struct ActExecutorCtx;

class MoveToAction : public Action {
    TargetKind _targetKind;

public:
    explicit MoveToAction(TargetKind kind) : _targetKind(kind) {}

  void onEnter(ActExecutorCtx& ctx, Blackboard& bb) override;
  Status tick(ActExecutorCtx& ctx, Blackboard& bb) override;


};

#endif //MOVETOACTION_H

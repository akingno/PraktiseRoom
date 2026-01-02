//
// Created by jacob on 26-1-2.
//

#ifndef SEQUENCEACTION_H
#define SEQUENCEACTION_H
#include "Action.h"
#include <memory>

class SequenceAction : public Action {

  // 需要执行的动作们
  std::vector<std::shared_ptr<Action>> _actions;
  size_t _currentIdx = 0;

public:
  void add(std::shared_ptr<Action> action) {
    _actions.push_back(action);
  }

  void onEnter(ActExecutorCtx& ctx, Blackboard& bb) override {
    _currentIdx = 0;
    if (!_actions.empty()) {
      _actions[0]->onEnter(ctx, bb);
    }
  }

  Status tick(ActExecutorCtx& ctx, Blackboard& bb) override {

    if (_currentIdx >= _actions.size()) {
      return Status::Success;
    }

    Action* curr = _actions[_currentIdx].get();

    Status s = curr->tick(ctx, bb);

    if (s == Status::Success) {
      curr->onExit(ctx, bb); // 结束当前子动作
      _currentIdx++;         // 进下一个

      if (_currentIdx < _actions.size()) {
        _actions[_currentIdx]->onEnter(ctx, bb); // 启动下一个
        return Status::Running;
      }
      else {
        return Status::Success; // 整个序列完成
      }
    }
    else if (s == Status::Failure) {
      return Status::Failure; // 任何一步失败，整体失败
    }
    return Status::Running;
  }

  void onExit(ActExecutorCtx& ctx, Blackboard& bb) override {
    if (_currentIdx < _actions.size()) {
      _actions[_currentIdx]->onExit(ctx, bb);
    }
  }

};

#endif //SEQUENCEACTION_H

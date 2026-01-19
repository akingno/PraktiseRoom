//
// Created by jacob on 26-1-19.
//

#ifndef ACTIONFACTORY_H
#define ACTIONFACTORY_H



#include <memory>
#include "../Character.h"
#include "Action.h"

class ActionFactory {
public:
  static std::shared_ptr<Action> createFromEnum(Character::Act act);

  // 预留：static std::shared_ptr<Action> createFromJson(const nlohmann::json& j);
};



#endif //ACTIONFACTORY_H

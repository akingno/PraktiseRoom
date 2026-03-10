//
// Created by jacob on 26-1-19.
//

#ifndef ACTIONFACTORY_H
#define ACTIONFACTORY_H



#include <memory>
#include "../Character.h"
#include "Action.h"
#include "../SmartItem.h"

class ActionFactory {
public:
  static std::shared_ptr<Action> createFromEnum(Character::Act act);

  static std::shared_ptr<Action> createFromSmartItem(const SmartItem* item);

  static std::shared_ptr<Action> createFromDescriptors(const std::vector<ActionDescriptor> &seqDef);
};



#endif //ACTIONFACTORY_H

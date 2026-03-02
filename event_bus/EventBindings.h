//
// Created by jacob on 26-3-1.
//

#ifndef EVENTBINDINGS_H
#define EVENTBINDINGS_H

#include <vector>
#include <memory>
#include "../Room.h"
#include "../ItemLayer.h"
#include "../Agent.h"
#include "../renders/IRender.h"
#include "../tools/AStarPathfinder.h"

namespace SystemBindings {
void bindAllUIEvents(Room& room, ItemLayer& items, std::vector<std::unique_ptr<Agent>>& agents, IPathfinder* pf, IRender* render);
}



#endif //EVENTBINDINGS_H

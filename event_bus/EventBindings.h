//
// Created by jacob on 26-3-1.
//

#ifndef EVENTBINDINGS_H
#define EVENTBINDINGS_H

#include "../Agent.h"
#include "../ItemLayer.h"
#include "../Room.h"
#include "../renders/IRender.h"
#include <memory>
#include <vector>

namespace SystemBindings {
void bindAllUIEvents(std::vector<std::unique_ptr<Agent>> &agents, IRender *render);
}

#endif//EVENTBINDINGS_H

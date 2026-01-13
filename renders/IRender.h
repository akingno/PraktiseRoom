//
// Created by jacob on 25-9-24.
//

#ifndef IRENDER_H
#define IRENDER_H

#include "../Character.h"
#include "../Item.h"
#include "../Room.h"
#include <memory>

class Agent;

class IRender {
  public:
  virtual void render_frame(const ItemLayer&,const std::vector<std::unique_ptr<Agent>>&, const Room&) = 0;
  virtual ~IRender() = default;
  virtual bool poll_quit() = 0;

};



#endif //IRENDER_H

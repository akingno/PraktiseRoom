//
// Created by jacob on 25-9-24.
//

#ifndef IRENDER_H
#define IRENDER_H


#include "Character.h"
#include "Room.h"

struct RenderStats {
  double scoreEat;
  double scoreWander;
  double scoreSleep;
};

class IRender {
  public:
  virtual void render_frame(const Character&character, const Room& room, const RenderStats& stats) = 0;
  virtual ~IRender() = default;
  virtual bool poll_quit() = 0;

};



#endif //IRENDER_H

//
// Created by jacob on 25-9-24.
//

#ifndef IRENDER_H
#define IRENDER_H

#include "ASCIIRender.h"
#include "Character.h"
#include "Room.h"

class IRender {
  public:
  void render_frame_ascii(const Character&character, const Room& room, double scoreEat, double scoreWander);
  IRender() : ascii_renderer(VIEW_H * (VIEW_W + 1)) {

  }
 private:
  ASCIIRender ascii_renderer;
};



#endif //IRENDER_H

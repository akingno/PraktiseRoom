//
// Created by jacob on 25-9-24.
//

#include "IRender.h"

#include "ASCIIRender.h"

void IRender::render_frame_ascii(const Character&character, const Room& room, double scoreEat, double scoreWander) {

  ascii_renderer.render_room(room);

  ascii_renderer.render_items(room);

  ascii_renderer.render_character(character);

  ascii_renderer.render_info(room, character, scoreEat, scoreWander);

}

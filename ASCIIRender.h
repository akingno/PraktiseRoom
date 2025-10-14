//
// Created by jacob on 25-9-24.
//

#ifndef ASCIIRENDER_H
#define ASCIIRENDER_H

#include "Character.h"
#include "IRender.h"
#include "Room.h"

#include <cstdint>
#include <iostream>
#include <windows.h>



class ASCIIRender : public IRender {

  static void enableAnsi() {
    //用于windwos终端使用
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING; // 关键
    SetConsoleMode(hOut, mode);
  }
  char& buf_at(int sx, int sy){
    return frame[ sy * (VIEW_W + 1) + sx ];
  }
  /**
   *
   * Output information at the last line
   *
   * */
  static void print_status_line(int row, const std::string& s) {
    std::cout << "\x1b[" << row << ";1H"   // 光标定位到 row 行 1 列
              << "\x1b[2K"                 // 清除整行
              << s                         // 输出内容
              << std::flush;
  }

public:
  explicit ASCIIRender(int size_frame) {
    frame.resize(size_frame);
  }

  static void initialTerminal() {
    enableAnsi();

    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
  }
  void render_room(const Room &room);
  void render_items(const Room &room);
  void render_character(const Character &character);
  void render_info(const Room & room, const Character & character, const RenderStats & stats);

  void render_frame_ascii(const Character&character, const Room& room, const RenderStats& stats) override;


 private:
  std::string frame;
  bool first = true; // 是否第一次渲染了

};



#endif //ASCIIRENDER_H

//
// Created by jacob on 26-3-9.
//

#ifndef ACTIONDEF_H
#define ACTIONDEF_H
#include<string>

//用于描述原子动作
struct ActionDescriptor {
  std::string name; // 动作名，如"MoveTo"
  std::string strParam; // 字符串参数，如属性名"hunger"
  int intParam = 0; // 参数1，如传送的X坐标
  int intParam2 = 0; // 参数2，如传送的Y坐标
  float floatParam = 0.0f;// 参数3，如增加的饥饿度
};

#endif //ACTIONDEF_H

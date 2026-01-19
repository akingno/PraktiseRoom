//
// Created by jacob on 25-9-24.
//

#ifndef DIRECTIONS_H
#define DIRECTIONS_H
#include "../Config.h"
#include "../tools/Random.h"

#include <string>

/**
 *
 *
 *
 *
 * Warning: 本类 因为 随机乱走 删除 而 废弃
 * 可能会因为 后续的图像的脸朝向方向而重新启用
 *
 *
 *
 *
 *
 *
 */
enum class Dir{
  Right = 0,
  Left = 1,
  Down = 2,
  Up = 3,
  Stay = 4
};

struct DirWeights{
  const int right = 1, left = 1, down = 1, up = 1, stay = 1;
private:
  [[nodiscard]] int sum() const{
    return right + left + down + up + stay;
  }
public:
  // 随机选择一个方向
  [[nodiscard]] Dir random_dir_weights() const{
    double r = AkRandom::rand01() * sum();
    if((r -= right) < 0 ) return Dir::Right;
    if((r -= left) < 0 ) return Dir::Left;
    if((r -= down) < 0 ) return Dir::Down;
    if((r -= stay) < 0) return Dir::Stay;
    return Dir::Stay;
  }
};

/**
 *
 * @param d 方向
 * @return pair<int, int> 把方向转为坐标表示, 如(1,0)表示向右
 */
static std::pair<int, int> dir_vec(Dir d){
  switch (d) {
    case Dir::Right: return {+1, 0};
    case Dir::Left: return  {-1, 0};
    case Dir::Down: return  {0,+1};
    case Dir::Up:   return  {0, -1};
    case Dir::Stay:   return  {0, 0};
  }
  return {0, 0};
}

/**
 *
 * @param d
 * @return the opposite direction of d
 */
static Dir opposite(Dir d){
  switch (d) {
    case Dir::Right:return  Dir::Left;
    case Dir::Left: return  Dir::Right;
    case Dir::Down: return  Dir::Up;
    case Dir::Up:   return  Dir::Down;
    case Dir::Stay: return  Dir::Stay;
  }
  return d;
}

/**
 *
 * @param num the index of direction: Right:0, Left:1, Down:2, Up:3, Stay:4
 * @return
 */
static Dir Int2Dir(int num) {
  switch (num) {
    case 0:
      return Dir::Right;
    case 1:
      return Dir::Left;
    case 2:
      return Dir::Down;
    case 3:
      return Dir::Up;
    case 4:
      return Dir::Stay;
    default:
      return Dir::Stay;
  }
}

/**
 *
 * @param dir
 * @return the string of dir, e.g. "Right"
 */
static std::string Dir2Str(Dir dir) {
  switch (dir) {
    case Dir::Right:
      return "Right";
    case Dir::Left:
      return "Left";
    case Dir::Down:
      return "Down";
    case Dir::Up:
      return "Up";
    case Dir::Stay:
      return "Stay";
    default:
      return "Unknown";
  }
}
#endif //DIRECTIONS_H

//
// Created by jacob on 25-10-24.
//

#ifndef ITEMLAYER_H
#define ITEMLAYER_H

#include "Config.h"
#include "Item.h"
#include <optional>
#include <unordered_map>
class Room;

class ItemLayer {

 public:
  explicit ItemLayer(int w = Cfg::room::view_w, int h = Cfg::room::view_h, int level_id = 0)
      : width_(w), height_(h), level_id_(level_id) {}

  bool place(const ItemId &id, int x, int y);// 放置/覆盖
  bool removeAt(int x, int y); // 移除
  std::optional<ItemId> idAt(int x, int y) const;


  void initDefaultLayout();
  void clear();
  void saveToFile(const std::string& filename = "world.json") const;
  void loadFromFile(const std::string& filename = "world.json");
  int getLevelId() const { return level_id_; }
  int getWidth() const { return width_; }
  int getHeight() const { return height_; }

  // 给渲染器遍历
  const std::unordered_map<int, ItemId> &items() const { return items_; }

 private:
  int key(int x, int y) const { return y * width_ + x; }
  std::unordered_map<int, ItemId> items_;// key= y*W+x
  int width_;
  int height_;
  int level_id_;
};

#endif//ITEMLAYER_H

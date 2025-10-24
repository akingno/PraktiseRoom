//
// Created by jacob on 25-10-24.
//

#ifndef ITEMREGISTRY_H
#define ITEMREGISTRY_H



#include "Item.h"
#include <memory>

class ItemRegistry {
public:
  static ItemRegistry& inst() { static ItemRegistry R; return R; }

  void register_item(std::unique_ptr<Item> item) {
    items_[item->id()] = std::move(item);
  }

  Item* get(const ItemId& id) const {
    auto it = items_.find(id);
    return it==items_.end() ? nullptr : it->second.get();
  }

private:
  std::unordered_map<ItemId, std::unique_ptr<Item>> items_;
};



#endif //ITEMREGISTRY_H

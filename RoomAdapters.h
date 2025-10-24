//
// Created by jacob on 25-10-24.
//

#ifndef ROOMADAPTERS_H
#define ROOMADAPTERS_H

// RoomAdapters.h
#pragma once
#include "Block.h"
#include "Item.h"
#include <optional>

//TODO: 这个是temp的， 之后要把room和item分离
inline std::optional<ItemId> tile_to_item(TileType t) {
  switch (t) {
    case TileType::FOOD: return ItemId("food");
    case TileType::BED:  return ItemId("bed");
    default: return std::nullopt;
  }
}

#endif //ROOMADAPTERS_H

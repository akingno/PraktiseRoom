//
// Created by jacob on 26-2-28.
//

#ifndef TERRAINREGISTRY_H
#define TERRAINREGISTRY_H

#include <string>
#include <unordered_map>
#include <memory>

using TileId = std::string;

struct TerrainDef {
  TileId id;
  std::string texture_name;
  bool blocks;
};

class TerrainRegistry {
public:
  static TerrainRegistry& inst() { static TerrainRegistry R; return R; }

  void register_terrain(const TerrainDef& def) {
    terrains_[def.id] = def;
  }

  const TerrainDef* get(const TileId& id) const {
    auto it = terrains_.find(id);
    return it == terrains_.end() ? nullptr : &it->second;
  }

  const std::unordered_map<TileId, TerrainDef>& getAllTerrains() const {
    return terrains_;
  }

  void clear() { terrains_.clear(); }

private:
  std::unordered_map<TileId, TerrainDef> terrains_;
};

#endif //TERRAINREGISTRY_H

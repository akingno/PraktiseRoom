//
// Created by jacob on 2025/9/10.
//

#ifndef WORLD_TEMP__BLOCK_H_
#define WORLD_TEMP__BLOCK_H_
#include <cstdint>
#include <utility>

enum class TileType{
  Grass = 0,
  WallV = 1,
  WallH = 2,
};

class Block {

 public:
  explicit Block(TileType tileType){
      _tile_type = tileType;
  };

  static inline char tile_glyph(TileType t) {
    switch (t) {
      case TileType::WallV:  return '|';
      case TileType::WallH: return '-';
      case TileType::Grass:
      default:             return ' ';
    }
  }

  static inline TileType Int2Tile(int num){
    switch (num) {
      case 0:
        return TileType::Grass;
      case 1:
        return TileType::WallV;
      case 2:
        return TileType::WallH;
      default:
        return TileType::Grass;
    }
  }

  inline TileType getTileType(){
    return _tile_type;
  }
  inline void setTileType(TileType tile_type){
    _tile_type = tile_type;
  }

  Block() : _tile_type(TileType::Grass){}
 private:
  TileType _tile_type;

};

#endif//WORLD_TEMP__BLOCK_H_

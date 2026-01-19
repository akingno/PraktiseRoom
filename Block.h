//
// Created by jacob on 2025/9/10.
//

#ifndef WORLD_TEMP__BLOCK_H_
#define WORLD_TEMP__BLOCK_H_

enum class TileType{
  Grass = 0,
  WallV = 1,
  WallH = 2,
  FOOD = 3,
  DOOR = 4,
  BED = 5,
  COMPUTER = 6,
};

class Block {

 public:
  explicit Block(TileType tileType){
      _tile_type = tileType;
  }

/*
 * Used by ascii render and not used now
 *
 */
  /*static char tile_glyph(const TileType t) {
    switch (t) {
      case TileType::WallV:  return '|';
      case TileType::WallH: return '-';
      case TileType::FOOD: return 'f';
      case TileType::DOOR: return 'D';
      case TileType::BED: return 'B';
      case TileType::Grass:
      default:             return ' ';
    }
  }*/


  TileType getTileType() const {
    return _tile_type;
  }
  void setTileType(TileType tile_type){
    _tile_type = tile_type;
  }

  Block() : _tile_type(TileType::Grass){}
 private:
  TileType _tile_type;

};

#endif//WORLD_TEMP__BLOCK_H_

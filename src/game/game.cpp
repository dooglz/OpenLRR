
#include <iostream>
#include <array>
#include <FastNoise.h>
#include <cmath>
#include <optional>
#include "game.h"

void Game::StartUp() {


  //create level
  const size_t squareSize = 4;
  const size_t levelSize = 10;


  struct Tile{
    enum TileType{
      empty,
      solid,
      rock,
      water,
      TileTypeCount
    };
    enum RockTypes{
      hard,lose,dirt,vein,RockTypesCount
    };

    TileType type;
    uint8_t height;
    std::optional<RockTypes> rockType;
    Tile():type{empty},height{1}{};
    char tostr(){
      switch (type){
        case empty:
        return ' ';
        case solid:
          return '#';
        case water:
          return '-';
        case rock:
          return ((char)rockType.value())+48;
        default:
          return 'Z';
      }
    }
  };

  std::array<Tile,levelSize*levelSize> tiles;
  FastNoise noise,noise2;
  noise.SetFrequency(0.1);
  for (int i = 0; i < levelSize; ++i) {
    for (int j = 0; j < levelSize; ++j) {

       Tile & t =  tiles[j+(i*levelSize)];
       auto hh = noise.GetPerlin(i,j);
     t.height =  (uint8_t) round(10.f*noise.GetPerlin(i,j));
     if(t.height == 0 ){
       t.type = Tile::water;
     }else {
       uint8_t rval = (uint8_t)round(((float)Tile::RockTypesCount+1.0f)*noise2.GetCellular(i, j));
        if(rval == 0 ){
          t.type = Tile::solid;
        }else if(rval>Tile::RockTypesCount){
          t.type = Tile::empty;
        }else{
          t.type = Tile::rock;
          t.rockType = Tile::dirt;
        }
     }
      std::cout << t.tostr();

    }
    std::cout << std::endl;

  }
  int g = 4;

}

void Game::Tick(double dt) {

}

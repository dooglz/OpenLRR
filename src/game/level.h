//
// Created by Sam Serrels on 07/12/2019.
//
#pragma once

#include <array>
#include <cmath>
#include <glm/glm.hpp>
//#include <iostream>
#include <optional>

#include "idx.h"

namespace Game {
const size_t squareSize = 4;
const size_t levelSize = 16;
const float wallheight = 3.0f;
const size_t nTiles = levelSize * levelSize;
const size_t nVerts = nTiles + (2 * levelSize) + 1;
// const size_t nVertsDim = sqrt(nVerts);
const size_t indiceCount = nTiles * 2 * 3;
const bool FLATLEVEL = true;

struct Tile {
  enum TileType { empty, rock, water, TileTypeCount };
  enum RockTypes { solid, hard, lose, dirt, vein, RockTypesCount };
  TileType type;
  uint8_t height;
  std::optional<RockTypes> rockType;
  bool isSpawn = false;
  bool inverted = false;
  size_t rockmask = 0;
  std::array<uint16_t, 6> tileIndices;

  Tile() : type{empty}, height{1} {};

  char tostr() {
    if (isSpawn) {
      return 'S';
    }
    switch (type) {
    case empty:
      return ' ';
    case water:
      return '-';
    case rock:
      return ((char)rockType.value()) + 48;
    default:
      return 'Z';
    }
  }
};

class Level {
public:
  Level();
  ~Level();
  void Render();
  std::array<Tile, levelSize * levelSize> _tiles;
  std::array<glm::vec3, nVerts> _verts;
  std::array<uint16_t, indiceCount> _inidces;
  //
  idx _spawnpoint;

private:
  // LevelGen.cpp
  bool validateMap(std::array<Tile, levelSize * levelSize>& tiles, idx& SpawnPoint);
  void PrintMap(std::array<Tile, levelSize * levelSize> tiles, bool masks = false);
  void Triangulate(std::array<Tile, levelSize * levelSize>& tiles, std::array<glm::vec3, nVerts>& verts, std::array<uint16_t, indiceCount>& inidces);
  void SquashWalls(std::array<Tile, levelSize * levelSize>& tiles);
  std::array<Tile, levelSize * levelSize> Generate();
};
} // namespace Game
//
// Created by Sam Serrels on 07/12/2019.
//
#pragma once
#pragma warning(disable : 26812)
#include <array>
#include <cmath>
#include <glm/glm.hpp>
//#include <iostream>
#include <map>
#include <optional>

#include "game_graphics.h"
#include "idx.h"

namespace Game {
const size_t squareSize = 4;
const size_t levelSize = 16;
const float wallheight = 3.0f;
const size_t nTiles = levelSize * levelSize;
const size_t nVerts = nTiles + (2 * levelSize) + 1;
// const size_t nVertsDim = sqrt(nVerts);
const size_t indiceCount = nTiles * 2 * 3;
const bool FLATLEVEL = false;

struct Tile {
  enum TileType { empty, rock, water, TileTypeCount };
  enum RockTypes { solid, hard, lose, dirt, vein, RockTypesCount };
  const static std::map<TileType, bool> isFlat;
  const static std::map<TileType, std::vector<TileType>> matchesHeight;
  TileType type;
  uint8_t height;
  std::optional<RockTypes> rockType;
  bool isSpawn = false;
  bool inverted = false;
  size_t rockmask = 0;
  Tile() : type{empty}, height{1} {};
  constexpr static glm::vec3 voidColour = glm::vec3(0.196f, 0.224f, 0.184f);
  glm::vec3 GetColor() {
    if (isSpawn) {
      return glm::vec3(0, 1, 0);
    }
    switch (type) {
    case empty:
      return glm::vec3(0.553f, 0.6f, 0.682f);
    case water:
      return glm::vec3(0.259f, 0.522f, 0.976f);
    case rock:
      switch (rockType.value()) {
      case dirt:
        return glm::vec3(0.839f, 0.812f, 0.796f);
      case lose:
        return glm::vec3(0.651f, 0.502f, 0.549f);
      case hard:
        return glm::vec3(0.439f, 0.40f, 0.467f);
      case solid:
        return glm::vec3(0.337f, 0.322f, 0.392f);
      case vein:
        return glm::vec3(0.773f, 0.847f, 0.427f);
      default:
        return glm::vec3(1, 1, 1);
      }
    default:
      return glm::vec3(1, 1, 1);
    }
  }
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
  std::vector<Vertex> _verts;
  std::vector<uint16_t> _inidces;
  //
  idx _spawnpoint;

private:
  // LevelGen.cpp
  bool validateMap(std::array<Tile, levelSize * levelSize>& tiles, idx& SpawnPoint);
  void PrintMap(std::array<Tile, levelSize * levelSize> tiles, bool masks = false);
  void Triangulate(std::array<Tile, levelSize * levelSize>& tiles, std::array<glm::vec3, nVerts>& verts, std::array<uint16_t, indiceCount>& inidces);
  void SquashWalls(std::array<Tile, levelSize * levelSize>& tiles);
  std::array<Tile, levelSize * levelSize> Generate();
  void Triangulate2(std::vector<Vertex>& allVerts, std::vector<uint16_t>& allIndices);
};
} // namespace Game

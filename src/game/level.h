//
// Created by Sam Serrels on 07/12/2019.
//
#pragma once

#include <array>
#include <iostream>
#include <optional>
#include <cmath>
#include <glm/glm.hpp>

namespace Game {
const size_t squareSize = 4;
const size_t levelSize = 4;
const size_t nTiles = levelSize * levelSize;
const size_t nVerts = nTiles + (2 * levelSize) + 1;
const size_t nVertsDim = sqrt(nVerts);
const size_t indiceCount = nTiles * 2 * 3;

struct Tile {
  enum TileType { empty, solid, rock, water, TileTypeCount };
  enum RockTypes { hard, lose, dirt, vein, RockTypesCount };
  TileType type;
  uint8_t height;
  std::optional<RockTypes> rockType;
  bool isSpawn = false;
  Tile() : type{empty}, height{1} {};
  char tostr() {
    if (isSpawn) {
      return 'S';
    }
    switch (type) {
    case empty:
      return ' ';
    case solid:
      return '#';
    case water:
      return '-';
    case rock:
      return ((char)rockType.value()) + 48;
    default:
      return 'Z';
    }
  }
};

struct idx {
  size_t x, y;
  size_t dist(const idx& a) { return sqrt(((double)a.y - (double)y) + ((double)a.x - (double)x)); }
  // bool adjacent(const idx& a) const { return false; }
  bool surround(const idx& a) const { return (a.x == x - 1 || a.x == x || a.x == x + 1) && (a.y == y - 1 || a.y == y || a.y == y + 1); }
  friend std::ostream& operator<<(std::ostream& os, const idx& dt) {
    os << "[" << dt.x << '/' << dt.y << ']';
    return os;
  }
  idx() : x{0}, y{0} {};
  idx(size_t a, size_t b) : x{a}, y{b} {};
  idx(int a, int b) : x{(size_t)a}, y{(size_t)b} {};
  // Copy assignment operator.
  /*idx& operator=(const idx& other) {
    if (this != &other) {
      x = other.x;
      y = other.y;
    }
    return *this;
  }*/
};

class Level {
public:
  Level();
  ~Level();
  std::array<Tile, levelSize * levelSize> _tiles;
  std::array<glm::vec3, nVerts> _verts;
  std::array<uint16_t, indiceCount> _inidces;

private:

};



} // namespace Game

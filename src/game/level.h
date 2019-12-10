//
// Created by Sam Serrels on 07/12/2019.
//

#include <array>
#include <optional>
#include <cmath>
#pragma once

namespace Game {
const size_t squareSize = 4;
const size_t levelSize = 16;

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
};

std::array<Tile, levelSize * levelSize> GenerateLevel();

} // namespace Game

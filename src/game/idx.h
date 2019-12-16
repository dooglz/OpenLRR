//
// Created by Sam Serrels on 16/12/2019.
//

#ifndef OPENLRR_IDX_H
#define OPENLRR_IDX_H
#include <cmath>
#include <iostream>
#include <vector>

namespace Game {
struct idx {
  size_t x, y;

  double dist(const idx& a);
  bool surround(const idx& a) const;
  bool adjacent(const idx& a) const;

  static std::vector<Game::idx> getAdjTiles(size_t a, size_t b, size_t s);
  static std::vector<Game::idx> getAdjTiles(const idx& a, size_t s);
  std::vector<Game::idx> getAdjTiles(size_t s) const;
  static std::vector<Game::idx> getSurTiles(size_t a, size_t b, size_t s);
  static std::vector<Game::idx> getSurTiles(const idx& a, size_t s);
  std::vector<Game::idx> getSurTiles(size_t s) const;

  friend std::ostream& operator<<(std::ostream& os, const idx& dt) {
    os << "[" << dt.x << '/' << dt.y << ']';
    return os;
  }

  idx() : x{0}, y{0} {};

  idx(size_t a, size_t b) : x{a}, y{b} {};

  idx(int a, int b) : x{(size_t)a}, y{(size_t)b} {};

  bool operator==(const idx& a) { return a.x == x && a.y == y; }

  friend bool operator==(const idx& a, const idx& b) { return a.x == b.x && a.y == b.y; }

  enum OrBit {
    l = 1,
    d = 2,
    r = 4,
    u = 8,
    dr = 16,
    dl = 32,
    ur = 64,
    ul = 128,
  };

  static size_t orientation(const idx& me, const idx& other);
  size_t orientation(const idx& other) const;
};

#define dimAt(a, b, s) b + (a * s)
#define dimPos(a, s)                                                                                                                                 \
  a == 0 ? idx{0, 0} : idx { (size_t) floor(a / s), a - (((size_t)floor(a / s)) * s) }

#define TileAt(v,a, b) v[dimAt(a, b, levelSize)];
#define TilePos(a) dimPos(a, levelSize);

#define getAsosiatedVerts(a, b)                                                                                                                      \
  {                                                                                                                                                  \
    {a, b}, {a, b + 1}, {a + 1, b}, { a + 1, b + 1 }                                                                                                 \
  }

#define VertAt(v,a, b) v[dimAt(a, b, (levelSize + 1))]
} // namespace Game
#endif // OPENLRR_IDX_H
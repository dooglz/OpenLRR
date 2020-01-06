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
  long long x, y;

  double dist(const idx& a);
  bool surround(const idx& a) const;
  bool adjacent(const idx& a) const;

  static std::vector<Game::idx> getAdjTiles(long long a, long long b, size_t s);
  static std::vector<Game::idx> getAdjTiles(const idx& a, size_t s);
  std::vector<Game::idx> getAdjTiles(size_t s) const;
  static std::vector<Game::idx> getSurTiles(long long a, long long b, size_t s);
  static std::vector<Game::idx> getSurTiles(const idx& a, size_t s);
  std::vector<Game::idx> getSurTiles(size_t s) const;
  static std::vector<Game::idx> getTouchingTilesForVert(const idx& p, const unsigned char& vert);
  std::vector<Game::idx> getTouchingTilesForVert(const unsigned char& vert) const;

  friend std::ostream& operator<<(std::ostream& os, const idx& dt) {
    os << "[" << dt.x << '/' << dt.y << ']';
    return os;
  }

  idx() : x{0}, y{0} {};

  idx(long long a, long long b) : x{a}, y{b} {};
  idx(size_t a, size_t b) {
    if (a > std::numeric_limits<long long>::max() || b > std::numeric_limits<long long>::max()) {
      throw std::overflow_error("Too big");
    }
    x = static_cast<long long>(a);
    y = static_cast<long long>(b);
  };

  idx(int a, int b) : x{(long long)a}, y{(long long)b} {};

  bool operator==(const idx& a) { return a.x == x && a.y == y; }
  bool operator!=(const idx& a) { return a.x != x || a.y != y; }
  idx& operator+=(const idx& rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
    return *this; // return the result by reference
  }
  friend idx operator+(idx lhs, const idx& rhs) {
    lhs += rhs;
    return lhs; // return the result by value (uses move constructor)
  }
  idx& operator-=(const idx& rhs) {
    this->x -= rhs.x;
    this->y -= rhs.y;
    return *this; // return the result by reference
  }
  friend idx operator-(idx lhs, const idx& rhs) {
    lhs -= rhs;
    return lhs; // return the result by value (uses move constructor)
  }

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
  static const idx U, UL, UR, L, R, D, DL, DR;
};

#define dimAt(a, b, s) a + (b * s)
#define dimPos(a, s)                                                                                                                                 \
  a == 0 ? idx{0, 0} : idx { a - (((size_t)floor(a / s)) * s), (size_t)floor(a / s) }

#define TileAt(v, a, b, s) v[dimAt(a, b, s)]
#define TilePos(a, s) dimPos(a, s)

#define getAsosiatedVerts(a, b)                                                                                                                      \
  {                                                                                                                                                  \
    {a, b}, {a + 1, b}, {a, b + 1}, { a + 1, b + 1 }                                                                                                 \
  }

#define VertAt(v, a, b) v[dimAt(a, b, (levelSize + 1))]
} // namespace Game
#endif // OPENLRR_IDX_H

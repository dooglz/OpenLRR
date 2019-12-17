//
// Created by Sam Serrels on 16/12/2019.
//

#include "idx.h"

namespace Game {

double idx::dist(const idx& a) { return sqrt(((double)a.y - (double)y) + ((double)a.x - (double)x)); }
bool idx::surround(const idx& a) const { return (a.x == x - 1 || a.x == x || a.x == x + 1) && (a.y == y - 1 || a.y == y || a.y == y + 1); }
bool idx::adjacent(const idx& a) const { return (a.x == x && (a.y == y - 1 || a.y == y + 1)) || (a.y == y && (a.x == x - 1 || a.x == x + 1)); }

std::vector<Game::idx> idx::getAdjTiles(size_t a, size_t b, size_t s) {
  s = s - 1;
  std::vector<Game::idx> v;
  if (a < s) {
    v.emplace(v.end(), a+1, b);
  }
  if (a > 0) {
    v.emplace(v.end(), a - 1, b);
  }
  if (b > 0) {
    v.emplace(v.end(), a, b - 1);
  }
  if (b < s) {
    v.emplace(v.end(), a, b + 1);
  }
  return v;
};
std::vector<Game::idx> idx::getAdjTiles(const idx& a, size_t s) { return idx::getAdjTiles(a.x, a.y, s); }
std::vector<Game::idx> idx::getAdjTiles(size_t s) const { return idx::getAdjTiles(*this, s); }

std::vector<Game::idx> idx::getSurTiles(size_t a, size_t b, size_t s) {
  std::vector<idx> adj = getAdjTiles(a, b, s);
  if (a > 0 && b > 0) {
    adj.push_back({a - 1, b - 1});
  }
  if (a < s - 1 && b < s - 1) {
    adj.push_back({a + 1, b + 1});
  }
  if (a > 0 && b < s - 1) {
    adj.push_back({a - 1, b + 1});
  }
  if (b > 0 && a < s - 1) {
    adj.push_back({a + 1, b - 1});
  }
  return adj;
};
std::vector<Game::idx> idx::getSurTiles(const idx& a, size_t s) { return idx::getSurTiles(a.x, a.y, s); }
std::vector<Game::idx> idx::getSurTiles(size_t s) const { return idx::getSurTiles(*this, s); }

size_t idx::orientation(const idx& me, const idx& other) {
  auto difX = (float)other.x - (float)me.x;
  auto difY = (float)other.y - (float)me.y;

  if (abs(difX) > abs(difY)) {
    return difX > 0 ? OrBit::d : OrBit::u;
  } else if (abs(difX) < abs(difY)) {
    return difY > 0 ? OrBit::r : OrBit::l;
  } else {
    return difX > 0 ? (difY > 0 ? OrBit::dr : OrBit::dl) : (difY > 0 ? OrBit::ur : OrBit::ul);
  }
}
size_t idx::orientation(const idx& other) const { return idx::orientation(*this, other); }

} // namespace Game

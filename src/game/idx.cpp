//
// Created by Sam Serrels on 16/12/2019.
//

#include "idx.h"

namespace Game {

double idx::dist(const idx& a) { return sqrt(((double)a.y - (double)y) + ((double)a.x - (double)x)); }
bool idx::surround(const idx& a) const { return (a.x == x - 1 || a.x == x || a.x == x + 1) && (a.y == y - 1 || a.y == y || a.y == y + 1); }
bool idx::adjacent(const idx& a) const { return (a.x == x && (a.y == y - 1 || a.y == y + 1)) || (a.y == y && (a.x == x - 1 || a.x == x + 1)); }

const idx idx::U = {0, -1};
const idx idx::UL = {-1, -1};
const idx idx::UR = {1, -1};
const idx idx::L = {-1, 0};
const idx idx::R = {1, 0};
const idx idx::D = {0, 1};
const idx idx::DL = {-1, 1};
const idx idx::DR = {1, 1};

std::vector<Game::idx> idx::getTouchingTilesForVert(const idx& p, const unsigned char& vert) {
  ;
  switch (vert) {
  case 0:
    return {{p + idx::UL}, {p + idx::U}, {p + idx::L}};
  case 1:
    return {{p + idx::U}, {p + idx::UR}, {p + idx::R}};
  case 2:
    return {{p + idx::L}, {p + idx::DL}, {p + idx::D}};
  case 3:
    return {{p + idx::R}, {p + idx::D}, {p + idx::DR}};
  }
}
std::vector<Game::idx> idx::getTouchingTilesForVert(const unsigned char& vert) const { return idx::getTouchingTilesForVert(*this, vert); }

std::vector<Game::idx> idx::getAdjTiles(long long a, long long b, size_t s) {
  s = s - 1;
  std::vector<Game::idx> v;
  if (a < s) {
    v.emplace(v.end(), a + 1, b);
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
// note this isn't used for jsut tiles, TODO change name
std::vector<Game::idx> idx::getSurTiles(long long a, long long b, size_t s) {
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
    return difX > 0 ? OrBit::r : OrBit::l;
  } else if (abs(difX) < abs(difY)) {
    return difY > 0 ? OrBit::d : OrBit::u;
  } else {
    return difX > 0 ? (difY > 0 ? OrBit::dr : OrBit::ur) : (difY > 0 ? OrBit::dl : OrBit::ul);
  }
}
size_t idx::orientation(const idx& other) const { return idx::orientation(*this, other); }

} // namespace Game

#include "game.h"
#include "game_graphics.h"
#include "level.h"
#include <memory>
#include <vector>
#include <algorithm>

std::unique_ptr<Game::Level> level;

void Game::StartUp() { level = std::make_unique<Game::Level>(); }

void Game::Tick(double dt) {}

#define RED                                                                                                                                          \
  { 1, 0, 0 }
#define GREEN                                                                                                                                        \
  { 0, 1, 0 }
#define BLUE                                                                                                                                         \
  { 0, 0, 1 }
#define PINK                                                                                                                                         \
  { 1, 0, 1 }
#define YONG                                                                                                                                         \
  { 1, 1, 0 }

std::vector<Game::Vertex> Avertices = {
    // center Square
    {{-0.5f, -0.5f, 0.f}, RED, {0, 0, 1}},
    {{0.5f, -0.5f, 0.f}, RED, {0, 0, 1}},
    {{0.5f, 0.5f, 0.f}, RED, {0, 0, 1}},
    {{-0.5f, 0.5f, 0.f}, RED, {0, 0, 1}},
    // X point,
    {{1.0f, 0.f, 0.f}, GREEN, {0, 0, 1}},
    // ypoint,
    {{0.0f, 1.f, 0.f}, PINK, {0, 0, 1}},
    {{0.25f, 0.5f, 0.f}, RED, {0, 0, 1}},
    {{-0.25f, 0.5f, 0.f}, RED, {0, 0, 1}},
    // zpoint
    {{0, 0, 1.f}, YONG, {0, 0, 1}},
    {{-0.25f, 0, 0}, RED, {0, 0, 1}},
    {{0.25f, 0, 0}, RED, {0, 0, 1}}};

std::vector<uint16_t> Aindices = {
    // center Square
    0, 1, 2, 2, 3, 0,
    //
    4, 2, 1,
    //
    5, 6, 7,
    //
    8, 9, 10};

std::vector<Game::Vertex> aa;
Game::Vertex* Game::getVertices(size_t& count) {
  // count = level->_verts.size();
  // return &level->_verts[0];
  aa = std::vector<Vertex>(level->_verts.begin(), level->_verts.end());
  aa.insert(aa.end(), Avertices.begin(), Avertices.end());
  count = aa.size();
  return aa.data();
}
std::vector<uint16_t> ai;

glm::uint16_t* Game::getIndices(size_t& count) {

  ai = std::vector<uint16_t>(level->_inidces.begin(), level->_inidces.end());
  const auto cnt = level->_verts.size();
  std::transform(Aindices.begin(), Aindices.end(), std::back_inserter(ai), [cnt](auto& c) { return c + cnt; });
  count = ai.size();
  return ai.data();
}

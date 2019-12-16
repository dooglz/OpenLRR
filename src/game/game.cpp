#include "game.h"
#include "game_graphics.h"
#include "level.h"
#include <memory>
#include <vector>

std::unique_ptr<Game::Level> level;

void Game::StartUp() { level = std::make_unique<Game::Level>(); }

void Game::Tick(double dt) {}

std::vector<glm::vec3> Avertices = {
    // center Square
    {-0.5f, -0.5f, 0.f},
    {0.5f, -0.5f, 0.f},
    {0.5f, 0.5f, 0.f},
    {-0.5f, 0.5f, 0.f},
    // X point
    {1.0f, 0.f, 0.f},
    // ypoint
    {0.0f, 1.f, 0.f},
    {0.25f, 0.5f, 0.f},
    {-0.25f, 0.5f, 0.f},
    // zpoint
    {0, 0, 1.f},
    {-0.25f, 0, 0},
    {0.25f, 0, 0},
};

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

  aa = std::vector<Vertex>(level->_verts.begin(), level->_verts.end());
  aa.push_back({{level->_spawnpoint.x - 0.5, level->_spawnpoint.y, 0},{0,1,1}});
  aa.push_back({{level->_spawnpoint.x, level->_spawnpoint.y, 3},{1,0,1}});
  aa.push_back({{level->_spawnpoint.x + 0.5, level->_spawnpoint.y, 0},{1,1,0}});
  count = aa.size();
  return aa.data();

  count = level->_verts.size();
  return &level->_verts[0];
  // count = Avertices.size();
  // return Avertices.data();
}
std::vector<uint16_t> ai;

glm::uint16_t* Game::getIndices(size_t& count) {

  ai = std::vector<uint16_t>(level->_inidces.begin(), level->_inidces.end());
  const auto c = level->_verts.size();
  ai.push_back(c);
  ai.push_back(c + 1);
  ai.push_back(c + 2);
  count = ai.size();
  return ai.data();

  count = level->_inidces.size();
  // count = 1*6;
  return level->_inidces.data();
  // count = Aindices.size();
  // return Aindices.data();
}

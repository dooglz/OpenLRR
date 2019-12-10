#include "game.h"
#include "level.h"
#include <memory>
#include <vector>

std::unique_ptr<Game::Level> level;

void Game::StartUp() { level = std::make_unique<Game::Level>(); }

void Game::Tick(double dt) {}

std::vector<glm::vec3> Avertices = {
        //center Square
        {-0.5f, -0.5f, 0.f}, {0.5f, -0.5f, 0.f}, {0.5f, 0.5f, 0.f}, {-0.5f, 0.5f, 0.f},
        //X point
        {1.0f, 0.f, 0.f},
        //ypoint
        {0.0f, 1.f, 0.f}, {0.25f, 0.5f, 0.f}, {-0.25f, 0.5f, 0.f},
        //zpoint
        {0, 0, 1.f}, {-0.25f, 0, 0}, {0.25f, 0, 0},
};

std::vector<uint16_t> Aindices = {
        //center Square
        0, 1, 2, 2, 3, 0,
        //
        4, 2, 1,
        //
        5, 6, 7,
        //
        8, 9, 10
};

glm::vec3 *Game::getVertices(size_t &count) {
  //count = level->_verts.size();
  // return level->_verts.data();
  count = Avertices.size();
  return Avertices.data();
}

glm::uint16_t *Game::getIndices(size_t &count) {
  //count = level->_inidces.size();
  //count = 12;
  //return level->_inidces.data();
  count = Aindices.size();
  return Aindices.data();
}

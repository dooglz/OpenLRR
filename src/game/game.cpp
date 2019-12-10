#include "game.h"
#include "level.h"
#include <memory>
#include <vector>

std::unique_ptr<Game::Level> level;

void Game::StartUp() { level = std::make_unique<Game::Level>(); }

void Game::Tick(double dt) {}

 std::vector<glm::vec3> Avertices = {{-0.5f, -0.5f, 0.f}, {0.5f, -0.5f, 0.f}, {0.5f, 0.5f, 0.f}, {-0.5f, 0.5f, 0.f}};
 std::vector<uint16_t> Aindices = {0, 1, 2, 2, 3, 0};

glm::vec3* Game::getVertices(size_t& count) {
  count = level->_verts.size();
  return level->_verts.data();
  count = Avertices.size();
  return Avertices.data();
}

glm::uint16_t* Game::getIndices(size_t& count) {
  count = level->_inidces.size();
  return level->_inidces.data();
  count = Aindices.size();
  return Aindices.data();
}

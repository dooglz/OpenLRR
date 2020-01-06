#include "game.h"
#include "../engine/graphics/graphics_backend.h"
#include "../engine/graphics/vk/vulkan.h"
#include "game_graphics.h"
#include "level.h"
#include <algorithm>
#include <memory>
#include <vector>

std::unique_ptr<Game::Level> level;
std::unique_ptr<RenderableItem> levelRI;

void Game::StartUp() {

  level = std::make_unique<Game::Level>();
  levelRI =
      std::make_unique<vkRenderableItem>(&level->_verts[0], level->_verts.size(), &level->_inidces[0], level->_inidces.size(), RenderableItem::lit);
  levelRI->setUniformModelMatrix(glm::mat4(1.0f));
}

void Game::Tick(double dt) {}

Game::Vertex* Game::getVertices(size_t& count) {
  count = level->_verts.size();
  return &level->_verts[0];
}

glm::uint16_t* Game::getIndices(size_t& count) {
  count = level->_inidces.size();
  return &level->_inidces[0];
}
void Game::Shutdown() {
  levelRI.reset();
  level.reset();
}

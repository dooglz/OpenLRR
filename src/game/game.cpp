#include "game.h"
#include "../engine/graphics/graphics_backend.h"
#include "../engine/graphics/vk/vulkan.h"
#include "game_graphics.h"
#include "geometry.h"
#include "level.h"
#include <algorithm>
#include <memory>
#include <vector>
#include <glm/gtx/transform.hpp>

std::unique_ptr<Game::Level> level;
std::unique_ptr<RenderableItem> levelRI;
std::unique_ptr<RenderableItem> lampRI;

void Game::StartUp() {

  level = std::make_unique<Game::Level>();
  levelRI = std::make_unique<vkRenderableItem>(&level->_verts[0], static_cast<uint32_t>(level->_verts.size()), &level->_inidces[0],
                                               static_cast<uint32_t>(level->_inidces.size()), RenderableItem::lit);
  levelRI->setUniformModelMatrix(glm::mat4(1.0f));

  simpleGeo box = debugCube();
   lampRI = std::make_unique<vkRenderableItem>(box.v.data(), static_cast<uint32_t>(box.v.size()), box.i.data(),
   static_cast<uint32_t>(box.i.size()),RenderableItem::lit);
   lampRI->setUniformModelMatrix(glm::mat4(1.0f));
}

void Game::Tick(double dt) {
  static double lifetime = 0;
  lifetime += dt;

 
    lampRI->setUniformModelMatrix(glm::rotate(glm::mat4(1.0f), ((float)lifetime), glm::vec3(0.0f, 0.0f, 1.0f)));

}

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
   lampRI.reset();
  level.reset();
}

#include "game.h"
#include "../engine/Engine.h"
#include "../engine/graphics/graphics_backend.h"
#include "../engine/graphics/vk/vulkan.h"
#include "game_graphics.h"
#include "geometry.h"
#include "level.h"
#include <algorithm>
#include <glm/gtx/transform.hpp>
#include <memory>
#include <vector>

std::unique_ptr<Game::Level> level;
std::unique_ptr<RenderableItem> levelRI;
std::unique_ptr<RenderableItem> lampRI;

void Game::StartUp() {

  level = std::make_unique<Game::Level>();
  levelRI = std::make_unique<vkRenderableItem>(&level->_verts[0], static_cast<uint32_t>(level->_verts.size()), &level->_inidces[0],
                                               static_cast<uint32_t>(level->_inidces.size()), RenderableItem::lit);
  levelRI->setUniformModelMatrix(glm::mat4(1.0f));

  simpleGeo box = debugCube();
  lampRI = std::make_unique<vkRenderableItem>(box.v.data(), static_cast<uint32_t>(box.v.size()), box.i.data(), static_cast<uint32_t>(box.i.size()),
                                              RenderableItem::lit);

  lampRI->setUniformModelMatrix(glm::mat4(1.0f));
}

void Game::Tick(double dt) {
  static double lifetime = 0;
  lifetime += dt;
  glm::dmat4 t = glm::translate(glm::dmat4(1.0), Engine::getLightPos());
  glm::dmat4 r = glm::rotate(glm::dmat4(1.0), (lifetime), glm::dvec3(0.0, 0.0, 1.0));
  glm::dmat4 s = glm::scale(glm::dmat4(1.0), glm::dvec3(0.1));
  lampRI->setUniformModelMatrix(glm::mat4(t * r * s));
  levelRI->setUniformModelMatrix(glm::mat4(1.0f));
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

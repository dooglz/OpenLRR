//
// Created by Sam Serrels on 30/11/2019.
//

#ifndef OPENLRR_GRAPHICS_BACKEND_H
#define OPENLRR_GRAPHICS_BACKEND_H

#include "../../game/game_graphics.h"

struct RenderableItem {
  enum PIPELINE { lit, unlit, PIPELINE_COUNT };
  RenderableItem(Game::Vertex* vertices, size_t vcount, glm::uint16_t* indices, size_t icount, PIPELINE p)
      : _pipeline{p}, _vcount{vcount}, _icount{icount} {}
  virtual void updateData(Game::Vertex* vertices, size_t vcount, glm::uint16_t* indices, size_t icount) = 0;
  virtual ~RenderableItem(){};
  const PIPELINE _pipeline;
  size_t _vcount;
  size_t _icount;
  virtual void setUniformModelMatrix(glm::mat4 m) = 0;
  virtual void updateUniform() = 0;
};

class GraphicsBackend {
public:
  virtual void startup() = 0;
  virtual void shutdown() = 0;
  virtual void drawFrame(double dt = 0) = 0;
  virtual void resize() = 0;
};

#endif // OPENLRR_GRAPHICS_BACKEND_H

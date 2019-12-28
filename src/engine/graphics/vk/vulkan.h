//
// Created by Sam Serrels on 30/11/2019.
//

#ifndef OPENLRR_VULKAN_H
#define OPENLRR_VULKAN_H

#include "../graphics_backend.h"

// todo: refactor this out:
#include "vulkan_internals.h"

#include <memory>

class VulkanBackend : public GraphicsBackend {
public:
  void startup() override;
  void shutdown() override;
  void drawFrame(double dt = 0) override;
  void resize() override;
};

struct vkRenderableItem : public RenderableItem {
  vkRenderableItem(Game::Vertex* vertices, size_t vcount, glm::uint16_t* indices, size_t icount, PIPELINE p);
  void updateData(Game::Vertex* vertices, size_t vcount, glm::uint16_t* indices, size_t icount) override;
  ~vkRenderableItem() override;
  void setUniformModelMatrix(glm::mat4 m) override;
  void updateUniform() override;
  //

  std::unique_ptr<VertexBuffer> _vbuffer;
  UniformBufferObject _uniformData;
};
#endif // OPENLRR_VULKAN_H

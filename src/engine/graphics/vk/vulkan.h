//
// Created by Sam Serrels on 30/11/2019.
//

#ifndef OPENLRR_VULKAN_H
#define OPENLRR_VULKAN_H

#include "../graphics_backend.h"

class VulkanBackend : public GraphicsBackend {
public:
  void startup() override;
  void shutdown() override;
  void drawFrame() override;
  void resize() override;
};

#endif // OPENLRR_VULKAN_H

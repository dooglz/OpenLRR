//
// Created by Sam Serrels on 30/11/2019.
//

#include "Engine.h"
#include "graphics/vk/vulkan.h"
#include "platform/platform_glfw.h"

VulkanBackend vk;

void Engine::Startup() {}

void Engine::CreateWindow(int w, int h) { platform::init(w, h); }

void Engine::Go() {
  bool go = true;
  vk.startup();
  while (go) {
    go &= !platform::shouldQuit();
    platform::tick();
    vk.drawFrame();
  }
}

void Engine::Shutdown() { vk.shutdown(); }

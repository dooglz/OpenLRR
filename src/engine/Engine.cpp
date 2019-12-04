//
// Created by Sam Serrels on 30/11/2019.
//

#include "Engine.h"
#include "graphics/vk/vulkan.h"
#include "platform/platform_glfw.h"
#include <exception>
#include <iostream>

VulkanBackend vk;

void Engine::Startup() {}

void Engine::CreateWindow(int w, int h) { platform::init(w, h); }

void Engine::Go() {
  bool go = true;
  vk.startup();
  try {
    while (go) {
      go &= !platform::shouldQuit();
      platform::tick();
      vk.drawFrame();
    }
  } catch (const std::exception& e) {
    std::cerr << "FAIL" << e.what() << std::endl;
  }

}

void Engine::Shutdown() { vk.shutdown(); }

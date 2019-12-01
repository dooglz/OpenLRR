//
// Created by Sam Serrels on 30/11/2019.
//

#include "Engine.h"
#include "platform/platform_glfw.h"
#include "graphics/vulkan.h"

VulkanBackend vk;

void Engine::Startup() {
  vk.probe();
}

void Engine::CreateWindow(int w, int h) {
 platform::init(w,h);
}

void Engine::Go() {
 bool go = true;
 while(go){
  go &= !platform::shouldQuit();
  platform::tick();
 }
}

void Engine::Shutdown() {

}

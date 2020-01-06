//
// Created by Sam Serrels on 30/11/2019.
//

#include "Engine.h"
#include "../game/game.h"
#include "../utils.h"
#include "graphics/vk/vulkan.h"
#include "platform/platform_glfw.h"
#include <chrono>
#include <exception>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <thread>

VulkanBackend vk;
const double TARGET_DT = 1.0 / 30.0;
size_t delay = 0;
void baa(double a) {}



void Engine::Startup() {}

void Engine::OpenWindow(int w, int h) { platform::init(w, h); }

void Engine::Go() {
  bool go = true;
  vk.startup();
  Game::StartUp();
  auto lastframe = std::chrono::high_resolution_clock::now();
  try {
    movingAverage frametimes(
        60, [](double v, double min, double max) { platform::setWindowTitle(std::to_string(1.0 / v) + " " + std::to_string(delay)); });
    while (go) {
      const auto now = std::chrono::high_resolution_clock::now();
      double dt = std::chrono::duration<double, std::chrono::seconds::period>(now - lastframe).count();
      lastframe = now;
      go &= !platform::shouldQuit();
      platform::tick(dt);
      Game::Tick(dt);
      vk.drawFrame(dt);
      frametimes.add(dt);
    }
  } catch (const std::exception& e) {
    std::cerr << "FAIL" << e.what() << std::endl;
  }
}

void Engine::Shutdown() { vk.shutdown(); }

glm::dvec3 camPos = glm::dvec3(4.0, 8.0, 5.0);
// glm::dquat camRot = glm::dquat(0.9,0.37,0,0);
glm::dquat camRot = glm::quat_cast(glm::lookAt(camPos, glm::dvec3(4, 2, 0), glm::dvec3(0, 0, -1.0)));

glm::dvec3 Engine::getCamPos() { return camPos; }

void Engine::setCamPos(const glm::dvec3& p) {
  camPos = p;
//  std::cout << p.x << " " << p.y << " " << p.z << std::endl;
}
glm::dquat Engine::getCamRot() { return camRot; }

void Engine::setCamRot(const glm::dquat& p) {
 // auto gg = normalize(GetForwardVector(p));
  //std::cout << "R: " << gg.x << " " << gg.y << " " << gg.z << std::endl;
  camRot = p;
}

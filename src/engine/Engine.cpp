//
// Created by Sam Serrels on 30/11/2019.
//

#include "Engine.h"
#include "../game/game.h"
#include "../utils.h"
#include "graphics/vk/vulkan.h"
#include "platform/platform_glfw.h"
#include <algorithm>
#include <chrono>
#include <exception>
#include <functional>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <thread>

VulkanBackend vkb;
const double TARGET_DT = 1.0 / 30.0;
size_t delay = 0;
void baa(double a) {}

struct movingAverage {
  double val;
  movingAverage(size_t r, std::function<void(double, double, double)> cb = nullptr) : reset{r}, cb{cb} { _reset(); }
  double add(double in) {
    if (count >= reset) {
      if (cb != nullptr) {
        cb(val, min, max);
      }
      _reset();
    }
    min = std::min(min, in);
    max = std::max(max, in);
    double scaling = 1. / (double)(count + 1);
    val = in * scaling + val * (1. - scaling);
    count++;
    return val;
  };

private:
  void _reset() {
    val = 0;
    count = 0;
    min = std::numeric_limits<double>::max();
    max = 0;
  }
  size_t count;
  size_t reset;
  double min, max;
  std::function<void(double, double, double)> cb;
};

void Engine::Startup() {}

void Engine::OpenWindow(int w, int h) { platform::init(w, h); }

void Engine::Go() {
  bool go = true;
  vkb.startup();
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
      vkb.drawFrame(dt);
      frametimes.add(dt);
    }
  } catch (const std::exception& e) {
    std::cerr << "FAIL" << e.what() << std::endl;
  }
}

void Engine::Shutdown() { vkb.shutdown(); }

glm::dvec3 camPos = glm::dvec3(4.0, 8.0, 5.0);
glm::dvec3 lightPos = glm::dvec3(4.0, 8.0, 5.0);
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


glm::dvec3 Engine::getLightPos() { return lightPos; }
void Engine::setLightPos(const glm::dvec3& p) {lightPos = p;}

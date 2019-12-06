//
// Created by Sam Serrels on 30/11/2019.
//

#include "Engine.h"
#include "graphics/vk/vulkan.h"
#include "platform/platform_glfw.h"
#include <algorithm>
#include <chrono>
#include <exception>
#include <functional>
#include <iostream>

VulkanBackend vk;

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

void Engine::CreateWindow(int w, int h) { platform::init(w, h); }

void Engine::Go() {
  bool go = true;
  vk.startup();
  auto lastframe = std::chrono::high_resolution_clock::now();
  try {
    movingAverage frametimes(10000, [](double v, double min, double max) {
      std::cout << "Fps:\t" << (1.0 / v) << "\t" << (1.0 / min) << "\t" << (1.0 / max) << std::endl;
    });
    while (go) {
      const auto now = std::chrono::high_resolution_clock::now();
      double dt = std::chrono::duration<double, std::chrono::seconds::period>(now - lastframe).count();
      lastframe = now;
      go &= !platform::shouldQuit();
      platform::tick();
      vk.drawFrame(dt);
      frametimes.add(dt);
    }
  } catch (const std::exception& e) {
    std::cerr << "FAIL" << e.what() << std::endl;
  }
}

void Engine::Shutdown() { vk.shutdown(); }

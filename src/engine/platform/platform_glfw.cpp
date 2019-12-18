//
// Created by Sam Serrels on 30/11/2019.
//

#include "platform_glfw.h"
#include "../../utils.h"
#include "../Engine.h"
#include "../graphics/vk/vulkan_internals.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <csignal>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <vulkan/vulkan.hpp>

static GLFWwindow* window;

void platform::init(int w, int h) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(w, h, "Vulkan", nullptr, nullptr);
  // nice positon 3915 -121
  // glfwSetWindowPos(window, 3915, -121);
  // glfwSetWindowPosCallback(window, [](GLFWwindow* wn, int xpos, int ypos) { std::cout << "WindowMoved," << xpos << " \t" << ypos << std::endl; });
  glfwSetWindowSizeCallback(window, [](GLFWwindow* wn, int w, int h) { std::cout << "WindowResized," << w << " \t" << h << std::endl; });
}

void platform::shutdown() {
  glfwDestroyWindow(window);

  glfwTerminate();
}

void processInput(GLFWwindow* window, double dt) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  glm::dvec3 v = glm::vec3(0, 0, 0);
  glm::dvec3 r = glm::dvec3(0, 0, 0);
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    v.x--;
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    v.x++;
  }
  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    v.y++;
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    v.y--;
  }
  if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    v.z--;
  }
  if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
    v.z++;
  }
  if (length(v)) {
    Engine::setCamPos(Engine::getCamPos() + (v * dt));
  }

  const double speed = 15.f * dt;
  const double hRotSpeed = 0.05f * speed;
  const double vRotSpeed = 0.035f * speed;
  // I doubt any of these are right but it works good.
  const glm::dvec3 right = normalize(GetRightVector(Engine::getCamRot()));
  const glm::dvec3 forward = normalize(GetForwardVector(Engine::getCamRot()));
  const glm::dvec3 up = normalize(GetUpVector(Engine::getCamRot()));
  {
    const glm::dvec3 UP = glm::dvec3(0, 0, 1.0);
    const glm::dvec3 FW = glm::dvec3(1.0, 0, 0);
    const glm::dvec3 RF = glm::dvec3(0, 1.0, 0);
    // pitch
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
      Engine::setCamRot(glm::normalize(glm::angleAxis(vRotSpeed, FW) * Engine::getCamRot()));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
      Engine::setCamRot(glm::normalize(glm::angleAxis(-vRotSpeed, FW) * Engine::getCamRot()));
    }
    // yaw
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
      Engine::setCamRot(glm::normalize(angleAxis(hRotSpeed, forward) * Engine::getCamRot()));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
      Engine::setCamRot(glm::normalize(angleAxis(-hRotSpeed, forward) * Engine::getCamRot()));
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
     // raise(SIGTRAP);
      //__builtin_trap();
    }
  }
}

void platform::tick(double dt) {
  glfwPollEvents();
  processInput(window, dt);
}

bool platform::shouldQuit() { return glfwWindowShouldClose(window); }

const char** platform::GetRequiredVKInstanceExtensions(unsigned int* count) { return glfwGetRequiredInstanceExtensions(count); }

void platform::GetFramebufferSize(int* width, int* height) { glfwGetFramebufferSize(window, width, height); }

void platform::setWindowTitle(const std::string& str) {
  // glfwSetWindowTitle(window,str);
  glfwSetWindowTitle(window, str.c_str());
}

VkSurfaceKHR CreateVKWindowSurface(const vk::Instance& instance) {
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
  return surface;
}

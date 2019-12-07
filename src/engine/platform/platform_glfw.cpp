//
// Created by Sam Serrels on 30/11/2019.
//

#include "platform_glfw.h"
#include "../graphics/vk/vulkan_internals.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vulkan/vulkan.hpp>

static GLFWwindow* window;

void platform::init(int w, int h) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window = glfwCreateWindow(w, h, "Vulkan", nullptr, nullptr);
  // nice positon 3915 -121
  //glfwSetWindowPos(window, 3915, -121);
  // glfwSetWindowPosCallback(window, [](GLFWwindow* wn, int xpos, int ypos) { std::cout << "WindowMoved," << xpos << " \t" << ypos << std::endl; });
  glfwSetWindowSizeCallback(window, [](GLFWwindow* wn, int w, int h) { std::cout << "WindowResized," << w << " \t" << h << std::endl; });
}

void platform::shutdown() {
  glfwDestroyWindow(window);

  glfwTerminate();
}

void processInput(GLFWwindow* window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

void platform::tick() {
  // while (!glfwWindowShouldClose(window)) {
  glfwPollEvents();
  processInput(window);

  //}
}

bool platform::shouldQuit() { return glfwWindowShouldClose(window); }

const char** platform::GetRequiredVKInstanceExtensions(unsigned int* count) { return glfwGetRequiredInstanceExtensions(count); }

void platform::GetFramebufferSize(int* width, int* height) { glfwGetFramebufferSize(window, width, height); }

VkSurfaceKHR CreateVKWindowSurface(const vk::Instance& instance) {
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
  return surface;
}

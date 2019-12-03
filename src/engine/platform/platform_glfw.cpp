//
// Created by Sam Serrels on 30/11/2019.
//

#include "platform_glfw.h"
#include "../graphics/vk/vulkan_internals.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

static GLFWwindow* window;

void platform::init(int w, int h) {
  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(w, h, "Vulkan", nullptr, nullptr);
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

VkSurfaceKHR CreateVKWindowSurface(const vk::Instance& instance) {
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }
  return surface;
}

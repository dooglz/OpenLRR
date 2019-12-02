//
// Created by Sam Serrels on 30/11/2019.
//

#include "platform_glfw.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

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

void platform::tick() {
  //while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  //}
}

bool platform::shouldQuit() {
  return glfwWindowShouldClose(window);
}

const char** platform::GetRequiredVKInstanceExtensions(unsigned int *count)
{
	
	return glfwGetRequiredInstanceExtensions(count);
}

bool platform::CreateVKWindowSurface(void* Vkinstance,void* vkSurfaceKHR)
{
	return glfwCreateWindowSurface(*((VkInstance*)Vkinstance), window, nullptr, (VkSurfaceKHR*)vkSurfaceKHR) == VK_SUCCESS;

}


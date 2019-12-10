//
// Created by Sam Serrels on 30/11/2019.
//

#ifndef OPENLRR_PLATFORM_GLFW_H
#define OPENLRR_PLATFORM_GLFW_H

#include <string>

namespace platform {
void init(int w, int h);

void shutdown();

void tick(double dt);

bool shouldQuit();

const char** GetRequiredVKInstanceExtensions(unsigned int* count);
void GetFramebufferSize(int* width, int* height);
void setWindowTitle(const std::string& str);
// bool CreateVKWindowSurface(void* Vkinstance, void* vkSurfaceKHR);

} // namespace platform

#endif // OPENLRR_PLATFORM_GLFW_H

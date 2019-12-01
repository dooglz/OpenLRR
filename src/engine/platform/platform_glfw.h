//
// Created by Sam Serrels on 30/11/2019.
//

#ifndef OPENLRR_PLATFORM_GLFW_H
#define OPENLRR_PLATFORM_GLFW_H
namespace platform {
 void init(int w, int h);

 void shutdown();

 void tick();

 bool shouldQuit();
}

#endif //OPENLRR_PLATFORM_GLFW_H
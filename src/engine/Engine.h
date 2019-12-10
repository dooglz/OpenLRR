//
// Created by Sam Serrels on 30/11/2019.
//

#ifndef OPENLRR_ENGINE_H
#define OPENLRR_ENGINE_H

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine {
void Startup();
void CreateWindow(int width, int height);
void Go();
void Shutdown();

glm::dvec3 getCamPos();
void setCamPos(const glm::dvec3& p);

glm::dquat getCamRot();
void setCamRot(const glm::dquat &p);

} // namespace Engine

#endif // OPENLRR_ENGINE_H

//
// Created by Sam Serrels on 30/11/2019.
//

#ifndef OPENLRR_ENGINE_H
#define OPENLRR_ENGINE_H

#include <glm/ext/quaternion_double.hpp>
#include <glm/ext/vector_double3.hpp>

namespace Engine {
void Startup();
void OpenWindow(int width, int height);
void Go();
void Shutdown();

glm::dvec3 getCamPos();
void setCamPos(const glm::dvec3& p);

glm::dvec3 getLightPos();
void setLightPos(const glm::dvec3& p);

glm::dquat getCamRot();
void setCamRot(const glm::dquat& p);

glm::dmat4 getViewMatrix();
glm::dmat4 getProjectionMatrix();

} // namespace Engine

#endif // OPENLRR_ENGINE_H

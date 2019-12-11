#ifndef OPENLRR_UTILS_H
#define OPENLRR_UTILS_H
#include <glm/ext/vector_double3.hpp>
#include <glm/ext/quaternion_double.hpp>

static glm::dvec3 GetUpVector(const glm::dquat &q) {
  return glm::dvec3(2 * (q.x * q.y - q.w * q.z), 1.0f - 2.0f * (q.x * q.x + q.z * q.z), 2.0f * (q.y * q.z + q.w * q.x));
}

static glm::dvec3 GetForwardVector(const glm::dquat &q) {
  return glm::dvec3(2.0f * (q.x * q.z + q.w * q.y), 2.0f * (q.y * q.z - q.w * q.x), 1.0f - 2.0f * (q.x * q.x + q.y * q.y));
}

static glm::dvec3 GetRightVector(const glm::dquat &q) {
  return glm::dvec3(1.0f - 2.0f * (q.y * q.y + q.z * q.z), 2.0f * (q.x * q.y + q.w * q.z), 2.0f * (q.x * q.z - q.w * q.y));
}

#endif //OPENLRR_UTILS_H

#ifndef OPENLRR_UTILS_H
#define OPENLRR_UTILS_H
#define NOMINMAX
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_double.hpp>
#include <glm/ext/vector_double3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <functional>
#include <algorithm>
// Z = UP
// X = L->R
// Y = 0->intoScreen


#define RED                                                                                                                                          \
  { 1, 0, 0 }
#define GREEN                                                                                                                                        \
  { 0, 1, 0 }
#define BLUE                                                                                                                                         \
  { 0, 0, 1 }
#define PINK                                                                                                                                         \
  { 1, 0, 1 }
#define YONG                                                                                                                                         \
  { 1, 1, 0 }

static glm::dvec3 GetUpVector(const glm::dquat &q) {
 // glm::dvec4 UP(0,0,1.0,0);
//  glm::dquat camQuatConj = glm::conjugate(q);
//  aa = glm::mat4_cast(q) * UP * glm::mat4_cast(camQuatConj);
  //glm::dvec3 camUp = glm::dvec3( glm::quat(q) * UP * camQuatConj );
//  glm::rotate()
  return glm::dvec3(2 * (q.x * q.y - q.w * q.z), 1.0f - 2.0f * (q.x * q.x + q.z * q.z), 2.0f * (q.y * q.z + q.w * q.x));
}

static glm::dvec3 GetForwardVector(const glm::dquat &q) {
  return glm::dvec3(2.0f * (q.x * q.z + q.w * q.y), 2.0f * (q.y * q.z - q.w * q.x), 1.0f - 2.0f * (q.x * q.x + q.y * q.y));
}

static glm::dvec3 GetRightVector(const glm::dquat &q) {
  return glm::dvec3(1.0f - 2.0f * (q.y * q.y + q.z * q.z), 2.0f * (q.x * q.y + q.w * q.z), 2.0f * (q.x * q.z - q.w * q.y));
}


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


#endif //OPENLRR_UTILS_H

//
// Created by Sam Serrels on 16/12/2019.
//

#ifndef OPENLRR_GAME_GRAPHICS_H
#define OPENLRR_GAME_GRAPHICS_H

#include <glm/glm.hpp>
namespace Game {
struct Vertex {
  glm::vec3 p;
  glm::vec3 c;
  glm::vec3 n;
};
} // namespace Game
#endif // OPENLRR_GAME_GRAPHICS_H

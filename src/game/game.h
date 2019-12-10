
#ifndef OPENLRR_GAME_H
#define OPENLRR_GAME_H
#include <glm/glm.hpp>

namespace Game {

void StartUp();
void Tick(double dt);

glm::vec3* getVertices(size_t& count);
glm::uint16_t* getIndices(size_t& count);

} // namespace Game

#endif

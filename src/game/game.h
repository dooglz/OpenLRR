
#ifndef OPENLRR_GAME_H
#define OPENLRR_GAME_H
#include "game_graphics.h"
#include <glm/glm.hpp>

namespace Game {

void StartUp();
void Tick(double dt);

Vertex* getVertices(size_t& count);
glm::uint16_t* getIndices(size_t& count);

} // namespace Game

#endif

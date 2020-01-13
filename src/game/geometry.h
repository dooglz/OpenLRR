
#pragma once
#include "game_graphics.h"
#include <vector>

struct simpleGeo {
  std::vector<Game::Vertex> v;
  std::vector<uint16_t> i;
};

simpleGeo debugCube() {
  simpleGeo g;

  g.v.reserve(24);
  g.i.reserve(24);
  // debug cube - clockwise
  // const glm::vec3 dbgCubePos(5, 5, 5);

  // top
  g.v[0].p = glm::vec3(0, 0, 1);
  g.v[1].p = glm::vec3(1, 0, 1);
  g.v[2].p = glm::vec3(1, 1, 1);
  g.v[3].p = glm::vec3(0, 0, 1);
  g.v[4].p = glm::vec3(1, 1, 1);
  g.v[5].p = glm::vec3(0, 1, 1);
  g.i[0] = 0;
  g.i[1] = 1;
  g.i[2] = 2;
  g.i[3] = 3;
  g.i[4] = 4;
  g.i[5] = 5;
  // bottom
  g.v[6].p = glm::vec3(0, 0, 0);
  g.v[7].p = glm::vec3(1, 0, 0);
  g.v[8].p = glm::vec3(1, 1, 0);
  g.v[9].p = glm::vec3(0, 0, 0);
  g.v[10].p = glm::vec3(1, 1, 0);
  g.v[11].p = glm::vec3(0, 1, 0);
  g.i[8] = 6;
  g.i[7] = 7;
  g.i[6] = 8;
  g.i[11] = 9;
  g.i[10] = 10;
  g.i[9] = 11;
  ////Front side
  g.v[12].p = glm::vec3(0, 1, 0);
  g.v[13].p = glm::vec3(0, 1, 1);
  g.v[14].p = glm::vec3(1, 1, 0);
  g.v[15].p = glm::vec3(0, 1, 1);
  g.v[16].p = glm::vec3(1, 1, 1);
  g.v[17].p = glm::vec3(1, 1, 0);
  g.i[12] = 12;
  g.i[13] = 13;
  g.i[14] = 14;
  g.i[15] = 15;
  g.i[16] = 16;
  g.i[17] = 17;
  ////Back side
  g.v[18].p = glm::vec3(0, 0, 0);
  g.v[19].p = glm::vec3(0, 0, 1);
  g.v[20].p = glm::vec3(1, 0, 0);
  g.v[21].p = glm::vec3(0, 0, 1);
  g.v[22].p = glm::vec3(1, 0, 1);
  g.v[23].p = glm::vec3(1, 0, 0);
  g.i[18] = 20;
  g.i[19] = 19;
  g.i[20] = 18;
  g.i[21] = 23;
  g.i[22] = 22;
  g.i[23] = 21;

  for (auto& v : g.v) {
    v.p -= 0.5f;
    // v.p *= 4.0f;
    // v.p += (dbgCubePos);
    v.n = glm::vec3(1, 0, 0);
    v.c = glm::vec3(0, 1, 0);
  }

}

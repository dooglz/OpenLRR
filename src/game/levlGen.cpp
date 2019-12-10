
#include "level.h"
#include <FastNoise.h>
#include <array>
#include <cmath>
#include <glm/glm.hpp>
#include <iostream>
#include <optional>
#include <set>
#include <vector>

using namespace Game;

// perlin noise rarely goes close to 0 or 1, usually between .2 and .7, this value stretches that up.
#define perlinPush 1.5
#define noiseToVal(mini, maxi, a) mini + (uint8_t)(round(((double)(maxi - mini)) * ((1.0 + (a * perlinPush)) * 0.5)))

void Triangulate(const std::array<Tile, levelSize * levelSize> &tiles, std::array<glm::vec3, nVerts> &verts,
                 std::array<uint16_t, indiceCount> &inidces);

bool canMerge(const std::vector<idx> &setA, const std::vector<idx> &setB) {
  for (size_t a = 0; a < setA.size(); a++) {
    for (size_t b = 0; b < setB.size(); b++) {
      // Test if adjacent
      if (setA[a].x == setB[b].x) {
        if (setA[a].y == setB[b].y + 1 || setA[a].y == setB[b].y - 1) {
          return true;
        }
      } else if (setA[a].y == setB[b].y) {
        if (setA[a].y == setB[b].x + 1 || setA[a].x == setB[b].x - 1) {
          return true;
        }
      }
    }
  }
  return false;
}

bool validateMap(std::array<Tile, levelSize * levelSize> &tiles, idx &SpawnPoint) {
  std::vector<std::vector<idx>> emptyAreas;
  for (size_t i = 0; i < levelSize; ++i) {
    for (size_t j = 0; j < levelSize; ++j) {
      Tile &t = tiles[j + (i * levelSize)];
      if (t.type == Tile::empty) {
        emptyAreas.push_back({{i, j}});
      }
    }
  }
  // clusterEmpties
  bool didmerge = true;
  while (didmerge) {
    didmerge = false;
    for (size_t i = 0; i < emptyAreas.size(); i++) {
      // Can this merge with anything?
      for (size_t j = i + 1; j < emptyAreas.size(); j++) {
        std::vector<idx> &setA = emptyAreas[i];
        std::vector<idx> &setB = emptyAreas[j];
        if (canMerge(setA, setB)) {
          setA.insert(setA.end(), setB.begin(), setB.end());
          emptyAreas.erase(emptyAreas.begin() + j);
          didmerge = true;
          j--;
        }
      }
    }
  }
  const auto &LargestCave =
          std::max_element(emptyAreas.begin(), emptyAreas.end(),
                           [](std::vector<idx> i, std::vector<idx> j) { return i.size() < j.size(); });
  if (LargestCave->size() < 10) {
    return false;
  }

  idx small = (*LargestCave)[0];
  idx big = (*LargestCave)[0];
  std::vector<idx> possibleSpawns;
  for (size_t i = 0; i < LargestCave->size(); i++) {
    const auto &t = (*LargestCave)[i];
    small.x = std::min(small.x, t.x);
    small.y = std::min(small.y, t.y);
    big.x = std::max(big.y, t.y);
    big.y = std::max(big.y, t.y);
    // spawn needs 8 surrounding tiles
    size_t surround = 0;
    for (size_t j = 0; j < LargestCave->size(); j++) {
      if (i == j) {
        continue;
      }
      if (t.surround((*LargestCave)[j])) {
        surround++;
      }
    }
    if (surround >= 8) {
      possibleSpawns.push_back(t);
    }
  }
  if (possibleSpawns.empty()) {
    return false;
  }

  idx center = {(big.x - small.x) / 2, (big.y - small.y) / 2};
  SpawnPoint =
          idx(*std::max_element(possibleSpawns.begin(), possibleSpawns.end(),
                                [center](idx i, idx j) { return i.dist(center) < j.dist(center); }));
  tiles[SpawnPoint.y + (SpawnPoint.x * levelSize)].isSpawn = true;

  std::cout << emptyAreas.size() << " caves, Spawn at: " << SpawnPoint << std::endl;
  return true;
}

void PrintMap(std::array<Tile, levelSize * levelSize> tiles) {
  std::cout << std::endl;
  for (int i = 0; i < levelSize; ++i) {
    for (int j = 0; j < levelSize; ++j) {
      std::cout << tiles[j + (i * levelSize)].tostr() << ".";
    }
    std::cout << std::endl;
  }
}

std::array<Tile, levelSize * levelSize> Generate() {
  std::array<Tile, levelSize * levelSize> tiles;
  FastNoise noise, noise2;
  noise.SetFrequency(0.1);
  noise2.SetFrequency(0.3);

  for (int i = 0; i < levelSize; ++i) {
    for (int j = 0; j < levelSize; ++j) {
      Tile &t = tiles[j + (i * levelSize)];
      t.height = noiseToVal(0, 5, noise.GetPerlin(i, j));
      if (i == 0 || j == 0 || i == (levelSize - 1) || j == (levelSize - 1)) {
        t.type = Tile::solid;
      } else if (t.height == 0) {
        t.type = Tile::water;
      } else {
        uint8_t rval = noiseToVal(0, Tile::RockTypesCount, noise2.GetCellular(i, j));
        if (rval == 0) {
          t.type = Tile::solid;
        } else if (rval >= Tile::RockTypesCount) {
          t.type = Tile::empty;
        } else {
          t.type = Tile::rock;
          t.rockType = Tile::RockTypes(rval);
          if (t.rockType == Tile::vein) {
            if (rand() % 10 != 0) {
              t.rockType = Tile::lose;
            }
          }
        }
      }
    }
  }
  return tiles;
}


Level::Level() {
  _tiles = Generate();
  idx spawnpoint;
  bool generated = false;
  const bool shouldV = false;
  if (shouldV) {
    for (size_t i = 0; i < 20; i++) {
      if (validateMap(_tiles, spawnpoint)) {
        generated = true;
        break;
      }
      std::cout << "." << std::endl;
      _tiles = Generate();
    }
    if (!generated) {
      throw std::runtime_error("LevelGen no validate");
    }
  } else {
    validateMap(_tiles, spawnpoint);
  }
  PrintMap(_tiles);
  Triangulate(_tiles, _verts, _inidces);
  std::cout << _verts.size() << " verts,  indices:" << _inidces.size() << std::endl;
}

Level::~Level() {}


#define dimAt(a, b, s) b + (a * s)
#define dimPos(a, s)                                                                                                                                 \
  a == 0 ? idx{0, 0} : idx { (size_t) floor(a / s), a - (((size_t)floor(a / s)) * s) }

#define TileAt(a, b) tiles[dimAt(a, b, levelSize)];
#define TilePos(a) dimPos(a, levelSize);

#define getAsosiatedVerts(a, b)                                                                                                                      \
  {                                                                                                                                                  \
    {a, b}, {a, b + 1}, {a + 1, b}, { a + 1, b + 1 }                                                                                                 \
  }

#define VertAt(a, b) verts[dimAt(a, b, nVertsDim)]

//THIS AINT RIGHT
#define VertPos(a) dimPos(a, nVertsDim)


std::vector<Game::idx> getAssTiles(size_t a, size_t b, size_t s) {
  s = s - 1;
  std::vector<Game::idx> v;
  if (a < s && b < s) {
    v.emplace(v.end(), a, b);
  }
  if (a > 0 && b < s) {
    v.emplace(v.end(), a - 1, b);
  }
  if (b > 0 && a < s) {
    v.emplace(v.end(), a, b - 1);
  }
  if (a > 0 && b > 0) {
    v.emplace(v.end(), a - 1, b - 1);
  }

  return v;
};

void Triangulate(const std::array<Tile, levelSize * levelSize> &tiles,
                 std::array<glm::vec3, nVerts> &verts, std::array<uint16_t, indiceCount> &inidces) {

  // set heights
  for (int i = 0; i < tiles.size(); ++i) {
    const Tile &t = tiles[i];
    idx tp = TilePos(i);
    std::array<idx, 4> averts = {getAsosiatedVerts(tp.x, tp.y)};
    VertAt(averts[1].x, averts[1].y).z = t.height;
    VertAt(averts[2].x, averts[2].y).z = t.height;
  }
  verts[0].z = tiles[0].height;
  verts[verts.size() - 1].z = tiles[tiles.size() - 1].height;

  for(auto& v : verts){
    //v.z = v.z * 0.1;
    v.z =0;
  }

  //sqaures share verts, so total verts may nto be a sqaure number
  //So we got to do it this ass backwards way, not the way you think.
  // set XY
  for (int k = 0; k < verts.size(); ++k) {
    glm::vec3& v = verts[k];
    idx vp = VertPos(k);
    v.x = vp.x;
    v.y = vp.y;
  }
/*
  for (int i = 0; i < nVertsDim; ++i) {
    for (int j = 0; j < nVertsDim; ++j) {
      glm::vec3 &vert = VertAt(i, j);
      vert.x = i;
      vert.y = j;
    }
  }*/

  // GenerateIndices
  if (indiceCount >= std::numeric_limits<uint16_t>::max()) {
    throw std::runtime_error("Map too BIG!");
  }

  for (int i = 0; i < tiles.size(); ++i) {
    // inidces[j]
  }
  size_t ptr = 0;
  for (int i = 0; i < levelSize; ++i) {
    for (int j = 0; j < levelSize; ++j) {
      inidces[ptr + 0] = j + (i * (levelSize + 1));
      inidces[ptr + 1] = j + (i * (levelSize + 1)) + 1;
      inidces[ptr + 2] = j + (i * (levelSize + 1)) + 5;
      //
      inidces[ptr + 3] = j + (i * (levelSize + 1)) + 1;
      inidces[ptr + 4] = j + (i * (levelSize + 1)) + 6;
      inidces[ptr + 5] = j + (i * (levelSize + 1)) + 5;
      ptr += 6;
    }
  }

}
// 1 - 1t - 4v
// 2 - 4t - 9v
// 3 - 9t - 16v
// 4 - 16t - 25v

// (1,1) > (1,1)(0,0)(1,0)(0,1)
//(0,0) > (0,0)
//(0,1) > (0,0) (0,2)
//(0,2) > (0,1) (0,2)
//(0,3) > (0,2) (0,3)
//(0,4) > (0,3)

//(1,2) >(1,2)(0,1)(0,2)(1,1)

// 0,0 - 0,1,5  1,6,5
// 0,1 - 1,2,6  2,7,6
// 0,2 - 2,3,7  3,8,7
// 0,3 - 3,4,8  4,9,8

// 1,0 - 5,6,10 6,11,10
// 1,1 - 6,7,11 7,12,11
// 1,2 - 7,8,12 8,13,12


#include "level.h"
#include <FastNoise.h>
#include <algorithm>
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

void Level::PrintMap(std::array<Tile, levelSize * levelSize> tiles, bool masks) {
  std::cout << std::endl;
  for (int i = 0; i < levelSize; ++i) {
    for (int j = 0; j < levelSize; ++j) {
      if (masks) {
        std::cout << tiles[j + (i * levelSize)].rockmask << ".";
      } else {
        std::cout << tiles[j + (i * levelSize)].tostr() << ".";
      }
    }
    std::cout << std::endl;
  }
}

void Triangulate(std::array<Tile, nTiles>& tiles, std::array<glm::vec3, nVerts>& verts, std::array<uint16_t, indiceCount>& inidces);

bool canMerge(const std::vector<idx>& setA, const std::vector<idx>& setB) {
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

void Level::SquashWalls(std::array<Tile, nTiles>& tiles) {
  size_t squashcount = 0;
  bool didsquash = true;
  std::vector<idx> walls;
  for (size_t i = 0; i < nTiles; i++) {
    if (tiles[i].type == Tile::rock) {
      walls.push_back(TilePos(i, levelSize));
    }
  }
  while (didsquash) {
    didsquash = false;
    for (size_t i = 0; i < walls.size(); ++i) {
      const idx& w = walls[i];
      if (w.x == 0 || w.y == 0 || w.x == levelSize - 1 || w.y == levelSize - 1) {
        continue;
      }

      std::vector<idx> adjacentWalls;
      std::vector<idx> surroundingWalls;
      for (size_t j = 0; j < walls.size(); ++j) {
        if (i == j) {
          continue;
        }
        if (w.surround(walls[j])) {
          surroundingWalls.push_back(walls[j]);
          if (w.adjacent(walls[j])) {
            adjacentWalls.push_back(walls[j]);
          }
        }
      }

      if (adjacentWalls.size() > 2) {
        continue;
      }
      // if only 2 adj, there must be 1 surrounding that is adj to both these adj
      if (adjacentWalls.size() == 2) {
        bool saveCondition = false;
        for (const auto& surr : surroundingWalls) {
          if (surr.adjacent(adjacentWalls[0]) && surr.adjacent(adjacentWalls[1])) {
            saveCondition = true;
            break;
          }
        }
        if (saveCondition) {
          continue;
        }
      }

      didsquash = true;
      squashcount++;
      Tile& t = TileAt(tiles, w.x, w.y, levelSize);
      t.type = Tile::empty;
      t.rockType.reset();
      // remove me from walls
      walls.erase(walls.begin() + i);
      i--;
    }
  }
  std::cout << "Suqashed Rocks: " << squashcount << std::endl;
}

bool Level::validateMap(std::array<Tile, levelSize * levelSize>& tiles, idx& SpawnPoint) {
  PrintMap(tiles);
  SquashWalls(tiles);
  std::vector<std::vector<idx>> emptyAreas;
  for (size_t i = 0; i < levelSize; ++i) {
    for (size_t j = 0; j < levelSize; ++j) {
      Tile& t = tiles[j + (i * levelSize)];
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
        std::vector<idx>& setA = emptyAreas[i];
        std::vector<idx>& setB = emptyAreas[j];
        if (canMerge(setA, setB)) {
          setA.insert(setA.end(), setB.begin(), setB.end());
          emptyAreas.erase(emptyAreas.begin() + j);
          didmerge = true;
          j--;
        }
      }
    }
  }
  const auto& LargestCave =
      std::max_element(emptyAreas.begin(), emptyAreas.end(), [](std::vector<idx> i, std::vector<idx> j) { return i.size() < j.size(); });
  if (LargestCave->size() < 10) {
    return false;
  }

  idx small = (*LargestCave)[0];
  idx big = (*LargestCave)[0];
  std::vector<idx> possibleSpawns;
  for (size_t i = 0; i < LargestCave->size(); i++) {
    const auto& t = (*LargestCave)[i];
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
      idx(*std::max_element(possibleSpawns.begin(), possibleSpawns.end(), [center](idx i, idx j) { return i.dist(center) < j.dist(center); }));
  tiles[SpawnPoint.y + (SpawnPoint.x * levelSize)].isSpawn = true;

  std::cout << emptyAreas.size() << " caves, Spawn at: " << SpawnPoint << std::endl;
  return true;
}

std::array<Tile, levelSize * levelSize> Level::Generate() {
  std::array<Tile, levelSize * levelSize> tiles;
  FastNoise noise, noise2;
  noise.SetFrequency(0.1f);
  noise2.SetFrequency(0.3f);

  for (int i = 0; i < levelSize; ++i) {
    for (int j = 0; j < levelSize; ++j) {
      Tile& t = tiles[j + (i * levelSize)];
      t.height = noiseToVal(0, 5, noise.GetPerlin(i, j));
      if (i == 0 || j == 0 || i == (levelSize - 1) || j == (levelSize - 1)) {
        t.type = Tile::rock;
        t.rockType = Tile::solid;
      } else if (t.height == 0) {
        t.type = Tile::water;
      } else {
        uint8_t rval = noiseToVal(0, Tile::RockTypesCount, noise2.GetCellular(i, j));
        if (rval == 0) {
          t.type = Tile::rock;
          t.rockType = Tile::solid;
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
      if (FLATLEVEL && t.height != 0) {
        t.height = 1;
      }
    }
  }
  return tiles;
}

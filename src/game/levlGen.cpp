
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

#define dimAt(a, b, s) b + (a * s)
#define dimPos(a, s)                                                                                                                                 \
  a == 0 ? idx{0, 0} : idx { (size_t) floor(a / s), a - (((size_t)floor(a / s)) * s) }

#define TileAt(a, b) tiles[dimAt(a, b, levelSize)];
#define TilePos(a) dimPos(a, levelSize);

#define getAsosiatedVerts(a, b)                                                                                                                      \
  {                                                                                                                                                  \
    {a, b}, {a, b + 1}, {a + 1, b}, { a + 1, b + 1 }                                                                                                 \
  }
auto adjacentTiles = [](idx t) {
  std::vector<idx> adj;
  if (t.x > 0) {
    adj.push_back({t.x - 1, t.y});
  }
  if (t.y > 0) {
    adj.push_back({t.x, t.y - 1});
  }
  if (t.x < levelSize - 1) {
    adj.push_back({t.x + 1, t.y});
  }
  if (t.y < levelSize - 1) {
    adj.push_back({t.x, t.y + 1});
  }
  return adj;
};

#define VertAt(a, b) verts[dimAt(a, b, (levelSize + 1))]

void PrintMap(std::array<Tile, levelSize * levelSize> tiles) {
  std::cout << std::endl;
  for (int i = 0; i < levelSize; ++i) {
    for (int j = 0; j < levelSize; ++j) {
      std::cout << tiles[j + (i * levelSize)].tostr() << ".";
    }
    std::cout << std::endl;
  }
}

void Triangulate(const std::array<Tile, levelSize * levelSize>& tiles, std::array<glm::vec3, nVerts>& verts,
                 std::array<uint16_t, indiceCount>& inidces);

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

void SquashWalls(std::array<Tile, levelSize * levelSize>& tiles) {
  size_t squashcount = 0;
  bool didsquash = true;
  while (didsquash) {
    didsquash = false;
    std::vector<idx> walls;
    for (size_t i = 0; i < levelSize; ++i) {
      for (size_t j = 0; j < levelSize; ++j) {
        Tile& t = tiles[j + (i * levelSize)];
        if (t.type == Tile::rock) {
          walls.push_back({i, j});
        }
      }
    }
    for (size_t i = 0; i < walls.size(); ++i) {
      const idx& w = walls[i];
      size_t adjacent = 0;
      for (size_t j = 0; j < walls.size(); ++j) {

        if (i == j) {
          continue;
        }
        if (w.surround(walls[j])) {
          adjacent++;
        }
      }
      if (adjacent < 3) {
        didsquash = true;
        squashcount++;
        Tile& t = tiles[w.y + (w.x * levelSize)];
        t.type = Tile::empty;
        t.rockType.reset();
        // remove me from walls
        walls.erase(walls.begin() + i);
        i--;
      }
    }
  }
  std::cout << "Suqashed Rocks: " << squashcount << std::endl;
}

bool validateMap(std::array<Tile, levelSize * levelSize>& tiles, idx& SpawnPoint) {
  // TODO squash freestanding walls;
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

std::array<Tile, levelSize * levelSize> Generate() {
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

Level::Level() {
  _tiles = Generate();

  bool generated = false;
  const bool shouldV = false;
  if (shouldV) {
    for (size_t i = 0; i < 20; i++) {
      if (validateMap(_tiles, _spawnpoint)) {
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
    validateMap(_tiles, _spawnpoint);
  }
  PrintMap(_tiles);
  Triangulate(_tiles, _verts, _inidces);
  std::cout << _verts.size() << " verts,  indices:" << _inidces.size() << std::endl;
}

Level::~Level() {}

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

void Triangulate(const std::array<Tile, levelSize * levelSize>& tiles, std::array<glm::vec3, nVerts>& verts,
                 std::array<uint16_t, indiceCount>& inidces) {

  // set heights
  for (int i = 0; i < tiles.size(); ++i) {

    const Tile& t = tiles[i];
    idx tp = TilePos(i);
    std::array<idx, 4> averts = {getAsosiatedVerts(tp.x, tp.y)};
    VertAt(averts[1].x, averts[1].y).z = t.height;
    VertAt(averts[2].x, averts[2].y).z = t.height;
    if (t.type == Tile::rock) {
      // rocky horror
      // anywhere between 1 and 4 of the tile verts should be at wall height
      for (const auto& adjtAvert : averts) {
        VertAt(adjtAvert.x, adjtAvert.y).z = t.height + wallheight;
      }
      std::vector<idx> adj = adjacentTiles(tp);
      for (const auto adjTidx : adj) {
        const Tile& adjt = TileAt(adjTidx.x, adjTidx.y);
        if (adjt.type != Tile::rock) {
          std::array<idx, 4> adjtAverts = {getAsosiatedVerts(adjTidx.x, adjTidx.y)};
          // any adjtAverts that is also a averts is wallheight
          for (const auto& adjtAvert : adjtAverts) {
            for (const auto& avert : averts) {
              if (adjtAvert == avert) {
                VertAt(adjtAvert.x, adjtAvert.y).z = t.height;
              }
            }
          }
        }
      }
    }
  }
  verts[0].z = tiles[0].height;
  verts[verts.size() - 1].z = tiles[tiles.size() - 1].height;

  // sqaures share verts, so total verts may not be a sqaure number
  // So we got to do it this ass backwards way, not the way you think.
  // set XY
  for (int k = 0; k < verts.size(); ++k) {
    glm::vec3& v = verts[k];
    v.z = v.z * 0.25;
    v.y = (k % (levelSize + 1));
    v.x = floor((float)k / (float)(levelSize + 1));
  }

  // GenerateIndices
  if (indiceCount >= std::numeric_limits<uint16_t>::max()) {
    throw std::runtime_error("Map too BIG!");
  }

  size_t ptr = 0;
  for (int i = 0; i < levelSize; ++i) {
    for (int j = 0; j < levelSize; ++j) {
      inidces[ptr + 0] = (uint16_t)(j + (i * (levelSize + 1)));
      inidces[ptr + 1] = (uint16_t)(j + (i * (levelSize + 1)) + 1);
      inidces[ptr + 2] = (uint16_t)(j + (i * (levelSize + 1)) + (levelSize + 1));
      //
      inidces[ptr + 3] = (uint16_t)(j + (i * (levelSize + 1)) + 1);
      inidces[ptr + 4] = (uint16_t)(j + (i * (levelSize + 1)) + (levelSize + 2));
      inidces[ptr + 5] = (uint16_t)(j + (i * (levelSize + 1)) + (levelSize + 1));
      ptr += 6;
    }
  }
}

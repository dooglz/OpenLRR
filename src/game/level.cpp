#include "level.h"
#include "../engine/graphics/vk/vulkan_internals.h"
#include <glm/gtx/normal.hpp>
#include <set>
using namespace Game;

const std::map<Tile::TileType, bool> Tile::isFlat = {{water, true}};
const std::map<Tile::TileType, std::vector<Tile::TileType>> Tile::matchesHeight = {{rock, {water, empty}}, {empty, {water}}};
bool ShouldBeInverted(size_t sidemask) {
  if (sidemask > 0) {
    if (sidemask == (idx::OrBit::dr) || sidemask == (idx::OrBit::ul) ||                                   // 128
        sidemask == (idx::OrBit::u + idx::OrBit::l) ||                                                    // 9
        sidemask == (idx::OrBit::r + idx::OrBit::d) ||                                                    // 6
        sidemask == (idx::OrBit::ul + idx::OrBit::dr) ||                                                  // 144
        sidemask == (idx::OrBit::dr + idx::OrBit::r + idx::OrBit::d) ||                                   // 22
        sidemask == (idx::OrBit::ul + idx::OrBit::u + idx::OrBit::l) ||                                   // 137
        sidemask == (idx::OrBit::ul + idx::OrBit::ur + idx::OrBit::u + idx::OrBit::l) ||                  // 201
        sidemask == (idx::OrBit::ur + idx::OrBit::dr + idx::OrBit::r + idx::OrBit::d) ||                  // 86
        sidemask == (idx::OrBit::ur + idx::OrBit::dl + idx::OrBit::dr + idx::OrBit::r + idx::OrBit::d) || // 118
        sidemask == (idx::OrBit::ul + idx::OrBit::ur + idx::OrBit::dl + idx::OrBit::u + idx::OrBit::l)    // 233
    ) {
      return true;
    }
  }
  return false;
}

Game::Level::Level() {
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
  Triangulate2(_verts, _inidces);
  PrintMap(_tiles, true);
  std::cout << _verts.size() << " verts,  indices:" << _inidces.size() << std::endl;
}

Level::~Level() {}
// System 1
// under normal conditions:
// a -- b e - f     1 -- 2 2 - 2     a -- b e - f     1 -- 3 3 - 3
// |  1 | | 2 |     |  1 | | 2 |     |  1 | | 3 |     |  1 | | 3 |
// c -- d g - h     1 -- 2 2 - 2     c -- d g - h     1 -- 2 2 - 2
// i -- j m - n ->  1 -- 2 2 - 2     i -- j m - n ->  1 -- 2 2 - 2
// |  1 | | 2 |     |  1 | | 2 |     |  1 | | 2 |     |  1 | | 2 |
// k -- L o - p     1 -- 2 2 - 2     k -- L o - p     1 -- 2 2 - 2
// The last tile to touch a vert get's to set it's height - going right(+y) then down(+x)
// so, Tiles verts are decided by:
// tl - (me)
// tr - (x+1,y)
// bl - (x,y+1)
// br - (x+1,y+1) first, or (x,y+1), or (x+1,y)
// However, for rocks neighbouring empty&water, and empty neighbouring water:
// ANY vert that touches the lowest state goes to that height.
// Water has all 4 verts at water height.

auto SetTileVertZ = [](const idx& p, const unsigned char& vert, const float& newHeight, std::vector<Game::Vertex>& verts, bool observeFlat = false,
                       const std::array<Game::Tile, nTiles>& tiles = {}) {
  const size_t i = dimAt(p.x, p.y, levelSize);
  if (i < levelSize * levelSize) {
    if (observeFlat && i < tiles.size()) {
      auto isAFlatTile = Tile::isFlat.find(tiles[i].type);
      if (isAFlatTile != Tile::isFlat.end() && isAFlatTile->second) {
        return;
      }
    }
    verts[(i * 4) + vert].p.z = newHeight;
  }
};

auto SetTrailingtilesVertZ = [](const idx& p, const float& newHeight, std::vector<Game::Vertex>& verts, bool observeFlat = false,
                                const std::array<Game::Tile, nTiles>& tiles = {}) {
  // ul
  SetTileVertZ({p.x - 1, p.y - 1}, 3, newHeight, verts, observeFlat, tiles);
  // u
  SetTileVertZ({p.x, p.y - 1}, 2, newHeight, verts, observeFlat, tiles);
  SetTileVertZ({p.x, p.y - 1}, 3, newHeight, verts, observeFlat, tiles);
  // ur
  SetTileVertZ({p.x + 1, p.y - 1}, 2, newHeight, verts, observeFlat, tiles);
  // l
  SetTileVertZ({p.x - 1, p.y}, 1, newHeight, verts, observeFlat, tiles);
  SetTileVertZ({p.x - 1, p.y}, 3, newHeight, verts, observeFlat, tiles);
};

auto SetSurroundingtilesVertZ = [](const idx& p, const float& newHeight, std::vector<Game::Vertex>& verts, bool observeFlat = false,
                                   const std::array<Game::Tile, nTiles>& tiles = {}) {
  // ul,u,ur,l
  SetTrailingtilesVertZ(p, newHeight, verts, observeFlat, tiles);
  // r
  SetTileVertZ({p.x + 1, p.y}, 0, newHeight, verts, observeFlat, tiles);
  SetTileVertZ({p.x + 1, p.y}, 2, newHeight, verts, observeFlat, tiles);
  // dl
  SetTileVertZ({p.x - 1, p.y + 1}, 1, newHeight, verts, observeFlat, tiles);
  // d
  SetTileVertZ({p.x, p.y + 1}, 0, newHeight, verts, observeFlat, tiles);
  SetTileVertZ({p.x, p.y + 1}, 1, newHeight, verts, observeFlat, tiles);
  // dr
  SetTileVertZ({p.x + 1, p.y + 1}, 0, newHeight, verts, observeFlat, tiles);
};

auto GetTouchingTileTypesForVert = [](const idx& p, const unsigned char& vert, const std::array<Game::Tile, nTiles>& tiles = {}) {
  std::set<Tile::TileType> touchingTypes;
  const size_t i = dimAt(p.x, p.y, levelSize);
  const size_t vi = (i * 4) + vert;
  const auto dirs = p.getTouchingTilesForVert(vert);
  for (auto tidx : dirs) {
    if (tidx == p) {
      continue;
    }
    if (dimAt(tidx.x, tidx.y, levelSize) >= tiles.size()) {
      continue;
    }
    touchingTypes.insert(tiles[dimAt(tidx.x, tidx.y, levelSize)].type);
  }
  return touchingTypes;
};

void Level::Triangulate2(std::vector<Game::Vertex>& allVerts, std::vector<uint16_t>& allIndices) {
  allVerts.clear();
  allVerts.resize(_tiles.size() * 4);
  allIndices.clear();
  allIndices.resize(_tiles.size() * 6);

  for (int i = 0; i < _tiles.size(); ++i) {
    Vertex* myVerts = &allVerts[i * 4];

    Tile& t = _tiles[i];
    const idx tp = TilePos(i, levelSize);
    const std::array<idx, 4> vertPositions = {getAsosiatedVerts(tp.x, tp.y)};
    // my verts
    for (int j = 0; j < 4; ++j) {
      myVerts[j].p.x = vertPositions[j].x;
      myVerts[j].p.y = vertPositions[j].y;
      myVerts[j].c = t.GetColor();
    }
    // set my heights
    myVerts[0].p.z = t.height;
    myVerts[1].p.z = t.height;
    myVerts[2].p.z = t.height;
    myVerts[3].p.z = t.height;
    // set heights of surrounding previous tiles
    SetTrailingtilesVertZ(tp, t.height, allVerts);
  }

  // flatten flat tiles, prop up walls, calulate rockmask, invert, calc normals.
  for (int i = 0; i < _tiles.size(); ++i) {
    Tile& t = _tiles[i];
    const idx tp = TilePos(i, levelSize);
    auto isAFlatTile = Tile::isFlat.find(t.type);
    if (isAFlatTile != Tile::isFlat.end() && isAFlatTile->second) {
      Vertex* myVerts = &allVerts[i * 4];
      myVerts[0].p.z = t.height;
      myVerts[1].p.z = t.height;
      myVerts[2].p.z = t.height;
      myVerts[3].p.z = t.height;
      // FlattenAllNeighbours
      SetSurroundingtilesVertZ(TilePos(i, levelSize), t.height, allVerts, true, _tiles);
    }

    {
      t.rockmask = 0;
      const auto surroundIdx = tp.getSurTiles(levelSize);
      for (const auto& sidx : surroundIdx) {
        const auto& otherT = _tiles[dimAt(sidx.x, sidx.y, levelSize)];
        if ((t.type == Tile::rock && otherT.type != Tile::rock) || otherT.height < t.height) {
          t.rockmask += tp.orientation(sidx);
        }
      }
      t.inverted = ShouldBeInverted(t.rockmask);
      const uint16_t inOf = (i * 4);
      uint16_t* myIndices = &allIndices[i * 6];
      if (t.inverted) {
        uint16_t Indices[6] = {0 + inOf, 3 + inOf, 2 + inOf, 0 + inOf, 1 + inOf, 3 + inOf};
        memcpy(myIndices, Indices, (6 * sizeof(uint16_t)));
        // myIndices = {0, 3, 2, 0, 1, 3};
      } else {
        uint16_t Indices[6] = {0 + inOf, 1 + inOf, 2 + inOf, 1 + inOf, 3 + inOf, 2 + inOf};
        memcpy(myIndices, Indices, 6 * sizeof(uint16_t));
        //  myIndices = {0, 1, 2, 1, 3, 2};
      }
    }

    if (t.type == Tile::rock) {
      Vertex* myVerts = &allVerts[i * 4];
      unsigned char upPoints = 0;
      for (int j = 0; j < 4; ++j) {
        auto tt = GetTouchingTileTypesForVert(tp, j, _tiles);
        if (tt.size() == 0 || (tt.size() == 1 && tt.find(Tile::rock) != tt.end())) {
          myVerts[j].p.z += wallheight;
          upPoints++;
        }
      }
      if (upPoints == 4) {
        for (int j = 0; j < 4; ++j) {
          myVerts[j].c = Tile::voidColour;
        }
      }
    }

    { /*
      // calculate Normals
      glm::vec3 n1 = glm::triangleNormal(myVerts[myIndices[0]].p, myVerts[myIndices[2]].p, myVerts[myIndices[1]].p);
      myVerts[myIndices[0]].n = n1;
      myVerts[myIndices[1]].n = n1;
      myVerts[myIndices[2]].n = n1;
      glm::vec3 n2 = glm::triangleNormal(myVerts[myIndices[3]].p, myVerts[myIndices[5]].p, myVerts[myIndices[4]].p);
      myVerts[myIndices[3]].n = n2;
      myVerts[myIndices[4]].n = n2;
      myVerts[myIndices[5]].n = n2;*/
    }
  }

  for (auto& v : allVerts) {
    v.p.z = v.p.z * 0.25;
    // v.p.z = 0;
  }

  std::cout << "WamBam" << std::endl;
}

void Level::Render() {}

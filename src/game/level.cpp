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
    uint16_t* myIndices = &allIndices[i * 6];

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
      const uint16_t inOf = ((uint16_t)i * 4);

      // normal        inverted
      // 0 -- 1        0 -- 1
      // | / |         |  \ |
      // 2 -- 3        2 -- 3

      if (t.inverted) {
        uint16_t Indices[6] = {static_cast<uint16_t>(3 + inOf), static_cast<uint16_t>(2 + inOf), static_cast<uint16_t>(0 + inOf),
                               static_cast<uint16_t>(3 + inOf), static_cast<uint16_t>(0 + inOf), static_cast<uint16_t>(1 + inOf)};
        memcpy(myIndices, Indices, (6 * sizeof(uint16_t)));
        // myIndices = {0, 3, 2, 0, 1, 3};
      } else {
        uint16_t Indices[6] = {static_cast<uint16_t>(0 + inOf), static_cast<uint16_t>(1 + inOf), static_cast<uint16_t>(2 + inOf),
                               static_cast<uint16_t>(1 + inOf), static_cast<uint16_t>(3 + inOf), static_cast<uint16_t>(2 + inOf)};
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
    // normal        inverted
    // 0 -- 1        0 -- 1
    // | / |         |  \ |
    // 2 -- 3        2 -- 3

    {
      Vertex* myVerts = &allVerts[i * 4];

      if (t.inverted) {
        myVerts[1].n = normalize(glm::triangleNormal(myVerts[2].p, myVerts[0].p, myVerts[1].p));
        myVerts[2].n = normalize(glm::triangleNormal(myVerts[1].p, myVerts[3].p, myVerts[2].p));
      }else {
        myVerts[0].n = normalize(glm::triangleNormal(myVerts[2].p, myVerts[0].p, myVerts[1].p));

        // myVerts[1].n = normalize(glm::triangleNormal(myVerts[0].p, myVerts[1].p, myVerts[2].p));
        // myVerts[2].n = normalize(glm::triangleNormal(myVerts[3].p, myVerts[2].p, myVerts[1].p));

        myVerts[3].n = normalize(glm::triangleNormal(myVerts[1].p, myVerts[3].p, myVerts[2].p));
      }
      // calculate Normals
      //glm::vec3 n1 = normalize(glm::triangleNormal(allVerts[myIndices[0]].p, allVerts[myIndices[1]].p, allVerts[myIndices[2]].p));
      //allVerts[myIndices[0]].n = n1;
      //allVerts[myIndices[1]].n = n1;
      //allVerts[myIndices[2]].n = n1;
      //glm::vec3 n2 = normalize(glm::triangleNormal(allVerts[myIndices[3]].p, allVerts[myIndices[4]].p, allVerts[myIndices[5]].p));
      //allVerts[myIndices[3]].n = n2;
      //allVerts[myIndices[4]].n = n2;
      //allVerts[myIndices[5]].n = n2;
    }
  }
  // glm::vec3 a(0,0,0);
  // glm::vec3 b(1, 0, 0);
  // glm::vec3 c(0, 1, 0);
  glm::vec3 a(0, 0, 1);
  glm::vec3 b(0, 0, 0);
  glm::vec3 c(0, 1, 0);

  glm::vec3 n1 = glm::triangleNormal(a, b, c);
  // glm::vec3 n2 = glm::triangleNormal(a, c, b);
  // glm::vec3 n3 = glm::triangleNormal(b, a, c);
  glm::vec3 n4 = glm::triangleNormal(b, c, a);
  glm::vec3 n5 = glm::triangleNormal(c, a, b);

  // glm::vec3 n6 = glm::triangleNormal(c, b, a);

  // debug cube - clockwise
  const glm::vec3 dbgCubePos(5, 5, 5);
  std::vector<Game::Vertex> dbgV(24);
  std::vector<uint16_t> dbgCubeInd(24);

  // top
  dbgV[0].p = glm::vec3(0, 0, 1);
  dbgV[1].p = glm::vec3(1, 0, 1);
  dbgV[2].p = glm::vec3(1, 1, 1);
  dbgV[3].p = glm::vec3(0, 0, 1);
  dbgV[4].p = glm::vec3(1, 1, 1);
  dbgV[5].p = glm::vec3(0, 1, 1);
  dbgCubeInd[0] = 0;
  dbgCubeInd[1] = 1;
  dbgCubeInd[2] = 2;
  dbgCubeInd[3] = 3;
  dbgCubeInd[4] = 4;
  dbgCubeInd[5] = 5;
  // bottom
  dbgV[6].p = glm::vec3(0, 0, 0);
  dbgV[7].p = glm::vec3(1, 0, 0);
  dbgV[8].p = glm::vec3(1, 1, 0);
  dbgV[9].p = glm::vec3(0, 0, 0);
  dbgV[10].p = glm::vec3(1, 1, 0);
  dbgV[11].p = glm::vec3(0, 1, 0);
  dbgCubeInd[8] = 6;
  dbgCubeInd[7] = 7;
  dbgCubeInd[6] = 8;
  dbgCubeInd[11] = 9;
  dbgCubeInd[10] = 10;
  dbgCubeInd[9] = 11;
  ////Front side
  dbgV[12].p = glm::vec3(0, 1, 0);
  dbgV[13].p = glm::vec3(0, 1, 1);
  dbgV[14].p = glm::vec3(1, 1, 0);
  dbgV[15].p = glm::vec3(0, 1, 1);
  dbgV[16].p = glm::vec3(1, 1, 1);
  dbgV[17].p = glm::vec3(1, 1, 0);
  dbgCubeInd[12] = 12;
  dbgCubeInd[13] = 13;
  dbgCubeInd[14] = 14;
  dbgCubeInd[15] = 15;
  dbgCubeInd[16] = 16;
  dbgCubeInd[17] = 17;
  ////Back side
  dbgV[18].p = glm::vec3(0, 0, 0);
  dbgV[19].p = glm::vec3(0, 0, 1);
  dbgV[20].p = glm::vec3(1, 0, 0);
  dbgV[21].p = glm::vec3(0, 0, 1);
  dbgV[22].p = glm::vec3(1, 0, 1);
  dbgV[23].p = glm::vec3(1, 0, 0);
  dbgCubeInd[18] = 20;
  dbgCubeInd[19] = 19;
  dbgCubeInd[20] = 18;
  dbgCubeInd[21] = 23;
  dbgCubeInd[22] = 22;
  dbgCubeInd[23] = 21;

  for (auto& v : dbgV) {
    v.p -= 0.5f;
    // v.p *= 4.0f;
    v.p += (dbgCubePos);
    v.n = glm::vec3(1, 0, 0);
    v.c = glm::vec3(0, 1, 0);
  }

  for (auto& v : allVerts) {
    v.p.z = v.p.z * 0.25;
    // v.p.z = 0;
  }

  const auto cnt = allVerts.size();
  std::transform(dbgCubeInd.begin(), dbgCubeInd.end(), std::back_inserter(allIndices), [cnt](auto& c) { return c + cnt; });
  allVerts.insert(allVerts.end(), dbgV.begin(), dbgV.end());

  std::cout << "WamBam" << std::endl;
}

void Level::Render() {}

#include "level.h"
#include "../engine/graphics/vk/vulkan_internals.h"
#include <glm/gtx/normal.hpp>
using namespace Game;

const std::map<Tile::TileType, bool> Tile::isFlat = {{water, true}};
const std::map<Tile::TileType, std::vector<Tile::TileType>> Tile::matchesHeight = {{rock, {water,empty}}, {empty, {water}}};
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

std::array<size_t, 4> TileVertHeights(const Tile& myTile, const idx& myPos, size_t height, const std::array<Tile, levelSize * levelSize>& tiles) {
  auto isAFlatTile = Tile::isFlat.find(myTile.type);
  if (isAFlatTile != Tile::isFlat.end() && isAFlatTile->second) {
    return {height, height, height, height};
  }

  auto GrabHeight = [&tiles, &height, &myTile](const std::vector<idx>& coords) -> size_t {
    for (auto& c : coords) {
      const auto i = dimAt(c.x, c.y, levelSize);
      if (i < tiles.size()) {
        return tiles[i].height + (tiles[i].type == Tile::rock && myTile.type == Tile::rock ? wallheight : 0);
      }
    }
    return height + (myTile.type == Tile::rock ? wallheight : 0);
  };
  auto GrabHeight2 = [&tiles, &height, &myTile](const std::vector<std::tuple<Tile::TileType, idx>>& coords) -> size_t {
    for (auto& c : coords) {
      auto cIdx = std::get<1>(c);
      auto cType = std::get<0>(c);
      const auto i = dimAt(cIdx.x, cIdx.y, levelSize);
      if (i < tiles.size() && (cType == Tile::TileTypeCount || cType == tiles[i].type)) {
       // auto gg = TileVertHeights(tiles[i], cIdx, tiles[i].height, tiles)[3];
     //   return gg + (tiles[i].type == Tile::rock && myTile.type == Tile::rock ? wallheight : 0);
        //return tiles[i].height + (tiles[i].type == Tile::rock && myTile.type == Tile::rock ? wallheight : 0);
      }
    }
    return height + (myTile.type == Tile::rock ? wallheight : 0);
  };

  // does this tiletype have matching conditions (e.g empty,rock)
  auto needsToMatch = Tile::matchesHeight.find(myTile.type);
  if (needsToMatch != Tile::matchesHeight.end() && !needsToMatch->second.empty()) {
    const std::vector<idx> surTiles = myPos.getSurTiles(levelSize);
    std::array<size_t, 4> heights;
    std::vector<std::tuple<Tile::TileType, idx>> LookupCoords[4];
    for (const auto& type : needsToMatch->second) {
      LookupCoords[0].push_back({type, {myPos.x - 1, myPos.y}});
      LookupCoords[0].push_back({type, {myPos.x, myPos.y - 1}});
      LookupCoords[0].push_back({type, {myPos.x - 1, myPos.y - 1}});
      LookupCoords[1].push_back({type, {myPos.x + 1, myPos.y}});
      LookupCoords[1].push_back({type, {myPos.x + 1, myPos.y - 1}});
      LookupCoords[1].push_back({type, {myPos.x, myPos.y - 1}});
      LookupCoords[2].push_back({type, {myPos.x, myPos.y + 1}});
      LookupCoords[2].push_back({type, {myPos.x - 1, myPos.y + 1}});
      LookupCoords[2].push_back({type, {myPos.x - 1, myPos.y}});
      LookupCoords[3].push_back({type, {myPos.x + 1, myPos.y + 1}});
      LookupCoords[3].push_back({type, {myPos.x, myPos.y + 1}});
      LookupCoords[3].push_back({type, {myPos.x + 1, myPos.y}});
    }
    LookupCoords[1].push_back({Tile::TileTypeCount, {myPos.x + 1, myPos.y}});
    LookupCoords[2].push_back({Tile::TileTypeCount, {myPos.x, myPos.y + 1}});
    LookupCoords[3].push_back({Tile::TileTypeCount, {myPos.x + 1, myPos.y + 1}});
    LookupCoords[3].push_back({Tile::TileTypeCount, {myPos.x, myPos.y + 1}});
    LookupCoords[3].push_back({Tile::TileTypeCount, {myPos.x + 1, myPos.y}});

    heights[0] = GrabHeight2(LookupCoords[0]);
    heights[1] = GrabHeight2(LookupCoords[1]);
    heights[2] = GrabHeight2(LookupCoords[2]);
    heights[3] = GrabHeight2(LookupCoords[3]);
    return heights;
  } else {
    std::array<size_t, 4> heights;
    heights[0] = height;
    heights[1] = GrabHeight({{myPos.x + 1, myPos.y}});
    heights[2] = GrabHeight({{myPos.x, myPos.y + 1}});
    heights[3] = GrabHeight({{myPos.x + 1, myPos.y + 1}, {myPos.x, myPos.y + 1}, {myPos.x + 1, myPos.y}});
    return heights;
  }
}

size_t MatchHeight(const Tile& myTile, const idx& myPos, std::array<Game::Vertex, 4>& myVerts,
                   const std::array<Tile, levelSize * levelSize>& tiles, size_t height) {
  auto heights = TileVertHeights(myTile, myPos, height, tiles);
  for (int i = 0; i < 4; ++i) {
      myVerts[i].p.z = heights[i];
  }
  size_t sidemask = 0;
  const std::vector<idx> surTiles = myPos.getSurTiles(levelSize);
  //do we need to invert?
  auto needsToMatch = Tile::matchesHeight.find(myTile.type);
  if (needsToMatch != Tile::matchesHeight.end() && !needsToMatch->second.empty()) {
    for (const auto& type : needsToMatch->second) {
      for (int j = 0; j < surTiles.size(); ++j) {
        const idx& surTidx = surTiles[j];
        const Tile& surT = TileAt(tiles, surTidx.x, surTidx.y, levelSize);
        if (surT.type == type) {
          auto direction = myPos.orientation(surTidx);
          sidemask += direction;
          /*
          //  auto newHeights = TileVertHeights(surTidx, surT.height,tiles, )
          if (direction == idx::OrBit::u || direction == idx::OrBit::ul || direction == idx::OrBit::l) {
            myVerts[0].p.z = surT.height;
          }
          if (direction == idx::OrBit::u || direction == idx::OrBit::ur || direction == idx::OrBit::r) {
            myVerts[1].p.z = surT.height;
          }
          if (direction == idx::OrBit::d || direction == idx::OrBit::dl || direction == idx::OrBit::l) {
            myVerts[2].p.z = surT.height;
          }
          if (direction == idx::OrBit::d || direction == idx::OrBit::dr || direction == idx::OrBit::r) {
            myVerts[3].p.z = surT.height;
          }*/
        }
      }
    }
  }


  return sidemask;
}

void Level::Triangulate2(std::vector<Vertex>& allVerts, std::vector<uint16_t>& allIndices) {
  allVerts.clear();
  allVerts.reserve(_tiles.size() * 4);
  allIndices.clear();
  allIndices.reserve(_tiles.size() * 6);

  for (int i = 0; i < _tiles.size(); ++i) {
    std::array<Vertex, 4> myVerts;
    std::array<uint16_t, 6> myIndices;
    Tile& t = _tiles[i];
    idx tp = TilePos(i, levelSize);
    std::array<idx, 4> vertPositions = {getAsosiatedVerts(tp.x, tp.y)};
    // 0 -- 1
    // |  / |
    // 2 -- 3
    for (int j = 0; j < 4; ++j) {
      myVerts[j].p.x = vertPositions[j].x;
      myVerts[j].p.y = vertPositions[j].y;
      myVerts[j].c = t.GetColor();
    }
    /*
    if (t.type == Tile::empty) {
      t.rockmask = MatchHeight(tp, myVerts, {Tile::water}, _tiles, t.height,true,{Tile::rock});
    } else if (t.type == Tile::rock) {
      t.rockmask = MatchHeight(tp, myVerts, {Tile::empty, Tile::water}, _tiles, t.height + wallheight);
      if (t.rockmask == 0) {
        for (int j = 0; j < 4; ++j) {
          myVerts[j].c = Tile::voidColour;
        }
      }
    } else {
      MatchHeight(tp, myVerts, {}, _tiles, t.height,false);
    }
     */
    t.rockmask = MatchHeight(t, tp, myVerts, _tiles, t.height);


    int a = 6;
    a = 5;
    a =4;
    t.inverted = ShouldBeInverted(t.rockmask);
    if (t.inverted) {
      myIndices = {0, 3, 2, 0, 1, 3};
    } else {
      myIndices = {0, 1, 2, 1, 3, 2};
    }
    // calculate Normals
    glm::vec3 n1 = glm::triangleNormal(myVerts[myIndices[0]].p, myVerts[myIndices[2]].p, myVerts[myIndices[1]].p);
    myVerts[myIndices[0]].n = n1;
    myVerts[myIndices[1]].n = n1;
    myVerts[myIndices[2]].n = n1;
    glm::vec3 n2 = glm::triangleNormal(myVerts[myIndices[3]].p, myVerts[myIndices[5]].p, myVerts[myIndices[4]].p);
    myVerts[myIndices[3]].n = n2;
    myVerts[myIndices[4]].n = n2;
    myVerts[myIndices[5]].n = n2;

    // join the world
    allVerts.insert(allVerts.end(), myVerts.begin(), myVerts.end());
    auto ofsetIndices = std::transform(myIndices.begin(), myIndices.end(), std::back_inserter(allIndices), [&i](auto& c) { return c + (i * 4); });
  }
  for (auto& v : allVerts) {
    v.p.z = v.p.z * 0.25;
    // v.p.z = 0;
  }
  std::cout << "WamBam" << std::endl;
}

void Level::Render() {}

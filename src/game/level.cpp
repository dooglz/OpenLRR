#include "level.h"
using namespace Game;

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
  Triangulate(_tiles, _verts, _inidces);
  PrintMap(_tiles, true);
  std::cout << _verts.size() << " verts,  indices:" << _inidces.size() << std::endl;
}

Level::~Level() {}

void Level::Triangulate(std::array<Tile, levelSize * levelSize>& tiles, std::array<glm::vec3, nVerts>& verts,
                        std::array<uint16_t, indiceCount>& inidces) {

  // set heights
  for (int i = 0; i < tiles.size(); ++i) {

    Tile& t = tiles[i];
    idx tp = TilePos(i);
    std::array<idx, 4> averts = {getAsosiatedVerts(tp.x, tp.y)};
    VertAt(averts[1].x, averts[1].y).z = t.height;
    VertAt(averts[2].x, averts[2].y).z = t.height;
    if (t.type == Tile::rock) {
      // rocky horror
      // anywhere between 1 and 4 of the tile verts should be at wall height
      // set to all wallheight first
      for (const auto& adjtAvert : averts) {
        VertAt(adjtAvert.x, adjtAvert.y).z = t.height + wallheight;
      }
      // any vert of that touches an empty should be floor height
      std::vector<idx> adj = tp.getSurTiles(levelSize);
      size_t sidemask = 0;
      // std::cout << i << " (" << adj.size() << ") ";
      for (int j = 0; j < adj.size(); ++j) {
        const idx& adjTidx = adj[j];
        const Tile& adjt = TileAt(adjTidx.x, adjTidx.y);
        if (adjt.type != Tile::rock) {
          // std::cout << tp.orientation(adjTidx) << ", " ;
          sidemask += tp.orientation(adjTidx);

          std::array<idx, 4> adjtAverts = {getAsosiatedVerts(adjTidx.x, adjTidx.y)};
          for (const auto& adjtAvert : adjtAverts) {
            for (const auto& avert : averts) {
              if (adjtAvert == avert) {
                VertAt(adjtAvert.x, adjtAvert.y).z = t.height;
              }
            }
          }
        }
      }
      t.rockmask = sidemask;
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
          t.inverted = true;
        }
      }
    }
  }
  verts[0].z = tiles[0].height + wallheight;
  ;
  verts[verts.size() - 1].z = tiles[tiles.size() - 1].height + wallheight;
  ;

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
      Tile& t = TileAt(i, j);
      const uint16_t ti[4] = {
          (uint16_t)(j + (i * (levelSize + 1))),
          (uint16_t)(j + (i * (levelSize + 1)) + 1),
          (uint16_t)(j + (i * (levelSize + 1)) + (levelSize + 1)),
          (uint16_t)(j + (i * (levelSize + 1)) + (levelSize + 2)),
      };
      if (t.inverted) {
        t.tileIndices = {ti[0], ti[3], ti[2], ti[0], ti[1], ti[3]};
      } else {
        t.tileIndices = {ti[0], ti[1], ti[2], ti[1], ti[3], ti[2]};
      }
      std::copy(t.tileIndices.begin(), t.tileIndices.end(), &inidces[ptr]);
      //
      ptr += 6;
    }
  }
}

void Level::Render() {}

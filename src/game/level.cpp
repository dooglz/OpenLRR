#include "level.h"
#include "../engine/graphics/vk/vulkan_internals.h"
using namespace Game;

bool ShouldBeInverted(size_t sidemask){
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
  Triangulate2( _verts, _inidces);
  PrintMap(_tiles, true);
  std::cout << _verts.size() << " verts,  indices:" << _inidces.size() << std::endl;
}

Level::~Level() {}

void Level::Triangulate2(std::vector<Vertex>& allVerts,
                         std::vector<uint16_t>& allIndices) {

  //std::vector<Vertex> allVerts;
  allVerts.clear();
  allVerts.reserve(_tiles.size()*4);
  //std::vector<uint16_t> allIndices;
  allIndices.clear();
  allIndices.reserve(_tiles.size()*6);

  for (int i = 0; i < _tiles.size(); ++i) {
    std::array<Vertex,4> myVerts;
    std::array<uint16_t,6> myIndices;
    Tile& t = _tiles[i];
    idx tp = TilePos(i,levelSize);
    std::array<idx, 4> vertPositions = {getAsosiatedVerts(tp.x, tp.y)};
    // 0 -- 1
    // |  / |
    // 2 -- 3
    for (int j = 0; j < 4; ++j) {
      myVerts[j].p.x = vertPositions[j].x;
      myVerts[j].p.y = vertPositions[j].y;


      if(j == 1 || j== 2){
        myVerts[j].p.z = t.height;
      }else if(j == 0){
      //as this is a per tile algo, need to set all our heights, rather than just 1&2
        if(tp.y>0){
          myVerts[j].p.z = TileAt(_tiles,tp.x,tp.y-1,levelSize).height;
        //  myVerts[j].p.z = 10;
        }else if(tp.x>0){
          myVerts[j].p.z = TileAt(_tiles,tp.x-1,tp.y,levelSize).height;
          //myVerts[j].p.z = -10;
        }else{
          myVerts[j].p.z = t.height;
        }
      }else{
        if(tp.y<levelSize){
          myVerts[j].p.z =TileAt(_tiles,tp.x,tp.y+1,levelSize).height;
        }else if(tp.x<levelSize){
          myVerts[j].p.z = TileAt(_tiles,tp.x+1,tp.y,levelSize).height;
        }else{
          myVerts[j].p.z = t.height;
        }
      }

      myVerts[j].c = t.GetColor();
    }
    if (t.type == Tile::rock) {
      // rocky horror
      // anywhere between 1 and 4 of the tile verts should be at wall height
      // set to all wall height first. if a vert touches T!rock, it goes to t.height
      for (auto& v : myVerts) {
        v.p.z = t.height + wallheight;
      }
      std::vector<idx> surTiles = tp.getSurTiles(levelSize);
      size_t sidemask = 0;
      for (int j = 0; j < surTiles.size(); ++j) {
        const idx& surTidx = surTiles[j];
        const Tile& surT = TileAt(_tiles,surTidx.x, surTidx.y,levelSize);
        if (surT.type != Tile::rock) {
          auto direction = tp.orientation(surTidx);
          sidemask += direction;
          if(direction == idx::OrBit::u || direction == idx::OrBit::ul || direction == idx::OrBit::l ){
            myVerts[0].p.z = surT.height;
          }
          if(direction == idx::OrBit::u || direction == idx::OrBit::ur || direction == idx::OrBit::r ){
            myVerts[1].p.z = surT.height;
          }
          if(direction == idx::OrBit::d || direction == idx::OrBit::dl || direction == idx::OrBit::l ){
            myVerts[2].p.z = surT.height;
          }
          if(direction == idx::OrBit::d || direction == idx::OrBit::dr || direction == idx::OrBit::r ){
            myVerts[3].p.z = surT.height;
          }
        }
      }
      t.rockmask = sidemask;
      t.inverted = ShouldBeInverted(sidemask);
    }//End RockyHorror
    if (t.inverted) {
      myIndices = {0,3,2,0,1,3};
    } else {
      myIndices = {0,1,2,1,3,2};
    }
    //join the world
    allVerts.insert(allVerts.end(), myVerts.begin(), myVerts.end());
    auto ofsetIndices = std::transform(myIndices.begin(), myIndices.end(), std::back_inserter(allIndices), [&i](auto& c){return c+(i*4);});
  //  allIndices.insert(allIndices.end(), ofsetIndices.begin(), ofsetIndices.end());
  }
  for(auto& v :allVerts){
    v.p.z = v.p.z * 0.25;
    //v.p.z = 0;
  }
  std::cout << "WamBam" << std::endl;
}

void Level::Triangulate(std::array<Tile, levelSize * levelSize>& tiles, std::array<glm::vec3, nVerts>& verts,
                        std::array<uint16_t, indiceCount>& inidces) {
//  Triangulate2();
/*


  // set heights
  for (int i = 0; i < tiles.size(); ++i) {

    Tile& t = tiles[i];
    idx tp = TilePos(i);
    std::array<idx, 4> averts = {getAsosiatedVerts(tp.x, tp.y)};
    VertAt(verts,averts[1].x, averts[1].y).z = t.height;
    VertAt(verts,averts[2].x, averts[2].y).z = t.height;
    if (t.type == Tile::rock) {

      // rocky horror
      // anywhere between 1 and 4 of the tile verts should be at wall height
      // set to all wallheight first
      for (const auto& adjtAvert : averts) {
        VertAt(verts,adjtAvert.x, adjtAvert.y).z = t.height + wallheight;
      }
      // any vert of that touches an empty should be floor height
      std::vector<idx> adj = tp.getSurTiles(levelSize);
      size_t sidemask = 0;
      // std::cout << i << " (" << adj.size() << ") ";
      for (int j = 0; j < adj.size(); ++j) {
        const idx& adjTidx = adj[j];
        const Tile& adjt = TileAt(tiles,adjTidx.x, adjTidx.y);
        if (adjt.type != Tile::rock) {
          // std::cout << tp.orientation(adjTidx) << ", " ;
          sidemask += tp.orientation(adjTidx);

          std::array<idx, 4> adjtAverts = {getAsosiatedVerts(adjTidx.x, adjTidx.y)};
          for (const auto& adjtAvert : adjtAverts) {
            for (const auto& avert : averts) {
              if (adjtAvert == avert) {
            //    VertAt(adjtAvert.x, adjtAvert.y).z = t.height;
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
  verts[verts.size() - 1].z = tiles[tiles.size() - 1].height + wallheight;


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
      //Tile& t = TileAt(i, j);
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
  */
}

void Level::Render() {}

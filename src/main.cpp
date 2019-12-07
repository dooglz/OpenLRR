#include "engine/Engine.h"
#include "game/game.h"
#include <iostream>
//#include <experimental/filesystem>

int main(int argc, const char* argv[]) {
  std::cout << "Hello world " << std::endl;
 // Game::StartUp();
  //return 0;
  Engine::Startup();
  Engine::CreateWindow(1280, 720);
  Engine::Go();
  // endloop
  Engine::Shutdown();
  return 0;
}

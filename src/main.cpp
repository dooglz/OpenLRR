#include "engine/Engine.h"
#include "game/game.h"
#include <iostream>
//#include <experimental/filesystem>

int main(int argc, const char* argv[]) {
  std::cout << "Hello world " << std::endl;
  Engine::Startup();
  Engine::OpenWindow(1280, 720);
  //  Game::StartUp();
  Engine::Go();
  // endloop
  Engine::Shutdown();
  return 0;
}

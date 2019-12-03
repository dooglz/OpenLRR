#include "engine/Engine.h"
#include <iostream>

int main(int argc, const char* argv[]) {
  std::cout << "Hello world " << std::endl;
  Engine::Startup();
  Engine::CreateWindow(1280, 720);
  Engine::Go();
  // endloop
  Engine::Shutdown();
  return 0;
}

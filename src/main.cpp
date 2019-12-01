#include <iostream>
#include "engine/Engine.h"

int main(int argc, const char * argv[]) {
    std::cout << "Hello world " << std::endl;
    Engine::Startup();
    Engine::CreateWindow(1280,720);
    Engine::Go();

    return 0;
}
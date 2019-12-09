#include "game.h"
#include "level.h"


void Game::StartUp() { 
	auto level = GenerateLevel();
}

void Game::Tick(double dt) {}

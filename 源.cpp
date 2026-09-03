#include <iostream>
#include "ChessGame.h"

int main(void) {
	Chess chess;
	Man man;
	AI ai;
	ChessGame game(&man, &ai, &chess);

	game.play();

	return 0;
}
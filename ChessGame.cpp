#include "ChessGame.h"


void ChessGame::play()
{
	chess->init();//初始化棋盘
	while (1) {
		man->go();//棋手走，检查是否有一方获胜
		if (chess->checkOver()) {
			chess->init();;
			continue;//本次对局结束
		}

		ai->go();//AI走
		if (chess->checkOver()) {
			chess->init();
			continue;
		}
	}
}

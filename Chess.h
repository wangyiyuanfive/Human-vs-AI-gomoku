#pragma once
#include <graphics.h>//easyx图形库的头文件
#include <vector>
using namespace std;

//棋的颜色
typedef enum {
	CHESS_WHITE = -1,  // 白方
	CHESS_BLACK = 1    // 黑方
} chess_kind_t;

//棋的位置
struct ChessPos {
	int row;
	int col;
};


class Chess
{
public:
	// 棋盘的初始化：加载棋盘的图片资源，初始化棋盘的相关数据
	void init();

	// 判断在指定坐标(x,y)位置，是否是有效点击
	// 如果是有效点击，把有效点击的位置(行，列）保存在参数pos中
	bool clickBoard(int x, int y, ChessPos* pos);

	// 在棋盘的指定位置（pos）, 落子（kind）
	void chessDown(ChessPos* pos, chess_kind_t kind);

	// 获取棋盘的大小（13线、15线、19线）
	int getGradeSize();

	// 获取指定位置是黑棋，还是白棋，还是空白
	int getChessData(ChessPos* pos);
	int getChessData(int row, int col);

	// 判断棋局是否结束
	bool checkOver();

//使用图形库显示棋盘棋子
private:
	// 棋盘尺寸
	int gradeSize;//棋盘大小（13，15，17，19）
	int margin_x;//49;棋盘的左边界
	int margin_y;// 49;棋盘的上边界
	float chessSize; //棋子大小（棋盘方格大小）

	IMAGE chessBlackImg;//黑棋
	IMAGE chessWhiteImg;//白棋

	// 存储当前游戏棋盘和棋子的情况,空白为0，黑子1，白子-1
	vector<vector<int>> chessMap;

	// 标示下棋方, true:黑棋方  false: AI 白棋方
	bool playerFlag;
};
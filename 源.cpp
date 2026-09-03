
// 禁用特定警告信息
#pragma warning(disable:6031)  // 禁用"忽略返回值"警告
#pragma warning(disable:6054)  // 禁用"不兼容的指针类型转换"警告
#pragma warning(disable:6385)  // 禁用"从数组读取无效数据"警告
// 允许使用不安全的CRT函数
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#include <windows.h>
#include <easyx.h>
#include <graphics.h>
#include <time.h> // 添加time.h头文件

// 常量定义
#define WWidth					640
#define WHeight					750
#define GHeight					640
#define BWidth					140
#define PieceRadius				18
#define FlushCount				10

// 链表节点结构体
struct Node
{
	int x;
	int y;
	int P;
	struct Node* LAST;
	struct Node* NEXT;
};

// 全局变量声明
int ret = 0;
int Map[15][15];
int Position = 0; // 0表示未开始，1:白棋, -1:黑棋
int PX, PY;
char RecvBuffer[256];
char SendBufferX[256];
char SendBufferY[256];
BOOL GameOver = FALSE;
BOOL ISLAUNCH = FALSE;
Node* HEAD, * _END;

// 高精度计时相关变量
clock_t startTime;   // 游戏开始时间
clock_t endTime;     // 游戏结束时间
BOOL timerStarted = FALSE; // 计时器是否已启动

/**
 * 初始化游戏模式选择界面
 * @return 选择单人模式返回1，双人模式返回2
 */
int InitGameModeMenu()
{
	int choice = 0;
	while (1)
	{
		system("cls");
		printf("===========================\n");
		printf("    五子棋游戏模式选择\n");
		printf("===========================\n");
		printf("        1. 单人对战\n");
		printf("        2. 双人对战\n");
		printf("===========================\n");
		printf("请选择模式(输入1或2): ");

		scanf("%d", &choice);

		if (choice == 1 || choice == 2) {
			return choice;
		}

		printf("\n无效选择！请重新输入。\n");
		Sleep(1000);
	}
}

/**
 * 初始化棋盘
 * @return 初始化成功返回TRUE，失败返回FALSE
 */
BOOL InitCheckerBoard()
{
	// 重置计时器状态
	timerStarted = FALSE;

	for (int i = 0; i < 15; i++)
		for (int j = 0; j < 15; j++)
			Map[i][j] = 0;

	HEAD = (Node*)malloc(sizeof(Node));
	_END = (Node*)malloc(sizeof(Node));
	if (HEAD == NULL || _END == NULL)
		return FALSE;

	HEAD->NEXT = _END;
	_END->LAST = HEAD;
	return TRUE;
}

/**
 * 向链表添加新的落子节点
 * @param x 落子的x坐标(行)
 * @param y 落子的y坐标(列)
 * @param p 棋子类型(1:白棋, -1:黑棋)
 * @return 添加成功返回TRUE，失败返回FALSE
 */
BOOL AddNewNode(int x, int y, int p)
{
	// 如果是第一步，启动计时器
	if (!timerStarted) {
		startTime = clock();
		timerStarted = TRUE;
	}

	Node* P = _END->LAST, * Q = _END;
	Node* N = (Node*)malloc(sizeof(Node));
	if (N == NULL)
		return FALSE;

	N->x = x;
	N->y = y;
	N->P = p;
	N->LAST = P;
	N->NEXT = Q;
	P->NEXT = N;
	Q->LAST = N;
	return TRUE;
}

/**
 * 获取游戏已用时间（秒）
 * @return 游戏已用时间（秒）
 */
double GetElapsedTime()
{
	if (!timerStarted) return 0.0;

	clock_t current;
	if (GameOver) {
		current = endTime;
	}
	else {
		current = clock();
	}

	return (double)(current - startTime) / CLOCKS_PER_SEC;
}

/**
 * 格式化时间显示
 * @param seconds 总秒数
 * @param buffer 输出缓冲区
 */
void FormatTimeString(double seconds, char* buffer)
{
	int totalSeconds = (int)seconds;
	int minutes = totalSeconds / 60;
	int remainingSeconds = totalSeconds % 60;
	sprintf(buffer, "%d分%d秒", minutes, remainingSeconds);
}

/**
 * 绘制棋盘和棋子
 */
void DrawBoardAndPiece()
{
	clearrectangle(0, 0, WWidth, GHeight);
	setfillcolor(RGB(105, 105, 105));
	fillrectangle(0, 0, WWidth, GHeight);

	for (int i = 0; i < 15; i++)
	{
		line(40 + 40 * i, 40, 40 + 40 * i, GHeight - 40);
		line(40, 40 + 40 * i, WWidth - 40, 40 + 40 * i);
	}

	setfillcolor(WHITE);
	fillcircle(160, 160, 5);
	fillcircle(480, 160, 5);
	fillcircle(320, 320, 5);
	fillcircle(160, 480, 5);
	fillcircle(480, 480, 5);

	for (int j = 0; j < 15; j++)
		for (int k = 0; k < 15; k++)
			if (Map[j][k] == 1)
			{
				setfillcolor(WHITE);
				fillcircle(40 + 40 * k, 40 + 40 * j, PieceRadius);
			}
			else if (Map[j][k] == -1)
			{
				setfillcolor(BLACK);
				fillcircle(40 + 40 * k, 40 + 40 * j, PieceRadius);
			}
}

/**
 * 处理用户输入(鼠标点击)
 * @param MX 鼠标X坐标
 * @param MY 鼠标Y坐标
 * @param UMSG 鼠标消息类型
 * @return 处理成功返回TRUE，失败返回FALSE
 */
BOOL GetInput(int MX, int MY, UINT UMSG)
{
	if (MX < 20 || MX > WWidth || MY < 20 || MY > GHeight)
		return FALSE;

	if (UMSG == WM_LBUTTONDOWN)
	{
		int x = (MX - 20) / 40;
		int y = (MY - 20) / 40;
		if (x < 0 || y < 0)
			return FALSE;

		if (Map[y][x] == 0)
		{
			Map[y][x] = Position;
			PX = y;
			PY = x;
			sprintf(SendBufferX, "%d", y);
			sprintf(SendBufferY, "%d", x);
			AddNewNode(y, x, Position);
			ISLAUNCH = TRUE;
			return TRUE;
		}
	}
	return FALSE;
}

/**
 * 绘制系统信息(状态提示)
 */
void DrawSystemInfo()
{
	clearrectangle(BWidth + 1, GHeight + 1, WWidth, WHeight);
	settextstyle(20, 0, "楷体");

	// 添加时间显示
	char timeBuffer[50];
	double elapsed = GetElapsedTime();
	FormatTimeString(elapsed, timeBuffer);

	char fullInfo[256];
	sprintf(fullInfo, "%s | 用时: %s", RecvBuffer, timeBuffer);

	outtextxy(BWidth + 10, GHeight + 45, fullInfo);
}

/**
 * 绘制按钮并处理按钮点击事件
 * @param MX 鼠标X坐标
 * @param MY 鼠标Y坐标
 * @param UMSG 鼠标消息类型
 * @return 点击"回放"返回1，点击"退出"返回-1，否则返回0
 */
int DrawButton(int MX, int MY, UINT UMSG)
{
	settextstyle(30, 0, "楷体");
	settextcolor(WHITE);
	outtextxy(30, GHeight + 20, "回放");
	outtextxy(30, GHeight + 60, "退出");

	if (MX > 30 && MX < 90 && MY > GHeight + 20 && MY < GHeight + 50)
	{
		settextcolor(RED);
		outtextxy(30, GHeight + 20, "回放");
		if (UMSG == WM_LBUTTONDOWN)
		{
			return 1;
		}
	}
	else if (MX > 30 && MX < 90 && MY > GHeight + 60 && MY < GHeight + 90)
	{
		settextcolor(RED);
		outtextxy(30, GHeight + 60, "退出");
		if (UMSG == WM_LBUTTONDOWN)
		{
			return -1;
		}
	}
	return 0;
}

/**
 * 检查游戏是否结束(判断是否有五子连珠)
 * @return 游戏结束返回TRUE，否则返回FALSE
 */
BOOL CaculateResult()
{
	for (int i = 2; i <= 12; i++)
		for (int j = 2; j <= 12; j++)
		{
			if (Map[i][j] != 0 && Map[i][j] == Map[i][j - 1] && Map[i][j] == Map[i][j - 2] && Map[i][j] == Map[i][j + 1] && Map[i][j] == Map[i][j + 2])
			{
				GameOver = TRUE;
				endTime = clock(); // 记录结束时间
				return TRUE;
			}
			if (Map[i][j] != 0 && Map[i][j] == Map[i - 1][j] && Map[i][j] == Map[i - 2][j] && Map[i][j] == Map[i + 1][j] && Map[i][j] == Map[i + 2][j])
			{
				GameOver = TRUE;
				endTime = clock();
				return TRUE;
			}
			if (Map[i][j] != 0 && Map[i][j] == Map[i - 1][j - 1] && Map[i][j] == Map[i - 2][j - 2] && Map[i][j] == Map[i + 1][j + 1] && Map[i][j] == Map[i + 2][j + 2])
			{
				GameOver = TRUE;
				endTime = clock();
				return TRUE;
			}
			if (Map[i][j] != 0 && Map[i][j] == Map[i + 1][j - 1] && Map[i][j] == Map[i + 2][j - 2] && Map[i][j] == Map[i - 1][j + 1] && Map[i][j] == Map[i - 2][j + 2])
			{
				GameOver = TRUE;
				endTime = clock();
				return TRUE;
			}
		}
	return FALSE;
}

/**
 * 游戏回放功能
 */
void RePlay()
{
	clearrectangle(0, 0, WWidth, GHeight);
	setfillcolor(RGB(105, 105, 105));
	fillrectangle(0, 0, WWidth, GHeight);
	for (int i = 0; i < 15; i++)
	{
		line(40 + 40 * i, 40, 40 + 40 * i, GHeight - 40);
		line(40, 40 + 40 * i, WWidth - 40, 40 + 40 * i);
	}

	Node* N = HEAD->NEXT;
	while (N != _END)
	{
		if (N->P == 1)
		{
			setfillcolor(WHITE);
			fillcircle(40 + 40 * N->y, 40 + 40 * N->x, PieceRadius);
		}
		else
		{
			setfillcolor(BLACK);
			fillcircle(40 + 40 * N->y, 40 + 40 * N->x, PieceRadius);
		}
		FlushBatchDraw();
		Sleep(800);
		N = N->NEXT;
	}
}

/**
 * 评估指定位置的价值
 * @param x 行坐标
 * @param y 列坐标
 * @param player 玩家类型(1:白棋, -1:黑棋)
 * @return 位置评分
 */
int EvaluatePosition(int x, int y, int player)
{
	int score = 0;
	int dx[8] = { 1, 0, 1, 1, -1, 0, -1, -1 };
	int dy[8] = { 0, 1, 1, -1, 0, -1, -1, 1 };

	for (int dir = 0; dir < 8; dir += 2)
	{
		int count = 1;
		int empty = 0;
		BOOL blocked = FALSE;

		for (int i = 1; i <= 4; i++)
		{
			int nx = x + dx[dir] * i;
			int ny = y + dy[dir] * i;

			if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15)
			{
				blocked = TRUE;
				break;
			}

			if (Map[nx][ny] == player)
			{
				count++;
			}
			else if (Map[nx][ny] == 0)
			{
				empty++;
				break;
			}
			else
			{
				blocked = TRUE;
				break;
			}
		}

		for (int i = 1; i <= 4; i++)
		{
			int nx = x + dx[dir + 1] * i;
			int ny = y + dy[dir + 1] * i;

			if (nx < 0 || nx >= 15 || ny < 0 || ny >= 15)
			{
				blocked = TRUE;
				break;
			}

			if (Map[nx][ny] == player)
			{
				count++;
			}
			else if (Map[nx][ny] == 0)
			{
				empty++;
				break;
			}
			else
			{
				blocked = TRUE;
				break;
			}
		}

		if (count >= 5)
		{
			score += 100000;
		}
		else if (count == 4)
		{
			if (empty == 2) score += 10000;
			else if (empty == 1) score += 1000;
		}
		else if (count == 3)
		{
			if (empty == 2) score += 1000;
			else if (empty == 1) score += 100;
		}
		else if (count == 2)
		{
			if (empty == 2) score += 100;
			else if (empty == 1) score += 10;
		}
	}

	return score;
}

/**
 * 主函数
 */
int main()
{
	if (!InitCheckerBoard()) {
		printf("棋盘初始化失败！");
		return -1;
	}

	int mode = InitGameModeMenu();

	if (mode == 1) {
		// AI对战模式
		initgraph(WWidth, WHeight);
		BeginBatchDraw();
		Position = -1; // 玩家执黑先行

		// 初始提示信息
		strcpy(RecvBuffer, "游戏开始，您执黑先行");
		DrawBoardAndPiece();
		DrawSystemInfo();
		FlushBatchDraw();

		// 记录上次更新时间
		DWORD lastUpdateTime = GetTickCount();

		// 游戏主循环
		while (1) {
			DWORD currentTime = GetTickCount();

			// 每秒更新一次时间显示
			if (currentTime - lastUpdateTime >= 1000) {
				DrawSystemInfo();
				FlushBatchDraw();
				lastUpdateTime = currentTime;
			}

			// 检查游戏是否结束
			if (CaculateResult()) {
				// 获取总用时
				double totalSeconds = GetElapsedTime();
				int totalIntSeconds = (int)totalSeconds;
				int minutes = totalIntSeconds / 60;
				int seconds = totalIntSeconds % 60;

				// 根据获胜方设置提示信息
				if (Position == 1) {
					sprintf(RecvBuffer, "恭喜你获胜了! 用时: %d分%d秒", minutes, seconds);
				}
				else {
					sprintf(RecvBuffer, "AI获胜! 用时: %d分%d秒", minutes, seconds);
				}

				DrawSystemInfo();
				FlushBatchDraw();
				MessageBox(GetHWnd(), RecvBuffer, "游戏结束", MB_OK);
				break;
			}

			// 处理鼠标消息
			if (MouseHit()) {
				MOUSEMSG msg = GetMouseMsg();
				int buttonRet = DrawButton(msg.x, msg.y, msg.uMsg);

				// 处理按钮事件
				if (buttonRet == -1) break;
				if (buttonRet == 1) RePlay();

				// 玩家回合
				if (Position == -1) {
					if (GetInput(msg.x, msg.y, msg.uMsg)) {
						DrawBoardAndPiece();
						FlushBatchDraw();

						Position = 1; // 轮到AI
						strcpy(RecvBuffer, "AI思考中...");
					}
				}
			}

			// AI回合
			if (Position == 1) {
				Sleep(800); // 模拟AI思考时间

				// AI评分矩阵
				int scoreMap[15][15] = { 0 };
				int maxScore = 0;
				int bestX = -1, bestY = -1;

				// 1. 计算每个位置的评分
				for (int i = 0; i < 15; i++) {
					for (int j = 0; j < 15; j++) {
						if (Map[i][j] == 0) {
							int score = 0;
							score += EvaluatePosition(i, j, 1);
							score += EvaluatePosition(i, j, -1) * 0.8;
							scoreMap[i][j] = score;

							if (score > maxScore) {
								maxScore = score;
								bestX = i;
								bestY = j;
							}
						}
					}
				}

				// 2. AI落子
				if (bestX != -1 && bestY != -1) {
					Map[bestX][bestY] = 1;
					AddNewNode(bestX, bestY, 1);

					setfillcolor(WHITE);
					fillcircle(40 + 40 * bestY, 40 + 40 * bestX, PieceRadius);

					Position = -1; // 轮到玩家
					strcpy(RecvBuffer, "请您落子");
				}

				// 更新显示
				DrawSystemInfo();
				FlushBatchDraw();
				lastUpdateTime = GetTickCount(); // 重置更新时间
			}

			Sleep(10);
		}

		EndBatchDraw();
		closegraph();
	}

	else {
		//双人联网//

		// 1. 初始化Windows Socket协议版本
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
		{
			printf("确定协议版本失败!\n");
			return -1;
		}
		printf("确定协议版本成功!\n");

		// 2. 创建Socket
		SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (SOCKET_ERROR == serverSocket)
		{
			printf("创建socket失败:%d\n", GetLastError());
			// 清理资源
			WSACleanup();
			return -1;
		}
		printf("创建socket成功!\n");

		// 3. 配置服务器地址信息
		SOCKADDR_IN addr = { 0 };
		addr.sin_family = AF_INET;                 // 使用IPv4协议
		addr.sin_addr.S_un.S_addr = inet_addr("10.81.41.149"); // 服务器IP地址
		addr.sin_port = htons(9527);               // 服务器端口号

		// 4. 连接到服务器
		ret = connect(serverSocket, (sockaddr*)&addr, sizeof addr);
		if (ret == -1)
		{
			printf("连接服务器失败:%d\n", GetLastError());
			// 清理资源
			closesocket(serverSocket);
			WSACleanup();
			return -1;
		}
		printf("连接服务器成功!\n");

		// 接收服务器消息，确定自己是黑方还是白方
		ret = recv(serverSocket, RecvBuffer, 255, NULL);
		if (ret == 0)
			return -1;
		RecvBuffer[ret] = '\0';

		// 默认设置为白方(先手)
		Position = 1;
		ISLAUNCH = FALSE; // 先手玩家初始状态为未落子

		// 如果服务器返回-1，则表示自己是黑方(后手)
		if (strcmp(RecvBuffer, "-1") == 0)
		{
			Position = -1;
			ISLAUNCH = TRUE; // 后手玩家初始状态为已落子(等待对方)
		}

		// 接收服务器的下一步消息
		ret = recv(serverSocket, RecvBuffer, 255, NULL);
		if (ret == 0)
			return -1;
		RecvBuffer[ret] = '\0';
		BOOL ISPOST = TRUE;
		if (Position == 1)
			ISPOST = FALSE; // 先手玩家不需要先落子

		// 初始化棋盘
		if (!InitCheckerBoard())
			return -1;

		// 初始化图形窗口
		HWND hwnd = initgraph(WWidth, WHeight, 0);
		SetWindowTextA(hwnd, "五子棋");
		setlinecolor(WHITE);
		MOUSEMSG m = { 0 };
		BeginBatchDraw(); // 开始批量绘图，提高效率

		// 记录上次更新时间
		DWORD lastUpdateTime = GetTickCount();

		// 游戏主循环
		while (!GameOver)
		{
			DWORD currentTime = GetTickCount();

			// 每秒更新一次时间显示
			if (currentTime - lastUpdateTime >= 1000) {
				DrawSystemInfo();
				FlushBatchDraw();
				lastUpdateTime = currentTime;
			}

			// 检查是否有鼠标消息
			if (MouseHit())
				m = GetMouseMsg();

			// 绘制棋盘和棋子
			DrawBoardAndPiece();
			// 绘制系统信息
			DrawSystemInfo();
			// 刷新绘图
			FlushBatchDraw();

			// 如果是后手玩家，先接收对方落子
			if (!ISPOST)
			{
				ret = recv(serverSocket, RecvBuffer, 255, NULL); // 获取服务器发来的信息
				if (ret == 0)
					return -1;
				RecvBuffer[ret] = '\0';
				DrawSystemInfo();
				FlushBatchDraw();
				ISPOST = TRUE;
			}

			// 如果自己还未落子，处理鼠标输入
			if (!ISLAUNCH && GetInput(m.x, m.y, m.uMsg))
			{
				DrawBoardAndPiece();
				FlushBatchDraw();

				// 检查是否获胜
				if (CaculateResult())
				{
					// 格式化胜利消息并发送给服务器
					sprintf(RecvBuffer, "%d-%d-GameOver+", PX, PY);
					send(serverSocket, RecvBuffer, 255, NULL);
					sprintf(RecvBuffer, "己方获胜！");
				}
				else
				{
					// 格式化继续游戏消息并发送给服务器
					sprintf(RecvBuffer, "%d-%d-Continue+", PX, PY);
					send(serverSocket, RecvBuffer, 255, NULL);
					sprintf(RecvBuffer, "轮到对方落子");
				}

				DrawSystemInfo();
				FlushBatchDraw();
			}

			// 如果自己已落子且游戏未结束，等待对方落子
			if (ISLAUNCH && !GameOver)
			{
				// 接收对方落子信息
				ret = recv(serverSocket, RecvBuffer, 255, NULL);
				if (ret == 0)
					return -1;
				RecvBuffer[ret] = '\0';

				// 解析接收的消息格式(X-Y-状态+)
				int num = 0;
				char result[10];
				for (; num < ret; num++)
					if (RecvBuffer[num] != '-')
					{
						SendBufferX[num] = RecvBuffer[num];
					}
					else
					{
						SendBufferX[num] = '\0';
						break;
					}
				int numL = num + 1;
				for (num = num + 1; num < ret; num++)
					if (RecvBuffer[num] != '-')
					{
						SendBufferY[num - numL] = RecvBuffer[num];
					}
					else
					{
						SendBufferY[num - numL] = '\0';
						break;
					}
				numL = num + 1;
				for (num = num + 1; num < ret; num++)
					if (RecvBuffer[num] != '+')
					{
						result[num - numL] = RecvBuffer[num];
					}
					else
					{
						result[num - numL] = '\0';
						break;
					}

				// 将字符串转换为坐标
				int x = atoi(SendBufferX);
				int y = atoi(SendBufferY);
				// 在棋盘上记录对方落子
				Map[x][y] = (-1) * Position;
				// 添加到落子历史
				AddNewNode(x, y, (-1) * Position);

				// 检查对方是否获胜
				if (strcmp(result, "GameOver") == 0)
				{
					GameOver = TRUE;
					endTime = clock(); // 记录结束时间
					sprintf(RecvBuffer, "游戏结束。对方获胜！");
				}
				else
					sprintf(RecvBuffer, "轮到己方落子");

				// 刷新显示
				DrawBoardAndPiece();
				DrawSystemInfo();
				FlushBatchDraw();
				ISLAUNCH = FALSE; // 标记自己可以落子
			}

			// 刷新显示
			DrawBoardAndPiece();
			DrawSystemInfo();
			FlushBatchDraw();
		}

		// 游戏结束循环，处理回放和退出
		while (GameOver)
		{
			if (MouseHit())
				m = GetMouseMsg();
			ret = DrawButton(m.x, m.y, m.uMsg);
			if (ret == 1)
				RePlay(); // 执行回放功能
			else if (ret == -1)
				break; // 退出游戏
			FlushBatchDraw();
		}

		// 清理资源
		closesocket(serverSocket); // 关闭Socket
		WSACleanup();             // 清理Socket环境
	}
	return 0;
}

#pragma warning(disable:6031)
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)
#include <windows.h>

SOCKADDR_IN cAddr = { 0 };
int len = sizeof cAddr;
SOCKET clientSocket[2];

void func(int index)
{
	//7 通信
	char buff[1024];
	int r;
	while (1)
	{
		r = recv(clientSocket[index], buff, 1023, NULL);
		if (r > 0)
		{
			buff[r] = 0;
			//printf("%s发来的数据:%s\n", inet_ntop(AF_INET,), buff);
			printf("%s发来的数据:%s\n", inet_ntoa(cAddr.sin_addr), buff);
		}
	}
}

int main() {
	//1 确定协议版本
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("确定协议版本失败!\n");
		return -1;
	}
	printf("确定协议版本成功!\n");

	//2 创建socket
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SOCKET_ERROR == serverSocket)
	{
		printf("创建socket失败:%d\n", GetLastError());
		//9 清理协议版本信息
		WSACleanup();
		return -1;
	}
	printf("创建socket成功!\n");

	//3 创建服务器协议地址簇
	SOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;//协议地址族
	addr.sin_addr.S_un.S_addr = inet_addr("10.81.41.149");//这里改成服务器IP地址
	addr.sin_port = htons(9527);//10000左右    小端转大端

	//4 绑定
	int r = bind(serverSocket, (struct sockaddr*)&addr, sizeof addr);
	if (r == -1)
	{
		printf("绑定失败:%d\n", GetLastError());
		//8 关闭socket
		closesocket(serverSocket);
		//9 清理协议版本信息
		WSACleanup();
		return -1;
	}
	printf("绑定成功!\n");

	//5 监听
	r = listen(serverSocket, 10);
	if (r == -1)
	{
		printf("监听失败:%d\n", GetLastError());
		//8 关闭socket
		closesocket(serverSocket);
		//9 清理协议版本信息
		WSACleanup();
		return -1;
	}
	printf("监听成功!\n");

	//6 等待客户端连接
	while (1)
	{
		system("CLS");
		char SendBuffer[256];
		printf("等待客户端连接...\n");
		clientSocket[0] = accept(serverSocket, (sockaddr*)&cAddr, &len);
		sprintf(SendBuffer, "%s", "1");
		send(clientSocket[0], SendBuffer, strlen(SendBuffer), NULL);//发送给客户端1
		sprintf(SendBuffer, "%s", "成功连接服务器！等待其它玩家加入");
		Sleep(200);
		send(clientSocket[0], SendBuffer, strlen(SendBuffer), NULL);//发送给客户端1
		printf("客户端1--%s--已连接\n", inet_ntoa(cAddr.sin_addr));
		clientSocket[1] = accept(serverSocket, (sockaddr*)&cAddr, &len);
		sprintf(SendBuffer, "%s", "-1");
		send(clientSocket[1], SendBuffer, strlen(SendBuffer), NULL);//发送给客户端2
		Sleep(200);
		sprintf(SendBuffer, "%s", "成功连接服务器！游戏开始。对方先手");
		send(clientSocket[1], SendBuffer, strlen(SendBuffer), NULL);//发送给客户端2
		Sleep(200);
		sprintf(SendBuffer, "%s", "游戏开始。己方先手");
		send(clientSocket[0], SendBuffer, strlen(SendBuffer), NULL);//发送给客户端2
		printf("客户端2--%s--已连接\n", inet_ntoa(cAddr.sin_addr));
		while (1)
		{	//这里是收发白子信息
			r = recv(clientSocket[0], SendBuffer, 255, NULL);//获取白子传来的棋子X信息
			if (r == 0)
				break;
			SendBuffer[r] = '\0';
			send(clientSocket[1], SendBuffer, 255, NULL);//转发给黑子


			//这里是收发黑子信息
			r = recv(clientSocket[1], SendBuffer, 255, NULL);//获取白子传来的棋子X信息
			if (r == 0)
				break;
			SendBuffer[r] = '\0';
			send(clientSocket[0], SendBuffer, 255, NULL);//转发给黑子
		}
	}

	//8 关闭socket
	closesocket(serverSocket);
	//9 清理协议版本信息
	WSACleanup();
	printf("服务器关闭！\n");
	return 0;
}
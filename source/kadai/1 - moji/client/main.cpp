/*
	コンソールウィンドウで"256"などの数字(数字は10000未満の整数"unsigned int")もしくは"*"や"#"などの記号を入力
	入力された文字列をサーバー側に送る
*/

#include <stdio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
//#pragma comment( lib, "ws2_32.lib" )
//#include "windows.h"
#include <tchar.h>

int main() {
	WSADATA wsaData;
	struct sockaddr_in server;
	SOCKET socketA;

	// winsock2の初期化
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		return -1;
	}

	// ソケットの作成
	socketA = socket(AF_INET, SOCK_STREAM, 0);
	if (socketA == INVALID_SOCKET) {
		printf("error_socket: %d\n", WSAGetLastError());
		return -1;
	}

	// 接続先指定用構造体の準備
	server.sin_family = AF_INET;
	server.sin_port = htons(12345);
	InetPton(server.sin_family, _T("127.0.0.1"), &server.sin_addr.S_un.S_addr);

	// サーバに接続
	connect(socketA, (struct sockaddr*)&server, sizeof(server));

	char buffer[100];
	memset(buffer, 0, sizeof(buffer));
	int n = recv(socketA, buffer, sizeof(buffer), 0);
	if (n < 1) {
		printf("error_recv: %d\n", WSAGetLastError());
	}
	printf("%s\n", buffer);

	// 文字入力
	scanf_s("%s", buffer, 32);

	// サーバー側へデータを送信
	n = send(socketA, buffer, 5, 0);
	if (n < 1) {
		printf("error_send: %d\n", WSAGetLastError());
		return -1;
	}

	closesocket(socketA);

	// winsock2の終了
	WSACleanup();

	return 0;
}
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
	SOCKET sock;
	char buf[32];
	int n;

	// 文字入力
	scanf_s("%s", buf, 32);

	// winsock2の初期化
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		return -1;
	}

	// ソケットの作成
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) {
		printf("error_socket: %d\n", WSAGetLastError());
		return -1;
	}

	// 接続先指定用構造体の準備
	server.sin_family = AF_INET;
	server.sin_port = htons(12345);
	InetPton(server.sin_family, _T("127.0.0.1"), &server.sin_addr.S_un.S_addr);

	// サーバに接続
	connect(sock, (struct sockaddr*)&server, sizeof(server));
	
	// サーバー側へデータを送信
	n = send(sock, buf, 5, 0);
	if(n < 1) {
		printf("error_send: %d\n", WSAGetLastError());
		return -1;
	}

	closesocket(sock);

	// winsock2の終了処理
	WSACleanup();

	return 0;
}
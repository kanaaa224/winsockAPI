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

	// winsock2の初期化
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	// ソケットの作成
	sock = socket(AF_INET, SOCK_STREAM, 0);

	// 接続先指定用構造体の準備
	server.sin_family = AF_INET;
	server.sin_port = htons(12345);
	InetPton(server.sin_family, _T("127.0.0.1"), &server.sin_addr.S_un.S_addr);

	// サーバに接続
	connect(sock, (struct sockaddr*)&server, sizeof(server));

	// サーバからデータを受信
	memset(buf, 0, sizeof(buf));
	int n = recv(sock, buf, sizeof(buf), 0);

	printf("%d, %s\n", n, buf);

	// winsock2の終了
	WSACleanup();

	return 0;
}

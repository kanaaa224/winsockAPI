/*
	acceptした相手を表示するサーバー（エラー処理付）
*/

#include <stdio.h>
#include <winsock2.h>
#include <Ws2tcpip.h>

int main() {
	WSADATA wsaData;
	SOCKET sock0;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;
	SOCKET sock;
	char s[sizeof "255.255.255.255"];
	int n;

	// winsock2の初期化
	if(WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		return -1;
	}

	// ソケットの作成
	sock0 = socket(AF_INET, SOCK_STREAM, 0);
	if(sock0 == INVALID_SOCKET) {
		printf("error_socket: %d\n", WSAGetLastError());
		return -1;
	}

	// 接続先指定用構造体の準備
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	if(bind(sock0, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		printf("error_bind: %d\n", WSAGetLastError());
		return -1;
	}

	if(listen(sock0, 5) != 0) {
		printf("error_listen: %d\n", WSAGetLastError());
		return -1;
	}

	while (1) {
		len = sizeof(client);
		sock = accept(sock0, (struct sockaddr*)&client, &len);
		if(sock == INVALID_SOCKET) {
			printf("error_accept: %d\n", WSAGetLastError());
			break;
		}

		// 接続成功
		printf("accepted connection from %s, port=%d\n", inet_ntop(AF_INET, &client.sin_addr, s, sizeof s), ntohs(client.sin_port));

		n = send(sock, "HELLO", 5, 0);
		if(n < 1) {
			printf("error_send: %d\n", WSAGetLastError());
			break;
		}

		closesocket(sock);
	}

	// winsock2の終了処理
	WSACleanup();

	return 0;
}
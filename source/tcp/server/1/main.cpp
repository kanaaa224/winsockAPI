#include <winsock2.h>

int main() {
	WSADATA wsaData;
	SOCKET sock0;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;
	SOCKET sock;

	// winsock2の初期化
	WSAStartup(MAKEWORD(2, 0), &wsaData);

	// ソケットの作成
	sock0 = socket(AF_INET, SOCK_STREAM, 0);

	// ソケットの設定
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(sock0, (struct sockaddr*)&addr, sizeof(addr));

	// TCPクライアントからの接続要求を待てる状態にする
	listen(sock0, 5);

	// TCPクライアントからの接続要求を受け付ける
	len = sizeof(client);
	sock = accept(sock0, (struct sockaddr*)&client, &len);

	// 5文字送信
	send(sock, "HELLO", 5, 0);

	// TCPセッションの終了
	closesocket(sock);

	// winsock2の終了処理
	WSACleanup();

	return 0;
}
/*
	何回もクライアントと接続できるサーバー
*/

#include <winsock2.h>

int main() {
	WSADATA wsaData;
	SOCKET sock0;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;
	SOCKET sock;

	WSAStartup(MAKEWORD(2,0), &wsaData);

	sock0 = socket(AF_INET, SOCK_STREAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(sock0, (struct sockaddr *)&addr, sizeof(addr));

	listen(sock0, 5);

	while(1) {
		len = sizeof(client);
		sock = accept(sock0, (struct sockaddr *)&client, &len);
		send(sock, "HELLO", 5, 0);

		closesocket(sock);
	}

	WSACleanup();

	return 0;
}
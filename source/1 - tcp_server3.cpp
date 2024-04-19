#include <stdio.h>
#include <winsock2.h>

int main() {
	WSADATA wsaData;
	SOCKET sock0;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;
	SOCKET sock;
	int n;

	if(WSAStartup(MAKEWORD(2,0), &wsaData) != 0) return 1;

	sock0 = socket(AF_INET, SOCK_STREAM, 0);
	if(sock0 == INVALID_SOCKET) {
		printf("socket : %d\n", WSAGetLastError());
		return 1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	if(bind(sock0, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
		printf("bind : %d\n", WSAGetLastError());
		return 1;
	}

	if(listen(sock0, 5) != 0) {
		printf("listen : %d\n", WSAGetLastError());
		return 1;
	}

	while(1) {
		len = sizeof(client);
		sock = accept(sock0, (struct sockaddr *)&client, &len);
		if(sock == INVALID_SOCKET) {
			printf("accept : %d\n", WSAGetLastError());
			break;
		}

		n = send(sock, "HELLO", 5, 0);
		if(n < 1) {
			printf("send : %d\n", WSAGetLastError());
			break;
		}

		closesocket(sock);
	}

	WSACleanup();

	return 0;
}
#include <stdio.h>
#include <winsock2.h>

int main() {
	WSADATA wsaData;
	SOCKET sock0;
	struct sockaddr_in addr;
	struct sockaddr_in client;
	int len;
	SOCKET sock;
	BOOL yes = 1;

	char buf[2048];
	char inbuf[2048];

	WSAStartup(MAKEWORD(2, 0), &wsaData);

	sock0 = socket(AF_INET, SOCK_STREAM, 0);
	if (sock0 == INVALID_SOCKET) {
		printf("socket : %d\n", WSAGetLastError());
		return 1;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	setsockopt(sock0,
		SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

	if (bind(sock0, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		printf("bind : %d\n", WSAGetLastError());
		return 1;
	}

	if (listen(sock0, 5) != 0) {
		printf("listen : %d\n", WSAGetLastError());
		return 1;
	}

	// 応答用HTTPメッセージ作成
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf),
		"HTTP/1.0 200 OK\r\n"
		"Content-Length: 20\r\n"
		"Content-Type: text/html\r\n"
		"\r\n"
		"HELLO\r\n");

	while (1) {
		len = sizeof(client);
		sock = accept(sock0, (struct sockaddr*)&client, &len);
		if (sock == INVALID_SOCKET) {
			printf("accept : %d\n", WSAGetLastError());
			break;
		}

		memset(inbuf, 0, sizeof(inbuf));
		recv(sock, inbuf, sizeof(inbuf), 0);
		// 本来ならばクライアントからの要求内容をパースすべきです
		printf("%s", inbuf);

		// 相手が何を言おうとダミーHTTPメッセージ送信
		send(sock, buf, (int)strlen(buf), 0);

		closesocket(sock);
	}

	WSACleanup();

	return 0;
}
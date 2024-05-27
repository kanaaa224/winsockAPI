/*
	ヒットアンドブローゲーム - クライアント
*/

#include <stdio.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <tchar.h>

#include <winsock2.h>
#include <Ws2tcpip.h>

std::string getDateTime() {
	auto now       = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	// localtime_s を使用して tm 構造体に変換
	std::tm tm;
	localtime_s(&tm, &in_time_t);

	// フォーマット
	std::stringstream ss;
	ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");

	return ss.str();
}

int main() {
	WSADATA wsaData;
	struct sockaddr_in server;
	SOCKET socketA;

	// winsock2の初期化
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		std::cout << getDateTime() << " | [ winsock2 API ] error_wsaStartup: " << WSAGetLastError() << std::endl;
		return -1;
	}

	// ソケットの作成
	socketA = socket(AF_INET, SOCK_STREAM, 0);
	if (socketA == INVALID_SOCKET) {
		std::cout << getDateTime() << " | [ winsock2 API ] error_socket: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return -1;
	}

	// 接続先指定用構造体の準備
	server.sin_family = AF_INET;
	server.sin_port = htons(12345);
	if (InetPton(server.sin_family, _T("127.0.0.1"), &server.sin_addr.S_un.S_addr) <= 0) {
		std::cout << getDateTime() << " | [ winsock2 API ] error_inetPton: " << WSAGetLastError() << std::endl;
		closesocket(socketA);
		WSACleanup();
		return -1;
	}

	// サーバに接続
	if (connect(socketA, (struct sockaddr*)&server, sizeof(server)) < 0) {
		std::cout << getDateTime() << " | [ winsock2 API ] error_connect: " << WSAGetLastError() << std::endl;
		closesocket(socketA);
		WSACleanup();
		return -1;
	}

	std::string buffer(100, '\0');
	int n = recv(socketA, &buffer[0], buffer.size() - 1, 0);
	if (n < 1) {
		std::cerr << "error_recv: " << WSAGetLastError() << std::endl;
		closesocket(socketA);
		WSACleanup();
		return -1;
	}
	buffer.resize(n); // 実際に受信したサイズに調整
	std::cout << getDateTime() << " | [ winsock2 API ] connected to server - serverName: " << buffer << std::endl;

	while (true) {
		// 文字入力
		std::cout << "入力してください: ";
		std::string input;
		std::getline(std::cin, input);

		if (input == "exit") break;

		// サーバー側へデータを送信
		n = send(socketA, input.c_str(), input.length(), 0);
		if (n < 1) {
			std::cout << getDateTime() << " | [ winsock2 API ] error_send: " << WSAGetLastError() << std::endl;
			return -1;
		}
	}

	closesocket(socketA);

	// winsock2の終了
	WSACleanup();

	return 0;
}
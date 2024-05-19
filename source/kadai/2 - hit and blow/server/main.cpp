/*
	ヒットアンドブローゲーム - サーバー
	終了コード: "#"
*/

#include <stdio.h>
#include <iostream>
#include <regex>
#include <winsock2.h>
#include <Ws2tcpip.h>

// ヒットアンドブローゲームの出題値
std::vector<std::string> hitAndBlowValues;

// ヒットアンドブローゲームで値をチェック
//std::vector<int> hitAndBlowCheck() {
//	//
//}

// ヒットアンドブローゲームを開始
bool hitAndBlowGame(int length = 3) {
	hitAndBlowValues = { "1", "2", "3" };
	std::cout << "HitAndBlow の文字列: " << hitAndBlowValues[0] << hitAndBlowValues[1] << std::endl;
	return true;
}

int main() {
	WSADATA wsaData;
	SOCKET socketA;
	SOCKET socketB;
	struct sockaddr_in addr;
	struct sockaddr_in client;

	// winsock2の初期化
	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
		return -1;
	}

	// ソケットの作成
	socketA = socket(AF_INET, SOCK_STREAM, 0);
	if (socketA == INVALID_SOCKET) {
		std::cout << "error_socket: " << WSAGetLastError() << std::endl;
		return -1;
	}

	// 接続先指定用構造体の準備
	addr.sin_family = AF_INET;
	addr.sin_port = htons(12345);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	if (bind(socketA, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
		std::cout << "error_bind: " << WSAGetLastError() << std::endl;
		return -1;
	}

	if (listen(socketA, 5) != 0) {
		std::cout << "error_listen: " << WSAGetLastError() << std::endl;
		return -1;
	}

	// HitAndBlow ゲームを開始
	if (!hitAndBlowGame()) return false;

	while (1) {
		int length = sizeof(client);
		socketB = accept(socketA, (struct sockaddr*)&client, &length);
		if (socketB == INVALID_SOCKET) {
			std::cout << "error_accept: " << WSAGetLastError() << std::endl;
			break;
		}

		// 接続成功
		char buffer[] = "connected server: 3 - kadai_server.cpp";
		int n = send(socketB, buffer, sizeof(buffer), 0);
		if (n < 1) {
			std::cout << "error_send: " << WSAGetLastError() << std::endl;
			return -1;
		}
		char psb[sizeof "255.255.255.255"];
		std::cout << "accepted connection. from: " << inet_ntop(AF_INET, &client.sin_addr, psb, sizeof psb) << ", port: " << ntohs(client.sin_port) << std::endl;

		// クライアント側から受信したデータを表示
		memset(buffer, 0, sizeof(buffer));
		n = recv(socketB, buffer, sizeof(buffer), 0);
		if (n < 1) {
			std::cout << "error_recv: " << WSAGetLastError() << std::endl;
		}
		else {
			// 接続、受信成功

			/*if (buffer[0] == '#') {
				printf("文字列: 終了コード\n");
			}
			else {
				std::string str = buffer;

				std::regex pattern_symbol("^[!-/:-@\[-`{-~]+$");
				std::regex pattern_alpha("^[a-zA-Z]+$");
				std::regex pattern_digit("^[0-9]+$");

				std::smatch sm;

				if (std::regex_match(str, sm, pattern_symbol) && !std::regex_match(str, sm, pattern_alpha) && !std::regex_match(str, sm, pattern_digit)) {
					printf("文字列: 記号\n");
				}
				else if (std::regex_match(str, sm, pattern_digit) && !std::regex_match(str, sm, pattern_symbol) && !std::regex_match(str, sm, pattern_alpha)) {
					printf("文字列: 数値\n");
				}
				else if (std::regex_match(str, sm, pattern_alpha) && !std::regex_match(str, sm, pattern_symbol) && !std::regex_match(str, sm, pattern_digit)) {
					printf("文字列: アルファベット\n");
				}
				else {
					printf("文字列: その他（複数の文字種）\n");
				}
			}*/
		}

		closesocket(socketB);
	}

	// winsock2の終了
	WSACleanup();

	return 0;
}
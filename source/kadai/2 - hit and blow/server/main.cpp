/*
	ヒットアンドブローゲーム - サーバー
	終了コード: "#"
*/

#include <stdio.h>
#include <iostream>
#include <regex>
#include <random>
#include <unordered_set>
#include <chrono>
#include <iomanip>
#include <sstream>

#include <winsock2.h>
#include <Ws2tcpip.h>

#include "json.hpp"

// ヒットアンドブローゲームの出題値
std::vector<int> hitAndBlowValues;

// ヒットアンドブローゲームを開始
bool hitAndBlowGame(int length = 3) {
	if (length > 10) return false; // 桁数は10桁以内でなければならない

	std::random_device r{};
	std::mt19937 gen(r());
	std::uniform_int_distribution<int> dist(0, 9); // 0 - 9 の範囲で生成

	std::unordered_set<int> usedValues; // 使用済みの数字を追跡

	hitAndBlowValues.clear();
	hitAndBlowValues.reserve(length);

	while (hitAndBlowValues.size() < length) {
		int newValue = dist(gen);
		if (usedValues.find(newValue) == usedValues.end()) {
			hitAndBlowValues.push_back(newValue);
			usedValues.insert(newValue);
		}
	}

	return true;
}

// ヒットアンドブローゲームのプレイヤーの入力値をチェック
std::vector<int> hitAndBlowCheck(const std::vector<int>& playerValues) {
	int hits  = 0;
	int blows = 0;

	std::vector<bool> hitFlags (hitAndBlowValues.size(), false);
	std::vector<bool> blowFlags(hitAndBlowValues.size(), false);

	// Hit の計算 | プレイヤーの入力値と出題値を比較し、位置と値の両方が一致する場合にヒットとしてカウント
	for (int i = 0; i < hitAndBlowValues.size(); ++i) {
		if (playerValues[i] == hitAndBlowValues[i]) {
			hits++;
			hitFlags[i] = true;
		}
	}

	// Blow の計算 | ヒットではない位置で、プレイヤーの値が他の位置に存在するかをチェック
	for (int i = 0; i < hitAndBlowValues.size(); ++i) {
		if (!hitFlags[i]) {
			for (int j = 0; j < hitAndBlowValues.size(); ++j) {
				// 現在のプレイヤーの入力値が他の位置の出題値と一致し、ヒットもブローもまだ判定されていない場合にブローとしてカウント
				if (i != j && !hitFlags[j] && !blowFlags[j] && playerValues[i] == hitAndBlowValues[j]) {
					blows++;
					blowFlags[j] = true;
					break;
				}
			}
		}
	}

	return { hits, blows };
}

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
	SOCKET socketA;
	SOCKET socketB;
	struct sockaddr_in sockaddrA;
	struct sockaddr_in sockaddrB;

	int state = 1;

	std::cout << getDateTime()         << " | " << "ヒットアンドブローゲーム - サーバー" << std::endl;
	std::cout << "                   " << " | " << "サーバーへ接続してゲームを開始します。" << std::endl;

	while (state > 0) {
		switch (state) {
			
			// 起動ステート
			case 1: {
				// winsock2の初期化
				if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: wsaStartup - " << WSAGetLastError() << std::endl;
					return -1;
				}

				// ソケットの作成
				socketA = socket(AF_INET, SOCK_STREAM, 0);
				if (socketA == INVALID_SOCKET) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: socket - " << WSAGetLastError() << std::endl;
					WSACleanup();
					return -1;
				}

				// 接続先指定用構造体の準備
				sockaddrA.sin_family = AF_INET;
				sockaddrA.sin_port = htons(12345);
				sockaddrA.sin_addr.S_un.S_addr = INADDR_ANY;

				// ローカルアドレスをソケットに関連付け
				if (bind(socketA, (struct sockaddr*)&sockaddrA, sizeof(sockaddrA)) != 0) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: bind - " << WSAGetLastError() << std::endl;
					WSACleanup();
					return -1;
				}

				// 受信接続をリッスンしている状態でソケットを配置
				if (listen(socketA, 5) != 0) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: listen - " << WSAGetLastError() << std::endl;
					WSACleanup();
					return -1;
				}

				state = 2; // 接続ステートへ

				break;
			}

			// 接続ステート
			case 2: {

				int length = sizeof(sockaddrB);
				socketB = accept(socketA, (struct sockaddr*)&sockaddrB, &length);
				if (socketB == INVALID_SOCKET) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: accept - " << WSAGetLastError() << std::endl;
					continue;
				}

				// 接続成功
				char psb[sizeof "255.255.255.255"];
				std::cout << getDateTime() << " | " << "[ winsock2 API ] 接続を許可しました - from: " << inet_ntop(AF_INET, &sockaddrB.sin_addr, psb, sizeof psb) << ", port: " << ntohs(sockaddrB.sin_port) << std::endl;

				// 接続してきたクライアントへサーバーの名前を返す
				char buffer[] = "3 - kadai_server.cpp";
				int n = send(socketB, buffer, sizeof(buffer), 0);
				if (n < 1) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error_send: " << WSAGetLastError() << std::endl;
					closesocket(socketB);
					continue;
				}

				break;
			}
		}
	}

	// HitAndBlow ゲームを開始
	if (!hitAndBlowGame()) return false;

	std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] 出題値: ";
	for (int i = 0; i < hitAndBlowValues.size(); i++) {
		std::cout << hitAndBlowValues[i];
	}
	std::cout << std::endl;

	while (true) {
		

		while (true) {
			// クライアント側から受信したデータを表示
			char buffer[] = "";
			memset(buffer, 0, sizeof(buffer));
			n = recv(socketB, buffer, sizeof(buffer), 0);
			if (n < 1) {
				std::cout << getDateTime() << " | " << "[ winsock2 API ] error_recv: " << WSAGetLastError() << std::endl;
				break;
			}
			
			// 受信成功
			buffer[n] = '\0'; // 文字列終端を追加

			std::string str = buffer;

			std::regex pattern_digit("^[0-9]+$");
			std::smatch sm;

			if (std::regex_match(str, sm, pattern_digit)) {
				if (str.length() >= 3) {
					std::vector<int> playerValues;
					for (char c : str) {
						playerValues.push_back(c - '0');
					}
					std::vector<int> result = hitAndBlowCheck(playerValues);

					std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] プレイヤー " << inet_ntop(AF_INET, &sockaddrB.sin_addr, psb, sizeof psb) << ":" << ntohs(sockaddrB.sin_port) << " による回答: " << buffer << " (Hits: " << result[0] << ", Blows: " << result[1] << ")" << std::endl;

					std::string response = "Hits: " + std::to_string(result[0]) + ", Blows: " + std::to_string(result[1]);
					send(socketB, response.c_str(), response.size(), 0);
					if (n < 1) {
						std::cout << getDateTime() << " | " << "[ winsock2 API ] error_send: " << WSAGetLastError() << std::endl;
						continue;
					}
				}
			}
		}

		closesocket(socketB);
	}

	closesocket(socketA);

	// winsock2の終了
	WSACleanup();

	return 0;
}

/* if (std::regex_match(str, sm, pattern_symbol) && !std::regex_match(str, sm, pattern_alpha) && !std::regex_match(str, sm, pattern_digit)) {
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
} */
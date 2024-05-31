/*
	ヒットアンドブローゲーム - サーバー
*/

#define WIN32_LEAN_AND_MEAN // 不要なヘッダファイルのインクルードを抑止してコンパイル時間を短縮

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

#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_PORT    12345

#define HAB_LENGTH 3 // ヒットアンドブローゲームの出題の桁数

// ヒットアンドブローゲームの出題値
std::vector<int> hitAndBlowValues;

// ヒットアンドブローゲームを開始
bool hitAndBlowGame(int length = 3) {
	if (length > 10) return false; // 桁数は10桁以内でなければならない

	std::random_device r{};                        // ランダムデバイス（シード生成器）
	std::mt19937 gen(r());                         // メルセンヌ・ツイスタ乱数生成器
	std::uniform_int_distribution<int> dist(0, 9); // 0 - 9 の範囲で生成

	std::unordered_set<int> usedValues; // 使用済みの数字を追跡

	hitAndBlowValues.clear();         // ベクターをクリア（初期化）
	hitAndBlowValues.reserve(length); // 必要な長さ分のメモリを予約

	while (hitAndBlowValues.size() < length) {
		int newValue = dist(gen);

		// 生成された乱数が使用済みでないか確認
		if (usedValues.find(newValue) == usedValues.end()) {
			hitAndBlowValues.push_back(newValue);
			usedValues.insert(newValue);
		}
	}

	return true;
}

// ヒットアンドブローゲームのプレイヤーの入力値をチェックし、HitとBlow数を返す
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
	WSADATA wsaData;                // Windowsソケットの実装に関する情報
	SOCKET sock, sock2;             // 特定のトランスポートサービスプロバイダーにバインドされたソケットを作成
	struct sockaddr_in addr, addr2; // IPアドレスやポート番号の情報を保持

	int state = 1;

	std::cout << getDateTime()         << " | " << "ヒットアンドブローゲーム - サーバー" << std::endl;
	std::cout << "                   " << " | " << "クライアントと接続して問題を出題し、回答を処理します。" << std::endl;

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
				sock = socket(AF_INET, SOCK_STREAM, 0);
				if (sock == INVALID_SOCKET) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: socket - " << WSAGetLastError() << std::endl;

					WSACleanup();

					return -1;
				}

				// 接続先指定用構造体の準備
				addr.sin_family           = AF_INET;
				addr.sin_port             = htons(DEFAULT_PORT);
				addr.sin_addr.S_un.S_addr = INADDR_ANY;

				// ローカルアドレスをソケットに関連付け
				if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: bind - " << WSAGetLastError() << std::endl;

					WSACleanup();

					return -1;
				}

				// 受信接続をリッスンしている状態でソケットを配置
				if (listen(sock, 5) != 0) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: listen - " << WSAGetLastError() << std::endl;

					WSACleanup();

					return -1;
				}

				state = 2; // 接続ステートへ

				break;
			}

			// 接続ステート
			case 2: {

				std::cout << getDateTime() << " | " << "接続を待機しています..." << std::endl;

				// 接続を待機し、許可する
				int length = sizeof(addr2);
				sock2 = accept(sock, (struct sockaddr*)&addr2, &length);
				if (sock2 == INVALID_SOCKET) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: accept - " << WSAGetLastError() << std::endl;

					std::cout << getDateTime() << " | " << "接続の許可に失敗しました。もう一度接続を待機します。" << std::endl;

					break;
				}

				char psb[sizeof "255.255.255.255"];
				std::cout << getDateTime() << " | " << "接続を許可しました。( from: " << inet_ntop(AF_INET, &addr2.sin_addr, psb, sizeof psb) << ", port: " << ntohs(addr2.sin_port) << " )" << std::endl;

				// 接続してきたクライアントへサーバーの名前を返す
				std::string response = "HITANDBLOW_SERVER";
				int ret = send(sock2, response.c_str(), response.length(), 0);
				if (ret < 1) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: send - " << WSAGetLastError() << std::endl;

					std::cout << getDateTime() << " | " << "接続を許可したクライアントへデータの送信に失敗しました。もう一度接続を待機します。" << std::endl;

					closesocket(sock2);

					break;
				}
				else {
					state = 3; // ゲームステートへ
				}

				break;
			}

			// ゲームステート
			case 3: {
				// ヒットアンドブローゲームを開始
				if (!hitAndBlowGame(HAB_LENGTH)) return -1;

				std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] 出題値: ";
				for (int i = 0; i < hitAndBlowValues.size(); i++) {
					std::cout << hitAndBlowValues[i];
				}
				std::cout << std::endl;

				while (true) {
					// クライアントからデータの受信
					std::string buffer(100, '\0');
					int ret = recv(sock2, &buffer[0], buffer.size() - 1, 0);
					if (ret < 1) {
						std::cout << getDateTime() << " | " << "[ winsock2 API ] error: recv - " << WSAGetLastError() << std::endl;

						std::cout << getDateTime() << " | " << "データを受信できませんでした。接続を切断してもう一度接続を待機します。" << std::endl;

						state = 2; // 接続ステートへ

						break;
					}
					buffer.resize(ret);

					// クライアントが終了した場合、ソケットをクローズし接続待機する
					if (buffer == "#") {
						char psb[sizeof "255.255.255.255"];
						std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] プレイヤー " << inet_ntop(AF_INET, &addr2.sin_addr, psb, sizeof psb) << ":" << ntohs(addr2.sin_port) << " がゲームを終了しました。" << std::endl;
						std::cout << getDateTime() << " | " << "接続を切断してもう一度接続を待機します。" << std::endl;

						state = 2; // 接続ステートへ

						break;
					}

					if (buffer == "##") {
						char psb[sizeof "255.255.255.255"];
						std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] プレイヤー " << inet_ntop(AF_INET, &addr2.sin_addr, psb, sizeof psb) << ":" << ntohs(addr2.sin_port) << " がゲームを終了し、サーバーの終了も指示しました。" << std::endl;
						std::cout << getDateTime() << " | " << "接続を切断します。" << std::endl;

						state = 4; // 終了ステートへ

						break;
					}

					std::regex pattern_symbol("^[!-/:-@\[-`{-~]+$"); // 記号の正規表現
					std::regex pattern_alpha("^[a-zA-Z]+$");         // 英字の正規表現
					std::regex pattern_digit("^[0-9]+$");            // 数値の正規表現
					std::smatch sm;

					// 3桁で数値のみの受信データの場合、HitとBlow数を計算して結果を送信する
					if (buffer.length() == HAB_LENGTH && std::regex_match(buffer, sm, pattern_digit) && !std::regex_match(buffer, sm, pattern_symbol) && !std::regex_match(buffer, sm, pattern_alpha)) {
						// HitとBlow数を計算
						std::vector<int> playerValues;
						for (char c : buffer) {
							playerValues.push_back(c - '0'); // '0'のASCIIコードを引いてintに変換
						}
						std::vector<int> result = hitAndBlowCheck(playerValues);

						char psb[sizeof "255.255.255.255"];
						std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] プレイヤー " << inet_ntop(AF_INET, &addr2.sin_addr, psb, sizeof psb) << ":" << ntohs(addr2.sin_port) << " による回答: " << buffer << " ( Hit: " << result[0] << ", Blow: " << result[1] << " )" << std::endl;

						// クライアントへ結果を送信
						std::string response = std::to_string(result[0]) + std::to_string(result[1]); // Hit / Blow 数
						int ret = send(sock2, response.c_str(), response.size(), 0);
						if (ret < 1) {
							std::cout << getDateTime() << " | " << "[ winsock2 API ] error: send - " << WSAGetLastError() << std::endl;

							std::cout << getDateTime() << " | " << "データを受信できませんでした。接続を切断してもう一度接続を待機します。" << std::endl;

							state = 2; // 接続ステートへ

							break;
						}
					}
				}

				closesocket(sock2);

				break;
			}

			// 終了ステート
			case 4: {
				closesocket(sock);
				WSACleanup();

				std::cout << getDateTime() << " | " << "アプリを終了します。" << std::endl;

				state = 0; // ループ終了

				break;
			}

			default: {
				return -1;
			}
		}
	}

	return 0;
}
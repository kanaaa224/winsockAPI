/*
	ヒットアンドブローゲーム - クライアント
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
#include <tchar.h>

#include <winsock2.h>
#include <Ws2tcpip.h>

#define DEFAULT_ADDRESS "127.0.0.1"
#define DEFAULT_PORT    12345

#define HAB_LENGTH 3 // ヒットアンドブローゲームの出題の桁数

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
	WSADATA wsaData;         // Windowsソケットの実装に関する情報
	SOCKET sock;             // 特定のトランスポートサービスプロバイダーにバインドされたソケットを作成
	struct sockaddr_in addr; // IPアドレスやポート番号の情報を保持

	int state = 1;

	std::cout << getDateTime()         << " | " << "ヒットアンドブローゲーム - クライアント" << std::endl;
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
				sock = socket(AF_INET, SOCK_STREAM, 0);
				if (sock == INVALID_SOCKET) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: socket - " << WSAGetLastError() << std::endl;

					WSACleanup();

					return -1;
				}

				// 接続先指定用構造体の準備
				addr.sin_family = AF_INET;
				addr.sin_port   = htons(DEFAULT_PORT);

				// ローカルアドレスを数値バイナリ形式に変換
				if (InetPton(addr.sin_family, _T(DEFAULT_ADDRESS), &addr.sin_addr.S_un.S_addr) <= 0) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: inetPton - " << WSAGetLastError() << std::endl;

					closesocket(sock);
					WSACleanup();

					return -1;
				}

				state = 2; // 接続ステートへ

				break;
			}

			// 接続ステート
			case 2: {
				std::cout << getDateTime() << " | " << "サーバー(" << DEFAULT_ADDRESS << ":" << DEFAULT_PORT << ")へ接続しています..." << std::endl;

				// サーバーに接続
				if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) <= SOCKET_ERROR) {
					std::cout << getDateTime() << " | " << "[ winsock2 API ] error: connect - " << WSAGetLastError() << std::endl;

					closesocket(sock);
					WSACleanup();

					while (true) {
						std::cout << getDateTime() << " | " << "サーバーへ接続できませんでした。再試行しますか？ (y / n): ";

						std::string input;
						std::getline(std::cin, input);

						if (input == "y" || input == "Y") {
							state = 1; // 起動ステートへ
							break;
						}
						else if (input == "n" || input == "N" || input == "#") {
							state = 5; // 終了ステートへ
							break;
						}
						else {
							std::cout << getDateTime() << " | " << "認識できない入力がされました。" << std::endl;
							continue;
						}
					}
				}
				else {
					// サーバーからのデータを受信
					std::string buffer(100, '\0');
					int ret = recv(sock, &buffer[0], buffer.size() - 1, 0);
					if (ret < 0) { // エラー
						std::cout << getDateTime() << " | " << "[ winsock2 API ] error: recv - " << WSAGetLastError() << std::endl;

						closesocket(sock);
						WSACleanup();

						return -1;
					}
					else if (ret == 0) { // クローズ
						closesocket(sock);
						WSACleanup();
						
						while (true) {
							std::cout << getDateTime() << " | " << "サーバーへ接続できましたが、データが受信できませんでした。再試行しますか？ (y / n): ";

							std::string input;
							std::getline(std::cin, input);

							if (input == "y" || input == "Y") {
								state = 1; // 起動ステートへ
								break;
							}
							else if (input == "n" || input == "N" || input == "#") {
								state = 5; // 終了ステートへ
								break;
							}
							else {
								std::cout << getDateTime() << " | " << "認識できない入力がされました。" << std::endl;
								continue;
							}
						}
					}
					else if (ret > 0) { // 受信成功
						buffer.resize(ret); // 実際に受信したサイズに調整

						std::cout << getDateTime() << " | " << "サーバーに接続しました。( serverName: " << buffer << " )" << std::endl;

						state = 3; // ゲームステートへ
					}
				}

				break;
			}

			// ゲームステート
			case 3: {
				int gameMode = 0; // 回答モード
				int num      = 0; // 回答回数

				std::vector<std::string> inputs; // 入力記録
				std::vector<int> hits;           // hit数の記録
				std::vector<int> blows;          // blow数の記録

				std::cout << getDateTime()         << " | " << "[ HIT AND BLOW ] ゲームを開始します。" << std::endl;
				std::cout << "                   " << " | " << "[ HIT AND BLOW ] #  を入力すると接続を切断しゲームを終了します。" << std::endl;
				std::cout << "                   " << " | " << "[ HIT AND BLOW ] ## を入力すると接続を切断しゲームを終了してサーバーに停止するよう指示します。" << std::endl;

				// ゲーム開始
				while (true) {
					std::string input;

					int hit  = 0;
					int blow = 0;

					// 回答モード選択
					while (gameMode <= 0) {
						std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] 回答モードを選択 ( 1: 自己回答モード / 2: 自動回答モード / #: 終了 ): ";
						std::getline(std::cin, input);

						if (input == "1") {
							std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] 自己回答モードが設定されました。" << std::endl;
							gameMode = 1; // 自己回答モード
							break;
						}
						else if (input == "2") {
							std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] 自動回答モードが設定されました。" << std::endl;
							gameMode = 2; // 自動回答モード
							break;
						}
						else if (input == "#" || input == "##") {
							std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] ゲームを終了します。" << std::endl;
							break;
						}
						else {
							std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] 認識できない入力がされました。" << std::endl;
							continue;
						}
					}

					// 自己回答モード
					if (gameMode == 1) {
						while (true) {
							// 文字入力
							std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] 回答を入力 ( 3桁の数値 / #: 終了 ): ";
							std::getline(std::cin, input);

							std::regex pattern_symbol("^[!-/:-@\[-`{-~]+$"); // 記号の正規表現
							std::regex pattern_alpha("^[a-zA-Z]+$");         // 英字の正規表現
							std::regex pattern_digit("^[0-9]+$");            // 数値の正規表現
							std::smatch sm;

							if (input.length() == HAB_LENGTH) {
								break;
							}
							else if (input == "#" || input == "##") {
								std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] ゲームを終了します。" << std::endl;

								gameMode = 0;

								break;
							}
							else if (input.length() != HAB_LENGTH) {
								std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] 必要桁数は3桁です。" << std::endl;

								continue;
							}
							else if (!(std::regex_match(input, sm, pattern_digit) && !std::regex_match(input, sm, pattern_symbol) && !std::regex_match(input, sm, pattern_alpha))) {
								std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] 数値でなければなりません。" << std::endl;

								continue;
							}
						}
					}

					// 自動回答モード
					else if (gameMode == 2) {
						std::vector<int> hitAndBlowValues;

						std::random_device r{};                        // ランダムデバイス（シード生成器）
						std::mt19937 gen(r());                         // メルセンヌ・ツイスタ乱数生成器
						std::uniform_int_distribution<int> dist(0, 9); // 0 - 9 の範囲で生成

						std::unordered_set<int> usedValues; // 使用済みの数字を追跡

						hitAndBlowValues.clear();             // ベクターをクリア（初期化）
						hitAndBlowValues.reserve(HAB_LENGTH); // 必要な長さ分のメモリを予約

						while (hitAndBlowValues.size() < HAB_LENGTH) {
							int newValue = dist(gen);

							// 生成された乱数が使用済みでないか確認
							if (usedValues.find(newValue) == usedValues.end()) {
								hitAndBlowValues.push_back(newValue);
								usedValues.insert(newValue);
							}
						}

						input = "";
						for (int i = 0; i < HAB_LENGTH; i++) {
							input += std::to_string(hitAndBlowValues[i]);
						}

						std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] 自動回答: " << input << std::endl;
					}

					// サーバーに回答を送信
					if (gameMode && input.length() == HAB_LENGTH) {
						// サーバーへデータを送信
						int ret = send(sock, input.c_str(), input.length(), 0);
						if (ret < 1) {
							std::cout << getDateTime() << " | " << "[ winsock2 API ] error: send - " << WSAGetLastError() << std::endl;
							
							std::cout << getDateTime() << " | " << "データを送信できませんでした。続行できないためゲームを終了します。" << std::endl;

							input = "#";

							gameMode = 0;
						}
					}

					// サーバーから Hit と Blow 数を受け取る
					if (gameMode && input.length() == HAB_LENGTH) {
						// サーバーからデータ受信
						std::string buffer(100, '\0');
						int ret = recv(sock, &buffer[0], buffer.size() - 1, 0);
						if (ret < 1) {
							std::cout << getDateTime() << " | " << "[ winsock2 API ] error: recv - " << WSAGetLastError() << std::endl;
							
							std::cout << getDateTime() << " | " << "データを受信できませんでした。続行できないためゲームを終了します。" << std::endl;

							input = "#";

							gameMode = 0;
						}
						else {
							buffer.resize(ret);

							// Hit と Blow 数を受け取る
							if (buffer.length() == 2) {

								// '1' の ASCII コードから '0' の ASCII コードを引くことで 1 になる
								hit  = buffer[0] - '0';
								blow = buffer[1] - '0';
							}
						}
					}

					// 結果に応じて回答を処理
					if (gameMode && input.length() == HAB_LENGTH) {
						num++;

						if (hit != HAB_LENGTH) {
							std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] " << hit << " HIT, " << blow << " BLOW." << std::endl;

							inputs.push_back(input);
							blows .push_back(blow);
							hits  .push_back(hit);

							continue;
						}
						else {
							std::cout << getDateTime() << " | " << "[ HIT AND BLOW ] ゲームクリア！ あなたは " << num << " 回目でクリアしました。 ゲームを終了します。" << std::endl;

							input = "#";

							gameMode = 0;
						}
					}

					// 入力 # / ## で終了
					if (input == "#" || input == "##") {
						// サーバーへデータを送信
						send(sock, input.c_str(), input.length(), 0);

						state = 4; // 終了確認ステートへ

						break;
					}
				}

				break;
			}

			// 終了確認ステート
			case 4: {
				closesocket(sock);
				WSACleanup();

				std::cout << getDateTime() << " | " << "サーバーとの接続を切断しました。" << std::endl;

				while (true) {
					std::cout << getDateTime() << " | " << "アプリを終了しますか？ (y / n): ";

					std::string input;
					std::getline(std::cin, input);

					if (input == "y" || input == "Y" || input == "#") {
						state = 5; // 終了ステートへ
						break;
					}
					else if (input == "n" || input == "N") {
						std::cout << getDateTime() << " | " << "もう一度サーバーへ接続し、新規ゲームを開始します。" << std::endl;
						state = 1; // 起動ステートへ
						break;
					}
					else {
						std::cout << getDateTime() << " | " << "認識できない入力がされました。" << std::endl;
						continue;
					}
				}
				
				break;
			}

			// 終了ステート
			case 5: {
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
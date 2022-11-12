#define OLC_PGE_APPLICATION
#define NOMINMAX

using namespace std;
#include "map.h"
#include "server.h"
#include <stdlib.h>

int NUM_PLAYERS = 0;
int MAX_PLAYERS = 8;

bool connected = true;

int turn = 1;

std::string IP = "192.168.1.140";
int port = 1234;
int seed = 1;
SOCKET listening;
fd_set master;

std::vector<SOCKET> sockets = {};
std::vector<unsigned int> last = {};
unsigned int messages = 0;

std::vector<std::vector<Tile>> map;
std::deque<std::string> buffer = {};

//Server Code ---------------------------------------------------------------------------------------
void server_join();
void server_listen();
void server_send();

void sendData(std::string s) {
	buffer.push_back(s);
}
std::string sendUnit(int x, int y, std::string r) {
	std::string s = "u" + r;
	//First the Coordinates
	s += to_str(x) + ".";
	s += to_str(y) + ".";
	//Now the Unit Data
	s += '0' + map[y][x].unit.owner;
	s += '0' + map[y][x].unit.type;
	s += '0' + map[y][x].unit.HP;
	s += '0' + map[y][x].unit.MP;

	return s;
}
std::string sendBuilding(int x, int y, std::string r) {
	std::string s = "b" + r;
	//First the Coordinates
	s += to_str(x) + ".";
	s += to_str(y) + ".";
	//Now the Building Data
	s += '0' + map[y][x].owner;
	s += '0' + map[y][x].building;

	return s;
}

std::string sendTile(int x, int y, std::string r) {
	std::string s = "g" + r;
	//First the Coordinates
	s += to_str(x) + ".";
	s += to_str(y) + ".";
	//Now the Building Data
	s += '0' + map[y][x].type;
	s += '0' + map[y][x].elev;
	s += '0' + map[y][x].forest;

	return s;
}

void unitChange(std::string s, std::string r) {
	int x = readInt(s);
	int y = readInt(s);

	Unit u;
	u.owner = s[0] - '0';
	u.type = s[1] - '0';
	u.HP = s[2] - '0';
	u.MP = s[3] - '0';

	map[y][x].unit = u;
	sendData(sendUnit(x, y, r));
}
void buildingChange(std::string s, std::string r) {
	int x = readInt(s);
	int y = readInt(s);

	map[y][x].owner = s[0] - '0';
	map[y][x].building = s[1] - '0';

	sendData(sendBuilding(x, y, r));
}

std::string sendRow(int i, std::string r) {
	std::string s = "r" + r;
	s += to_str(i) + ".";
	for (Tile t : map[i]) {
		s += '0' + t.type;
		s += '0' + t.elev;
		s += '0' + t.forest;
		s += '0' + t.owner;
		s += '0' + t.building;
		s += '0' + t.unit.owner;
		s += '0' + t.unit.type;
		s += '0' + t.unit.HP;
		s += '0' + t.unit.MP;
	}
	return s;
}

std::string sendSeed(int n, std::string r) {
	return "s" + r + to_str(n);
}
std::string sendMAPSIZE(int n, std::string r) {
	return "m" + r + to_str(n);
}

void serverInit() {
	//Set Up Server--------------------------------------
	std::string input = "";
	//std::cout << "IP: ";
	//std::cin >> IP;
	//std::cout << "Port: ";
	//std::cin >> port;
	//std::cout << "Map Size: ";
	//std::cin >> input;
	//MAPSIZE = std::stoi(input);
	MAPSIZE = 10;
	MODI = std::max(.4, (MAPSIZE) / (1000.0));
	//std::cout << "Seed: ";
	//std::cin >> input;
	//seed = std::stoi(input);
	seed = 50;

	map = createMap(seed);

	server_join();
	std::thread aList(server_listen);
	std::thread bList(server_send);
	aList.detach();
	bList.detach();

	for (;;) {}
	//-----------------------------------------------------
}
void server_join() {
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);
	WSAStartup(ver, &wsData);

	listening = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, IP.c_str(), &hint.sin_addr);

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	listen(listening, SOMAXCONN);
}
void server_listen() {
	FD_ZERO(&master);
	FD_SET(listening, &master);

	while (connected) {
		fd_set copy = master;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);
		for (int i = 0; i < socketCount; i++) {
			SOCKET current = copy.fd_array[i];
			if (current == listening) {
				std::cout << "New Connection" << std::endl;
				SOCKET new_client = accept(listening, nullptr, nullptr);
				FD_SET(new_client, &master);
				sockets.push_back(new_client);
				last.push_back(messages);

				std::string p_turn = "t0" + to_str((++NUM_PLAYERS) % (MAX_PLAYERS + 1));
				std::cout << "Setting new player's turn to " << p_turn << std::endl;
				send(new_client, p_turn.c_str(), p_turn.size() + 1, 0);

				sendData(sendMAPSIZE(MAPSIZE, "0" + to_str((NUM_PLAYERS) % (MAX_PLAYERS + 1))));
				for (int i = 0; i < map.size(); i++) {
					sendData(sendRow(i, "0" + to_str((NUM_PLAYERS) % (MAX_PLAYERS + 1))));
				}
				sendData(sendMAPSIZE(MAPSIZE, "0" + to_str((NUM_PLAYERS) % (MAX_PLAYERS + 1))));
				sendData("e00" + to_str(turn));
			}
			else {
				char buf[2400];
				ZeroMemory(buf, 2400);

				// Receive message
				int bytesIn = recv(current, buf, 2400, 0);
				std::cout << "Received a message: " << buf << std::endl;
				if (bytesIn <= 0) {
					// Drop the client
					std::cout << "Dropping Client" << std::endl;
					for (int i = 0; i < sockets.size(); i++) {
						if (sockets[i] == current) {
							sockets.erase(sockets.begin() + i);
							last.erase(last.begin() + i);
							break;
						}
					}
					closesocket(current);
					FD_CLR(current, &master);
				}
				else if (bytesIn > 1) { //Process Message from Client
					std::string msg(buf);
					char c = msg[0];
					char sender = msg[1];
					msg = msg.substr(2);
					if (c == 'u') { //Unit Change
						unitChange(msg, "1" + sender);
					}
					else if (c == 'b') { //Building CHange
						buildingChange(msg, "1" + sender);
					}
					else if (c == 'e') {
						std::cout << "Received Turn Update" << std::endl;
						std::cout << "Sender: " << (sender) << std::endl;
						std::cout << "Turn: " << turn << std::endl;
						if ((sender - '0') == turn) {
							turn = 1 + (turn % (NUM_PLAYERS));
							sendData("e00" + to_str(turn));
						}
					}
					else if (c == 'r') { //User is Requesting a Row
						int index = readInt(msg);
						std::cout << "Sending Row to " << (int)sender << std::endl;
						sendData(sendRow(index, "0" + to_str(sender)));
					}
				}
			}
		}
	}
}
void server_send() {
	FD_ZERO(&master);
	FD_SET(listening, &master);

	int index = 0;
	bool received = false;
	static int wait = 25;

	while (connected) {
		if (wait < 25) {
			wait = 25;
		}
		Sleep(wait);
		if (buffer.size() > 0) {
			std::string s = buffer.front();
			for (int i = 0; i < master.fd_count; i++) {
				SOCKET outSock = master.fd_array[i];
				if (outSock != listening) {
					//Check if this socket has already received the message
					received = false;
					for (int j = 0; j < sockets.size(); j++) {
						if (sockets[j] == outSock) {
							index = j;
							if (last[j] > messages) {
								received = true;
							}
							break;
						}
					}
					if (!received) { //Send the msg to the client
						last[index] = messages + 1;
						wait = 40 + s.size() / 10;
						int ape = send(outSock, s.c_str(), s.size() + 1, 0);
						if (ape != s.size() + 1) {
							last[index]--;
							std::cout << "Error sending msg: " << ape << std::endl;
						}
						else {
							std::cout << messages << std::endl;
						}
					}
				}
			}
			received = true;
			for (int j = 0; j < last.size(); j++) {
				if (last[j] <= messages) {
					received = false;
				}
			}
			if (received) {
				messages++;
				buffer.pop_front();
			}
		}
	}
}


//End Server Code--------------------------------------------------------------------------------------

int main()
{
	serverInit();

	return 0;
}




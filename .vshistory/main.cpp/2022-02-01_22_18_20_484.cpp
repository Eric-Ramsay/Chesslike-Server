#define OLC_PGE_APPLICATION
#define NOMINMAX

using namespace std;
#include "map.h"
#include "server.h"
#include <stdlib.h>
#include <fstream>
#include <chrono>

bool connected = true;

int turn = 1;

std::string IP = "192.168.1.140";
int port = 1234;
int seed = 1;
SOCKET listening;
fd_set master;

struct Player {
	int turn = 0;
	int gold = 0;
	int last = 0;
	SOCKET socket;
	bool active = false;
};

std::vector<Player> players(8);

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

void tileChange(std::string s, std::string r) {
	int x = readInt(s);
	int y = readInt(s);

	Tile t;
	t.type = (TERRAIN)(s[0] - '0');
	t.elev = (ELEVATION)(s[1] - '0');
	t.forest = (FOREST)(s[2] - '0');

	map[y][x] = t;

	sendData(sendTile(x, y, r));
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

std::vector<Tile> readRow(std::string in) {
	int which = 0;
	std::vector<Tile> row = {};
	Tile t;
	for (char c : in) {
		switch (which) {
		case 0: t.type = (TERRAIN)(c - '0'); break; //Type
		case 1: t.elev = (ELEVATION)(c - '0');  break; //Elevation
		case 2: t.forest = (FOREST)(c - '0'); break; //Forest

		case 3: t.owner = (c - '0'); break; //Owner
		case 4: t.building = (c - '0'); break; //Building

		case 5: t.unit.owner = (c - '0'); break; //U Owner
		case 6: t.unit.type = (c - '0'); break; //U Type
		case 7: t.unit.HP = (c - '0'); break; //U HP
		case 8: t.unit.MP = (c - '0'); 
		row.push_back(t); t = Tile(); if (t.elev < FLAT) {
			if (t.type >= GRASS) { t.elev = FLAT; }
		}
		which = -1; break; //U MP
		}
		which++;
	}
	return row;
}

void loadMap(std::vector<std::vector<Tile>>& map, std::string path) {
	ifstream myfile;
	std::string line;
	std::string num = "";
	int i = 0;
	int selector = 0;
	Tile t;
	myfile.open("saves/" + path);
	if (myfile.is_open()) {
		map = {};
		std::vector<Tile> current = {};
		while (std::getline(myfile, line)) {
			std::vector<Tile> row = readRow(line);
			map.push_back(row);
		}
	}
	myfile.close();
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
	MAPSIZE = 45;
	MODI = std::max(.4, (MAPSIZE) / (1000.0));
	//std::cout << "Seed: ";
	//std::cin >> input;
	//seed = std::stoi(input);
	
	seed = rand() % 20000;
	map = createMap(seed);

	//loadMap(map, "save.txt");

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
			if (current == listening) { //A new Player is connecting -----------
				int sTurn = -1;
				for (int i = 0; i < players.size(); i++) {
					if (!players[i].active) {
						sTurn = i;
						i = players.size();
					}
				}
				if (sTurn != -1) {
					std::cout << "New Connection: " << sTurn+1 << std::endl;
					SOCKET new_client = accept(listening, nullptr, nullptr);
					FD_SET(new_client, &master);

					//Set server-side player variables
					players[sTurn].active = true;
					players[sTurn].socket = new_client;
					players[sTurn].last = messages;
					players[sTurn].turn = sTurn+1;

					//Send the player their turn
					std::string p_turn = "t0" + to_str(sTurn + 1);
					send(new_client, p_turn.c_str(), p_turn.size() + 1, 0);

					//Send the map to the player, row by row
					sendData(sendMAPSIZE(MAPSIZE, "0" + to_str(sTurn + 1)));
					for (int i = 0; i < map.size(); i++) {
						sendData(sendRow(i, "0" + to_str(sTurn + 1)));
					}
					//Send the mapsize, signifying the map has finished sending
					sendData(sendMAPSIZE(MAPSIZE, "0" + to_str(sTurn + 1)));

					//Let the player know whose turn it is
					sendData("e0" + to_str(sTurn + 1) + to_str(turn));
				}
				else {
					std::cout << "Too many players. Connection rejected." << std::endl;
				}
			}
			else { //Otherwise, a message came in from a player who was already connected --------------------
				for (Player p : players) { //Check which player sent the msg
					if (p.socket == current) {
						char buf[2400];
						ZeroMemory(buf, 2400);

						int bytesIn = recv(current, buf, 2400, 0);
						std::cout << "Received a message: " << buf << std::endl;

						if (bytesIn <= 0) { //The player DC'd
							std::cout << "Player Disconnected" << std::endl;
							p.active = false;
							closesocket(current);
							FD_CLR(current, &master);
						}
						else if (bytesIn > 1) { //Process Message from Client
							std::string msg(buf);
							char c = msg[0];
							std::string sender = to_str(p.turn);
							msg = msg.substr(1);
							if (c == 't') { //Tile Change
								tileChange(msg, "1" + sender);
							}
							if (c == 'u') { //Unit Change
								unitChange(msg, "1" + sender);
							}
							else if (c == 'b') { //Building CHange
								buildingChange(msg, "1" + sender);
							}
							else if (c == 'e' || c == 'E') {
								if (p.turn == turn || c == 'E') { //End the current players turn. If, 'E', admin is overriding
									while (!players[turn - 1].active) {
										turn = 1 + ((1 + turn) % (players.size() + 1));
									}
									sendData("e00" + to_str(turn));
								}
								else if (c != 'E') { //Make sure player knows whose turn it is
									sendData("e0" + sender + to_str(turn));
								}
							}
							else if (c == 'r') { //User is Requesting a Row
								int index = readInt(msg);
								sendData(sendRow(index, "0" + sender));
							}
						}
					}
				}
			}
		}
	}
}

void server_send() {
	FD_ZERO(&master);
	FD_SET(listening, &master);
	int timer = 0;
	int target = 0;
	auto time = std::chrono::system_clock::now();
	auto last_time = time;
	std::chrono::duration<float> elapsedTime = time - last_time;
	float aTime = 0.0f;

	while (connected) {
		time = std::chrono::system_clock::now();
		elapsedTime = time - last_time;
		last_time = time;
		aTime += elapsedTime.count();
		if (aTime >= 1.0f/50.0f) {
			aTime -= 1.0f / 50.0f;
			timer++;
			if (timer == 500) {
				timer = 0;
			}
		}
		if (buffer.size() > 0) {
			if (timer == target) {
				std::string s = buffer.front();
				for (int i = 0; i < players.size(); i++) {
					if (players[i].active) {
						int byt = send(players[i].socket, s.c_str(), s.size() + 1, 0);
						std::cout << "Sending a message: " << byt << std::endl;
						target = (timer + 5) % 500;
					}
				}
				buffer.pop_front();
			}
		}
		else {
			target = (timer + 5) % 500;
		}
	}
}


//End Server Code--------------------------------------------------------------------------------------

int main()
{
	serverInit();

	return 0;
}




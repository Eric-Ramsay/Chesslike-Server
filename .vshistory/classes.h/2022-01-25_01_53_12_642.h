#pragma once
#include <deque> //used for faster river generation, and for pathfinding
#include <vector> //used because arrays suck
#include <algorithm> //used for min and max functions
#include <stdlib.h>     /* srand, rand */
#include <time.h> 
#include <iostream>
#include <thread>
#include <string>
#include <sstream>

int MAPSIZE = 50;
float MODI = std::max(.3, (MAPSIZE) / (2000.0));

enum TERRAIN {
	SHALLOW, OCEAN, RIVER, LAKE, GRASS, STEPPE, SAVANNA, MEADOW, DESERT, TUNDRA, ICE, BOG, COLD_DESERT
};
enum ELEVATION {
	WATER, FLAT, HILL, MOUNTAIN
};

enum FOREST {
	NONE, TEMPERATE, JUNGLE, TAIGA, GLADE
};

struct C {
	int x;
	int y;
	C(int x1, int y1) {
		x = x1;
		y = y1;
	}
};

struct Unit {
	char owner = 0;
	char type = 0;

	char HP = 100;
	char MP = 0;

};

struct Tile {
	char owner = 0;

	TERRAIN type = GRASS;
	ELEVATION elev = FLAT;
	FOREST forest = NONE;

	char building = 0;
	Unit unit;
};

struct MapTile {
	float heat = 50; // 0 to 100
	float rain = 0; //0 to 1000
	char type = 'g';
	float elevation = 25; //0 to 100
	char forest = 'e';
	float forestChance = 0;
	int id[3] = { -1, -1, -1 };
	int checked = -1;
};

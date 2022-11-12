#pragma once
#include "classes.h"
template < typename Type > std::string to_str(const Type& t)
{
	std::ostringstream os;
	os << t;
	return os.str();
}

int readInt(std::string& s) {
	int num = 0;
	bool reading_num = true;
	std::string copy = "";
	for (char c : s) {
		if (reading_num) {
			if (c >= '0' && c <= '9') {
				num *= 10;
				num += (int)(c - '0');
			}
			else {
				reading_num = false;
			}
		}
		else {
			copy += c;
		}
	}
	s = copy;
	return num;
}

int safeC(int a, int MAX = MAPSIZE) {
	if (a >= MAX) {
		return a % MAX;
	}
	else if (a < 0) {
		return safeC(a + MAX);
	}
	return a;
}

std::string low(std::string a) {
	std::string word = "";
	bool skip = false;
	for (char c : a) {
		if (c == '*') {
			skip = !skip;
		}
		if (skip) {
			word += c;
		}
		else {
			word += std::tolower(c);
		}
	}
	return word;
}

Tile Terrain_Convert(MapTile t) {
	Tile til;
	static int mmax = 0;
	if (t.elevation <= 0 || t.type == 'e' || t.type == 'w' || t.type == 'r' || t.type == 'a') {
		til.elev = WATER;
	}
	else if (t.elevation >= 50) {
		if (t.elevation <= 75) {
			til.elev = HILL;
		}
		else {
			til.elev = MOUNTAIN;
		}
	}
	if (t.forest != 'e' && til.elev < MOUNTAIN && til.elev > WATER) {
		switch (t.forest) {
		case 'e': til.forest = NONE; break;
		case 's': til.forest = FISH; break;
		case 'f': til.forest = TEMPERATE; break;
		case 'b': til.forest = TAIGA; break;
		case 'n': til.forest = JUNGLE; break;
		case 'l': til.forest = GLADE; break;
		}
	}
	switch (t.type) {
	case 'w': til.type = OCEAN; break;
	case 'r': til.type = RIVER; break;
	case 'a': til.type = LAKE; break;
	case 'g': til.type = GRASS; break;
	case 'p': til.type = STEPPE; break;
	case 'v': til.type = SAVANNA; break;
	case 'q': til.type = COLD_DESERT; break;
	case 'd': til.type = DESERT; break;
	case 't': til.type = TUNDRA; break;
	case 'c': til.type = ICE; break;
	case 'o': til.type = BOG; break;
	case 'm': til.type = MEADOW; break;
	case 'e': til.type = SHALLOW; break;
	default: std::cout << t.type << std::endl; til.type = GRASS; break;
	}
	return til;
}

int min(int a, int b) {
	if (a < b) {
		return a;
	}
	return b;
}


int measureText(std::string text) {
	bool skipping = false;
	int size = 0;
	for (char& c : text) {
		if (c == '*') {
			skipping = !skipping;
		}
		else if (!skipping) {
			if (c != '\n') {
				size++;
			}
		}
	}
	return size;
}
bool range(int x1, int y1, int x2, int y2, int width, int height) {
	if (height == -1) {
		height = width;
	}
	return (x1 >= x2 && y1 >= y2 && x1 <= x2 + width && y1 <= y2 + height);
}

std::string trimNum(int x, int throttle = -1, bool prettify = false) {
	char append = '!';
	int excess = 0;
	int num = x;
	if (num >= 1000) {
		excess = num;
		if (num >= 1000000) {
			append = 'm';
			num /= 1000000;
			excess -= num * 1000000;
			excess = (excess / 100000);
		}
		else {
			append = 'k';
			num /= 1000;
			excess -= num * 1000;
			excess = (excess / 100);
		}
	}
	std::string result = std::to_string(num);
	if (excess > 0 && (throttle == -1 || num < pow(10, throttle - 1))) {
		if (excess - (excess / 100) * 100 == 0) {
			excess = 100 * (excess / 100);
		}
		result += "." + std::to_string(excess);
	}
	if (append == '!') {
		return result;
	}
	if (prettify) {
		return result + "*GREY*" + append + "*GREY*";
	}
	return  result + append;
}

int typePrecedent(TERRAIN a) {
	switch (a) {
	case GRASS: return 1;
	case MEADOW: return 2;
	case STEPPE: return 3;
	case SAVANNA: return 4;
	case DESERT: case COLD_DESERT: return 13;
	case TUNDRA: return 6;
	case BOG: return 7;
	case ICE: return 12;
	case OCEAN: return -1;
	case SHALLOW: return 0;
	case RIVER: return -3;
	case LAKE: return -2;
	}
	return -1;
}

unsigned long long int srandC(unsigned long long int seed) {
	unsigned long long int a = 1103515245;
	int c = 12345;
	unsigned long long int m = 2 ^ 32;
	return ((a * seed + c) % m);
}

float randC(int x1, int y1) {
	unsigned long long int x = (unsigned long long int)x1;
	unsigned long long int y = (unsigned long long int)y1;
	unsigned long long int a = 1103515245;
	int c = 12345;
	long m = 2 ^ 32;
	unsigned long long int seed = srandC(x + y + x * y + x * x + y * y);
	return fabsf((float)((a * seed + c) % m) / m);
}
float square(float x) {
	return x * x;
}
int square(int x) {
	return x * x;
}
double square(double x) {
	return x * x;
}


int dist(int x1, int y1, int x2, int y2) {
	//This is 'Manhattan dist', since units can't move diagonally
	int xDist = std::abs(x1 - x2);
	int yDist = std::abs(y1 - y2);

	if (xDist > MAPSIZE / 2) {
		xDist = MAPSIZE - xDist;
	}
	if (yDist > MAPSIZE / 2) {
		yDist = MAPSIZE - yDist;
	}

	return xDist + yDist;
}


float fDist(int x1, int y1, int x2, int y2) {
	int xDist = std::abs(x1 - x2);
	int yDist = std::abs(y1 - y2);

	if (xDist > MAPSIZE / 2) {
		xDist = MAPSIZE - xDist;
	}
	if (yDist > MAPSIZE / 2) {
		yDist = MAPSIZE - yDist;
	}

	return std::sqrt(xDist * xDist + yDist * yDist);
}

int coord_dist(int a, int b) {
	int d = std::abs(a - b);
	if (d > MAPSIZE / 2) {
		return MAPSIZE - d;
	}
	return d;
}


bool c_less(int a, int b) {
	int d = std::abs(a - b); //coord less

	if (d > MAPSIZE / 2) {
		if (a > MAPSIZE / 2) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return a < b;
	}
}


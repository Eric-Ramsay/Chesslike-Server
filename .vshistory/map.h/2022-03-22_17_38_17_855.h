#include "functions.h"

int NUM_THREADS = 4;

float latTemp(int x) {
	//return 100 - square((float)(x - MAPSIZE / 2.0) / (MAPSIZE / 20.0));
	//return (100.0 / square(1.0 + square((x - MAPSIZE/2) / (MAPSIZE/3.0))));
	float temp = 100 * (1 - (abs(MAPSIZE / 2 - (x-5)) / (float)(MAPSIZE / 4)));
	if (temp > 0) {
		return temp;
	}
	return 0;
}

float jungEquator(int x) {
	return (100.0 / square(1.0 + square((x - MAPSIZE / 2) / (MAPSIZE / 20.0))));
}



void hill(std::vector<std::vector<MapTile>>& map, int elev, int rSize, int rNum) {
	int yC = (int)floor((((double)rand() / RAND_MAX)) * MAPSIZE);
	int xC = (int)floor((((double)rand() / RAND_MAX)) * MAPSIZE);
	int rS = 0;
	int rN = 0;
	int dir = 0;
	int lastDir = -1;
	int bannedDir = (int)floor((((double)rand() / RAND_MAX)) * 4);
	while (rN < rNum)
	{
		if (rS >= rSize || rN == 0)
		{
			rS = 0;
			rN++;
			yC = (MAPSIZE * .3) + rand() % MAPSIZE * .4;
			xC = (MAPSIZE * .3) + rand() % MAPSIZE * .4;
			bannedDir = (int)floor((((double)rand() / RAND_MAX)) * 4);
		}
		do {
			dir = (int)floor((((double)rand() / RAND_MAX)) * 4);
		} while (dir == lastDir || dir == bannedDir);
		switch (dir) {
		case 0: yC--; xC--; break;
		case 1: yC--; xC++; break;
		case 2: yC++; xC++; break;
		case 3: yC++; xC--; break;
		}
		yC = safeC(yC);
		xC = safeC(xC);
		if (map[yC][xC].elevation > 0) {
			map[yC][xC].elevation = elev / 2;
			if (rS == 0 || rand() % 10 > 5) {
				map[yC][xC].elevation = elev;
			}
			if (dir < 4) {
				lastDir = dir + 2;
			}
			else {
				lastDir = dir - 2;
			}
		}
		else { rS = rSize - 1; }
		rS++;
	}
}

void rainSpread(std::vector< std::vector<MapTile>>& map) {
	for (int m = 2; m > 0; m--) {
		for (int h = 1; h >= 0; h--) {
			int i = h * (MAPSIZE - 1);
			while (i < MAPSIZE && i >= 0) {
				int j = h * (MAPSIZE - 1);
				while (j < MAPSIZE && j >= 0) {
					float avg = 0;
					float num = 0;
					if (map[i][j].elevation > 0 && map[i][j].elevation < 75) {
						for (int a = -1; a < 2; a++) {
							for (int b = -1; b < 2; b++) {
								if (map[safeC(i + a)][safeC(j + b)].elevation > 0 && map[safeC(i + a)][safeC(j + b)].elevation < 75) {
									avg += map[safeC(i + a)][safeC(j + b)].rain;
									num++;
									if (a == 0 && b == 0) {
										avg += map[safeC(i + a)][safeC(j + b)].rain;
										num++;
									}
								}
							}
						}
						avg /= num;
						map[i][j].rain = (avg * (.99 + ((((double)rand() / RAND_MAX)) * .02)));
					}
					if (h == 0) { j++; }
					else { j--; }
				}
				if (h == 0) { i++; }
				else { i--; }
			}
		}
	}
}


void elevSpread(std::vector< std::vector<MapTile>>& map) {
	for (int m = 4; m > 0; m--) {
		for (int h = 1; h >= 0; h--) {
			int i = h * (MAPSIZE - 1);
			while (i < MAPSIZE && i >= 0) {
				int j = h * (MAPSIZE - 1);
				while (j < MAPSIZE && j >= 0) {
					int Min = 100;
					int Max = -100;
					float avg = 0;
					for (int a = -1; a < 2; a++) {
						for (int b = -1; b < 2; b++) {
							if (map[safeC(i + a)][safeC(j + b)].elevation > Max) { Max = map[safeC(i + a)][safeC(j + b)].elevation; }
							if (map[safeC(i + a)][safeC(j + b)].elevation < Min) { Min = fmax(0, map[safeC(i + a)][safeC(j + b)].elevation); }
							if (map[i][j].elevation > 0) {
								avg += fmax(0, map[safeC(i + a)][safeC(j + b)].elevation);
							}
							else {
								avg += fmin(0, (int)map[safeC(i + a)][safeC(j + b)].elevation);
							}
						}
					}

					if (map[i][j].elevation > 0) {
						avg /= 9.0;
						
						if (map[i][j].elevation >= 60) {
							map[i][j].elevation = rand() % 3 + ((Max + 4 * avg) / 5);
						}
						else if (map[i][j].elevation >= 40) {
							map[i][j].elevation = (rand() % 3 + ((Max + 2 * avg) / 3 * (.91)));
						}
						else if (Min < 20) {
							map[i][j].elevation = rand() % 5 + (Max + map[i][j].elevation + Min) / 3;
						}
						else {
							map[i][j].elevation = rand() % 3 + (.93 * (Max + avg) / 2);
						}
					}
					else {
						avg += 2*min(Max, 0);
						avg /= 11.0;
						map[i][j].elevation = avg;
					}
					if (h == 0) { j++; }
					else { j--; }
				}
				if (h == 0) { i++; }
				else { i--; }
			}
		}
	}
}

void forestSpread(std::vector< std::vector<MapTile>>& map) {
	for (int m = 4; m > 0; m--) {
		for (int h = 1; h >= 0; h--) {
			int i = h * (MAPSIZE - 1);
			while (i < MAPSIZE && i >= 0) {
				int j = h * (MAPSIZE - 1);
				while (j < MAPSIZE && j >= 0) {
					float avg = 0;
					float num = 0;
					if (map[i][j].elevation > 0 && map[i][j].elevation < 75) {
						for (int a = -1; a < 2; a++) {
							for (int b = -1; b < 2; b++) {
								if (map[safeC(i + a)][safeC(j + b)].elevation > 0 && map[safeC(i + a)][safeC(j + b)].elevation < 75) {
									avg += map[safeC(i + a)][safeC(j + b)].forestChance;
									num++;
									if (a == 0 && b == 0) {
										avg += map[safeC(i + a)][safeC(j + b)].forestChance;
										num++;
									}
								}
							}
						}
						avg /= num;
						map[i][j].forestChance = (avg * (.99 + ((((double)rand() / RAND_MAX)) * .02)));
					}
					else {
						map[i][j].forestChance = 75;
					}
					if (h == 0) { j++; }
					else { j--; }
				}
				if (h == 0) { i++; }
				else { i--; }
			}
		}
	}
}

void currents(std::vector<std::vector<MapTile>>& map, bool fir, int id) {
	float ticks = 0;
	auto makeCurrent = [&](int yC, int xC) {
		int limit = 40;
		float water = 20 + latTemp(yC) / 2;
		int y = safeC(yC);
		int x = safeC(xC);
		int y1;
		int x1;
		int dir = 0;
		float temp = fmax(0, map[y][x].heat);
		int lastDir = 0;
		int bannedDir = -1;
		float cS = 0;
		MapTile* t;
		if (!fir) {
			water *= 3;
		}
		while (cS < limit && water > 0) {
			ticks++;
			y = safeC(y);
			x = safeC(x);
			MapTile til = map[y][x];
			if ((til.type == 'w' || til.type == 'a' || til.type == 'r')) {
				water += 1 + til.heat / 35;
				cS += .1;
			}
			for (int a = -2; a < 2; a++) {
				for (int b = -2; b < 2; b++) {
					if (a != 0 || b == 0) {
						y1 = safeC(y + a);
						x1 = safeC(x + b);
						if (map[y1][x1].elevation > 0) {
							t = &map[y1][x1];
							if (t->heat > temp) { //Colder current than land. Decrease land temp
								t->heat -= (100 - temp) / 1200;
								temp += .01;
								t->rain += .02;
								t->forestChance += .01;
							}
							else if (t->heat < temp) {  //Hot current over cold land. increase land temp
								t->heat += .04 + temp / 1000;
								temp -= .005;
								water -= .01;
							}

							float rainfall = 0.2;
							if (!fir) {
								rainfall *= 2;
							}
							t->rain += rainfall;
							water -= (.05 + t->heat / 8000.0);
							if (t->heat > 2) {
								t->heat *= 1 - t->elevation / 40000.0;
							}
							if (t->elevation >= 55) { //Make mountains colder, and lose water when traversing mountains
								water -= (t->elevation / (700.0));
								if (t->heat > 2) {
									t->heat *= 1 - t->elevation / 80000.0;
								}
							}
						}
					}
				}
			}
			int maxY = -1;
			int maxX = 0;
			//stay course
			if (map[y][x].elevation <= 0 && rand() % limit < cS) {
				for (int a = -1; a < 2; a++) {
					for (int b = -1; b < 2; b++) {
						if (a * b == 0 && a != b) {
							if (maxY == -1 || (int)map[safeC(y + a)][safeC(x + b)].elevation > (int)map[maxY][maxX].elevation) {
								maxY = safeC(y + a);
								maxX = safeC(x + b);
							}
							else if (rand() % 2 == 0 || (int)map[safeC(y + a)][safeC(x + b)].elevation == (int)map[maxY][maxX].elevation) {
								if (rand() % 2) {
									maxY = safeC(y + a);
									maxX = safeC(x + b);
								}
							}
						}
					}
				}
				if (map[maxY][maxX].elevation > 0) {
					if (maxX > x) {
						bannedDir = 3;
					}
					if (maxY > y) {
						bannedDir = 2;
					}
					if (bannedDir == -1 || rand() % 2 == 0) {
						if (maxX < x) {
							bannedDir = 1;
						}
						if (maxY < y) {
							bannedDir = 0;
						}
					}
				}
				y = maxY;
				x = maxX;
			}
			else {
				x += 1 - rand() % 3;
				y += 1 - rand() % 3;
			}
		}
	};
	int count = 0;
	float avgLen = 0;
	for (int i = id; i < MAPSIZE; i += NUM_THREADS) {
		for (int j = id; j < MAPSIZE; j += NUM_THREADS) {
			switch (map[i][j].type) {
			case 'w': if (fir) {
				makeCurrent(i, j);
				makeCurrent(i, j);
			} break;
			case 'a': case 'r': if (!fir) { makeCurrent(i, j); } break;
			}
		}
	}
}
void rivers(std::vector< std::vector<MapTile>>& map) {
	int id = 0;
	int temp = 0;
	auto makeRiver = [&](int yC, int xC) {
		int interval = 80.0 * MODI + 1;
		std::vector<std::vector<int>> stack;
		int stop = false;
		stack.push_back({ yC, xC });
		int lastDir = -1;
		int lastlast = -1;
		int bannedDir = -1;
		int rS = 0;
		std::deque<std::deque<int>> river;
		int minDir = -1;
		int y = -1; int x = -1;
		int min = 100;
		while (stack.size() > 0 && !stop) {
			y = safeC(stack.back()[0]);
			x = safeC(stack.back()[1]);
			stack.pop_back();
			river.push_back({ y, x, rS });
			if (map[y][x].elevation > 0) {
				min = map[y][x].elevation + 5;
				map[y][x].id[0] = id;
				map[y][x].id[1] = rS;
				minDir = -1;
				if (map[safeC(y - 1)][x].elevation <= min && lastDir != 0 && lastlast != 0 && bannedDir != 0 && map[safeC(y - 1)][x].id[0] != id) {
					min = map[safeC(y - 1)][x].elevation;
					minDir = 0;
				}
				if (map[y][safeC(1 + x)].elevation <= min && lastDir != 1 && lastlast != 1 && bannedDir != 1 && map[y][safeC(1 + x)].id[0] != id) {
					min = map[y][safeC(1 + x)].elevation;
					minDir = 1;
				}
				if (map[safeC(y + 1)][x].elevation <= min && lastDir != 2 && lastlast != 2 && bannedDir != 2 && map[safeC(y + 1)][x].id[0] != id) {
					min = map[safeC(y + 1)][x].elevation;

					minDir = 2;
				}
				if (map[y][safeC(x - 1)].elevation <= min && lastDir != 3 && lastlast != 3 && bannedDir != 3 && map[y][safeC(x - 1)].id[0] != id) {
					min = map[y][safeC(x - 1)].elevation;
					minDir = 3;
				}
				if (minDir > -1) {
					lastlast = lastDir;
					if (minDir == 0) { stack.push_back({ y - 1, x }); }
					else if (minDir == 1) { stack.push_back({ y, x + 1 }); }
					else if (minDir == 2) { stack.push_back({ y + 1, x }); }
					else if (minDir == 3) { stack.push_back({ y, x - 1 }); }
					if (minDir < 2) { lastDir = minDir + 2; }
					else { lastDir = minDir - 2; }

					if (rS % interval == 0) {
						bannedDir = rand() % 4;
					}
				}
			}
			else { stop = true; }
			rS++;
		}
		if (stop && rS > 3) {
			std::vector<std::vector<int>> keep;
			int y = safeC(river.front()[0]);
			int x = safeC(river.front()[1]);
			river.pop_front();
			int cur = 0;
			int dir = -1;
			keep.push_back({ y, x });
			while (cur < rS) {
				dir = -1;
				if ((map[safeC(y - 1)][x].id[0] > -1 && map[safeC(y - 1)][x].id[0] < map[safeC(y - 1)][x].id[0]) || (map[safeC(y - 1)][x].id[0] == id && map[safeC(y - 1)][x].id[1] >= cur)) {
					dir = 0;
					cur = map[safeC(y - 1)][x].id[1];
				}
				if ((map[y][safeC(x + 1)].id[0] > -1 && map[y][safeC(x + 1)].id[0] < map[y][safeC(x + 1)].id[0]) || (map[y][safeC(x + 1)].id[0] == id && map[y][safeC(x + 1)].id[1] >= cur)) {
					dir = 1;
					cur = map[y][safeC(x + 1)].id[1];
				}
				if ((map[safeC(y + 1)][x].id[0] > -1 && map[safeC(y + 1)][x].id[0] < map[safeC(y + 1)][x].id[0]) || (map[safeC(y + 1)][x].id[0] == id && map[safeC(y + 1)][x].id[1] >= cur)) {
					dir = 2;
					cur = map[safeC(y + 1)][x].id[1];
				}
				if ((map[y][safeC(x - 1)].id[0] > -1 && map[y][safeC(x - 1)].id[0] < map[y][safeC(x - 1)].id[0]) || (map[y][safeC(x - 1)].id[0] == id && map[y][safeC(x - 1)].id[1] >= cur)) {
					dir = 3;
					cur = map[y][safeC(x - 1)].id[1];
				}
				switch (dir) {
				case 0: keep.push_back({ y - 1, x }); y = safeC(y - 1); break;
				case 1: keep.push_back({ y, x + 1 }); x = safeC(x + 1); break;
				case 2: keep.push_back({ y + 1, x }); y = safeC(y + 1); break;
				case 3: keep.push_back({ y, x - 1 }); x = safeC(x - 1); break;
				}
				cur++;
			}
			int kepL = keep.size();
			for (int a = 0; a < kepL; a++) {
				int y = safeC(keep[a][0]);
				int x = safeC(keep[a][1]);
				map[y][x].type = 'r';
				map[y][x].elevation = -100;
				map[y][x].rain = 260;
				map[y][x].id[2] = map[y][x].id[0];
			}
		}
	};
	for (int i = 0; i < MAPSIZE; i++) {
		//std::cout << i << std::endl;
		for (int j = 0; j < MAPSIZE; j++) {
			if (map[i][j].elevation > 25 && map[i][j].elevation < 75) {
				int chance = 990 - min(map[i][j].rain / 20, 20);
				if ((rand() % 1000) > chance) {
					makeRiver(i, j);
					id++;
				}
			}
		}
	}

}
void lakes(std::vector< std::vector<MapTile>>& map) {
	int id = 0;
	auto makeLake = [&](int yC, int xC) {
		std::deque<std::deque<int>> stack;
		int stop = false;
		stack.push_back({ yC, xC });
		double min = 0;
		while (stack.size() > 0) {
			int y = safeC(stack.front()[0]);
			int x = safeC(stack.front()[1]);
			stack.pop_front();
			if (map[y][x].elevation > 0) {
				min = map[y][x].elevation + 1;
				if (min > 15) {
					map[y][x].type = 'a';
					map[y][x].rain = 260;
					map[y][x].id[0] = id;
					map[y][x].id[2] = id;
					map[y][x].elevation = -100;
					if (map[safeC(y - 1)][x].elevation < min && map[safeC(y - 1)][x].id[0] != id) {
						stack.push_back({ y - 1, x });
						map[safeC(y - 1)][x].id[0] = id;
					}
					if (map[y][safeC(1 + x)].elevation < min && map[y][safeC(1 + x)].id[0] != id) {
						stack.push_back({ y, x + 1 });
						map[y][safeC(x + 1)].id[0] = id;
					}
					if (map[safeC(y + 1)][x].elevation < min && map[safeC(y + 1)][x].id[0] != id) {
						stack.push_back({ y + 1, x });
						map[safeC(y + 1)][x].id[0] = id;
					}
					if (map[y][safeC(x - 1)].elevation < min && map[y][safeC(x - 1)].id[0] != id) {
						stack.push_back({ y, x - 1 });
						map[y][safeC(x - 1)].id[0] = id;
					}
				}
			}
		}
	};
	for (int i = 0; i < MAPSIZE; i++) {
		for (int j = 0; j < MAPSIZE; j++) {
			if (map[i][j].elevation > 35 && map[i][j].elevation < 40) {
				double chance = 1.003 - fmin(map[i][j].rain / 60000, .005);
				if (((double)rand() / RAND_MAX) > chance) {
					makeLake(i, j);
					id++;
				}
			}
		}
	}
}

void genOceans(int oceanSize, int numOceans, std::vector< std::vector<MapTile>>& map) {
	int oS = oceanSize;
	int nO = numOceans;
	int dir = 0;
	int lastDir = 0;
	int yCoord = (int)floor((((double)rand() / RAND_MAX)) * MAPSIZE);
	int xCoord = (int)floor((((double)rand() / RAND_MAX)) * MAPSIZE);
	while (oS > 0 && nO > 0)
	{
		if (oS == 1)
		{
			nO--;
			oS = oceanSize; //Ocean size is too small, go to new location to begin making ocean
			yCoord = (int)floor((((double)rand() / RAND_MAX)) * MAPSIZE);
			xCoord = (int)floor((((double)rand() / RAND_MAX)) * MAPSIZE);
		}
		else
		{
			yCoord = safeC(yCoord);
			xCoord = safeC(xCoord);
			map[yCoord][xCoord].type = 'w';
			map[yCoord][xCoord].elevation = -100;
			map[yCoord][xCoord].rain = 300;
			do {
				dir = rand() % 4;
			} while (dir == lastDir);
			if (dir == 0) { yCoord++; }
			else if (dir == 1) { xCoord++; }
			else if (dir == 2) { yCoord--; }
			else { xCoord--; }
			if (lastDir < 2) { lastDir = dir + 2; }
			else { lastDir = dir - 2; } //Asign Last Dir
			oS--;
		}
	}
}

void slap(std::vector< std::vector<MapTile>>& map, int minSize, int maxSize, char typeDetect, char typeMake, double elevChange, int nind) {
	int ind = 0;
	int counter = 0;
	bool checking = true;
	std::vector<std::vector<int>> stack;
	auto checkValidity = [&](int y, int x, bool checking) {
		MapTile a = map[safeC(y)][safeC(x)];
		//std::cout << a.checked << std::endl;
		return (a.checked != nind || !checking) && a.type == typeDetect;
	};
	auto slapInner = [&](int yC, int xC, bool checking) {
		stack.push_back({ yC, xC });
		while (stack.size() > 0) {
			counter++;
			int y = safeC(stack.back()[0]);
			int x = safeC(stack.back()[1]);

			stack.pop_back();

			if (counter % 100 == 0) {
				//std::cout << typeDetect << std::endl;
			}
			if (checking) { map[y][x].checked = nind; }
			else {
				map[y][x].type = typeMake;
				if (elevChange != 2) {
					map[y][x].elevation = elevChange;
				}
			}

			if (checkValidity(y + 1, x, checking)) {
				stack.push_back({ y + 1, x });
			}
			if (checkValidity(y, x + 1, checking)) {
				stack.push_back({ y, x + 1 });
			}
			if (checkValidity(y - 1, x, checking)) {
				stack.push_back({ y - 1, x });
			}
			if (checkValidity(y, x - 1, checking)) {
				stack.push_back({ y, x - 1 });
			}
		}
	};
	int i = MAPSIZE;
	int j = MAPSIZE;
	while (--i >= 0) {

		while (--j >= 0) {

			if (checkValidity(i, j, true)) {
				counter = 0;
				slapInner(i, j, true);
				if (counter <= maxSize && counter >= minSize) {
					slapInner(i, j, false);
				}
			}
		}
		j = MAPSIZE;
	}
}

void thiccify_forests(std::vector< std::vector<MapTile>>& map) {
	int treeChance = 0;
	for (int m = 4; m > 0; m--) {
		for (int h = 1; h >= 0; h--) {
			int i = h * (MAPSIZE - 1);
			while (i < MAPSIZE && i >= 0) {
				int j = h * (MAPSIZE - 1);
				while (j < MAPSIZE && j >= 0) {
					if (map[i][j].elevation > 0 && map[i][j].forest == 'e' && map[i][j].elevation < 75) {
						treeChance = 0;
						for (int a = -1; a < 2; a++) {
							for (int b = -1; b < 2; b++) {
								if (a * b == 0 && a != b) {
									if (map[safeC(i + a)][safeC(j + b)].type == 'a' || map[safeC(i + a)][safeC(j + b)].type == 'r') {
										treeChance += 3;
										map[i][j].forestChance += 4;
									}
									if (map[safeC(i + a)][safeC(j + b)].forest != 'e') {
										treeChance += 3;
									}
								}
							}
						}
						if (map[i][j].elevation >= 50) {
							treeChance -= (map[i][j].elevation - 50);
						}
						if (rand() % 100 < treeChance) {
							if (map[i][j].rain < 250) {
								map[i][j].forest = 'l';
							}
							else if (map[i][j].heat < 35) {
								map[i][j].forest = 'b';
							}
							else if (map[i][j].heat < 80) {
								map[i][j].forest = 'f';
							}
							else {
								map[i][j].forest = 'n';
							}
						}

					}
					if (h == 0) { j++; }
					else { j--; }
				}
				if (h == 0) { i++; }
				else { i--; }
			}
		}
	}
}

void finalize(std::vector< std::vector<MapTile>>& map) {
	MapTile t;
	bool stop = false;
	for (int i = 0; i < MAPSIZE; i++) {
		for (int j = 0; j < MAPSIZE; j++) {
			/*case 'w': til.type = OCEAN; break;
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
			case 'e': til.type = SHALLOW; break;*/

			t = map[i][j];
			if (t.elevation > 0) {
				if (t.rain < 100) {
					map[i][j].type = 'd';
				}
				else if (t.rain < 125) {
					map[i][j].type = 'q';
				}
				else if (t.rain < 150) {
					map[i][j].type = 't';
				}
				else if (t.rain < 175) {
					map[i][j].type = 'p';
				}
				else if (t.rain < 200) {
					map[i][j].type = 'v';
				}
				else if (t.rain < 225) {
					map[i][j].type = 'o';
				}
				else if (t.rain < 250) {
					map[i][j].type = 'g';
				}
				else {
					map[i][j].type = 'm';
				}
				
				/*//Cold Biomes
				if (map[i][j].heat < 30) {
					if (map[i][j].heat < 5) {
						map[i][j].type = 'c';
					}
					else if (map[i][j].heat < 10) {
						if (map[i][j].rain > 230) {
							map[i][j].type = 'o';
						}
						else {
							map[i][j].type = 't';
						}
					}
					else {
						if (map[i][j].rain < 190) {
							map[i][j].type = 'd';
							if (map[i][j].heat < 50) {
								map[i][j].type = 'q';
							}
						}
						else if (map[i][j].rain < 240) {
							map[i][j].type = 'p';
						}
						else if (map[i][j].rain < 280) {
							map[i][j].type = 'g';
						}
						else {
							map[i][j].type = 'm';
						}
					}
				}
				// Grasslands
				else if (map[i][j].heat < 50) {
					if (map[i][j].heat > 25 && map[i][j].rain < 190) {
						map[i][j].type = 'd';
					}
					else if (map[i][j].rain < 240) {
						map[i][j].type = 'p';
					}
					else if (map[i][j].rain < 260) {
						map[i][j].type = 'g';
					}
					else {
						map[i][j].type = 'm';
					}
				}
				//Hot Biomes
				else {
					if (map[i][j].rain < 190) {
						map[i][j].type = 'd';
					}
					else if (map[i][j].rain < 240) {
						map[i][j].type = 'v';
					}
					else if (map[i][j].rain < 260) {
						map[i][j].type = 'g';
					}
					else {
						map[i][j].type = 'm';
					}
				}
				MapTile t = map[i][j];
				int treeChance = -100 + t.forestChance * max(.5, min(1.25, min(200.0 + pow(latTemp(i) / 50, 7), map[i][j].rain) / (200.0 + t.elevation / 2 + t.heat / 4)));
				map[i][j].forestChance = (t.rain / 4 + t.forestChance / 2 + t.heat / 8) * max(.1, fmin(300.0, map[i][j].rain) / (275.0 + t.elevation));
				if (t.rain > 300) {
					map[i][j].forestChance *= .9;
				}
				if (treeChance > 0) {
					treeChance = 95;
				}
				if (map[i][j].rain > 195) {
					treeChance = max(treeChance, 4);
				}
				if (rand() % 100 < treeChance) {
					if (map[i][j].rain < 250) {
						map[i][j].forest = 'l';
					}
					else if (map[i][j].heat < 35) {
						map[i][j].forest = 'b';
					}
					else if (map[i][j].heat < 80) {
						map[i][j].forest = 'f';
					}
					else {
						map[i][j].forest = 'n';
					}
				}
				*/
			}
			else if (map[i][j].type == 'w') {
				bool stop = false;
				for (int a = -5; a <= 5; a++) {
					for (int b = -5; b <= 5; b++) {
						if (a != 0 || b != 0) {
							if (map[safeC(i + a)][safeC(j + b)].elevation > 0) {
								if (fDist(i, j, safeC(i + a), safeC(j + b)) <= 4) {
									stop = true;
									break;
								}
							}
						}
					}
				}
				if (!stop) {
					map[i][j].type = 'e';
				}
			}
		}
	}
}

std::vector<std::vector<Tile>> clearMap(Tile t) {
	std::vector<std::vector<Tile>> map;
	map.resize(MAPSIZE);
	for (int i = 0; i < MAPSIZE; i++) {
		map[i].resize(MAPSIZE);
	}
	for (int i = 0; i < MAPSIZE; i++) {
		for (int j = 0; j < MAPSIZE; j++) {
			map[i][j] = t;
		}
	}
	return map;
}

std::vector<std::vector<Tile>> createMap(int seed, int num_tries = 0) {
	srand(seed);
	bool makeMountains = true;
	std::vector<std::vector<MapTile>> map = {};

	map.resize(MAPSIZE);
	for (int i = 0; i < MAPSIZE; i++) {
		map[i].resize(MAPSIZE);
	}

	for (int i = 0; i < MAPSIZE / 5; i++) {
		int x = (MAPSIZE * .3) + rand() % MAPSIZE * .4;
		int y = (MAPSIZE * .3) + rand() % MAPSIZE * .4;
		bool fill = rand() % 2;
		if (fill) {
			for (int a = -1; a < 2; a++) {
				for (int b = -1; b < 2; b++) {
					map[safeC(y + a)][safeC(x + b)].elevation = 60;
				}
			}
		}
		else {
			map[y][x].elevation = 80;
		}
	}
	elevSpread(map);
	elevSpread(map);
	elevSpread(map);

	for (int i = 0; i < MAPSIZE; i++) {
		for (int j = 0; j < MAPSIZE; j++) {
			map[i][j].elevation -= 28;
			if (map[i][j].elevation <= 0) {
				map[i][j].type = 'w';
				map[i][j].elevation = -100;
			}
			map[i][j].heat = latTemp(i);
			map[i][j].forestChance = 50 + rand() % 50;
			if (rand() % MAPSIZE == 0) {
				map[i][j].forestChance = 150;
			}
		}
	}

	//genOceans(MAPSIZE * MAPSIZE, 1, map);
	slap(map, 0, 3, 'g', 'w', -100, 0);
	slap(map, 0, 10, 'w', 'a', -100, 1);

	hill(map, 100, 1 + MAPSIZE / 10, MAPSIZE / 10);
	hill(map, 60, 1 + MAPSIZE / 20, MAPSIZE / 10);


	elevSpread(map);
	forestSpread(map);

	for (int i = 0; i < 4; i++) {
		currents(map, false, i);
	}
	lakes(map);
	rivers(map);
	for (int i = 0; i < 4; i++) {
		currents(map, true, i);
	}

	//rainSpread(map);
	finalize(map);
	//thiccify_forests(map);

	std::vector<std::vector<Tile>> gameMap;
	gameMap.resize(MAPSIZE);
	for (int i = 0; i < MAPSIZE; i++) {
		gameMap[i].resize(MAPSIZE);
	}
	for (int i = 0; i < MAPSIZE; i++) {
		for (int j = 0; j < MAPSIZE; j++) {
			if (map[i][j].elevation < 1 && !(map[i][j].type == 'w' || map[i][j].type == 'e' || map[i][j].type == 'a' || map[i][j].type == 'r')) {
				map[i][j].elevation = 1;
			}
			if (map[i][j].elevation < 0) {
				if (rand() % 12 == 0) {
					if (map[i][j].type == 'a' || map[i][j].type == 'r' || rand() % 4 == 0) {
						bool adj_fish = false;
						for (int a = -4; a < 5; a++) {
							for (int b = -4; b < 5; b++) {
								if (map[safeC(i + a)][safeC(j + b)].forest == 's') {
									adj_fish = true;
								}
							}
						}
						if (!adj_fish) {
							map[i][j].forest = 's';
						}
					}
				}
			}

			gameMap[i][j] = Terrain_Convert(map[i][j]);
		}
	}
	std::cout << "All Done!" << std::endl;

	if (num_tries < 10) {
		int num_problems = 0;
		for (int i = 0; i < MAPSIZE; i++) {
			if (map[0][i].elevation >= FLAT || map[i][0].elevation >= FLAT || map[MAPSIZE - 1][i].elevation >= FLAT || map[i][MAPSIZE - 1].elevation >= FLAT) {
				num_problems++;
			}
		}
		if (num_problems > MAPSIZE/10) {
			std::cout << "Map Failed: " << num_tries << std::endl;
				return createMap(rand(), num_tries + 1);
		}
	}

	return gameMap;
}

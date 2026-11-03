#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#include <stpdmem.h>

#include <ncurses.h>

#define randi(m, M) ((rand() % ((m) - (M))) + (m))
#define imin(a, b) ((a) > (b) ? (b) : (a))
#define imax(a, b) ((a) > (b) ? (a) : (b))
#define iclamp(i, m, M) (imin(imax((i), (m)), (M)))

typedef enum {
	STATE_STOP,
	STATE_BUILD,
	STATE_RUN
} State;

typedef struct {
	int x;
	int y;
} Cell;

typedef enum {
	BUILD_PIPE,
	BUILD_TUNNEL,
	BUILD_SPLITTER,
	BUILD_ASSEMBLER,
	BUILD_SMELTER,
	BUILD_MINER_IRON,
	BUILD_MINER_COPPER,
	BUILD_MINER_COAL
} BuildState;

typedef enum {
	ORE_NONE,
	ORE_IRON,
	ORE_COPPER,
	ORE_COAL
} Ore;

typedef enum {
	PIPE_NONE,
	PIPE_OCCUPIED,
	PIPE_RIGHT,
	PIPE_DOWN,
	PIPE_LEFT,
	PIPE_UP,
	PIPE_SPLIT,
	TUNNEL_ENTER_RIGHT,
	TUNNEL_EXIT_RIGHT,
	TUNNEL_ENTER_LEFT,
	TUNNEL_EXIT_LEFT
} Pipe;

typedef struct {
	Cell cell;
	BuildState building;
	int rotation;
} Cursor;

typedef struct {
	Cell anchor; //top left
	int width;
	int height;
	int output;
} Building;

typedef struct {
	int width;
	int height;
	int exit;
	uint8_t* oreMap; //cast Ore
	uint8_t* pipeMap; //cast Pipe
	int buildingCount;
	Building* buildings;
} World;

World* initWorld(int width, int height) {
	World* world = smock(sizeof(World) + 
	                     width * height * sizeof(uint8_t) +
	                     width * height * sizeof(uint8_t));
	world->oreMap = (uint8_t*) world + sizeof(World);
	world->pipeMap = (uint8_t*) world->oreMap + width * height * sizeof(uint8_t);
	world->buildings = NULL; //to be realloced later
	world->width = width;
	world->height = height;
	world->exit = 0; //to be assighned later
	world->buildingCount = 0;
}

void cleanupWorld(World* world) {
	see(world->buildings);
	see(world);
}

bool inPatch(Cell* patch, Cell cell, int size) {
	for(int i = 0; i < size; i++) {
		if(patch[i].x == cell.x && patch[i].y == cell.y) {
			return true;
		}
	}
	return false;
}

void generateOre(uint8_t* oreMap, int w, int h, Ore ore, int n) {
	Cell patch[n];
	patch[0] = (Cell) {randi(0, w), randi(0, h)};
	for(int i = 1; i < n; ) {
		Cell cell = patch[randi(0, i)];
		switch(randi(0, 3)) {
			case 0:
				cell.x++;
			break;
			case 1:
				cell.y++;
			break;
			case 2:
				cell.x--;
			break;
			case 3:
				cell.y--;
			break;
		}
		cell.x = iclamp(cell.x, 0, w - 1); 
		cell.y = iclamp(cell.y, 0, h - 1); 
		if(!inPatch(patch, cell, i)) {
			patch[i] = cell;
			i++;
		}
	}
	for(int i = 0; i < n; i++) {
		oreMap[patch[i].x + patch[i].y * w] = ore;
	}
}

void setupWorld(World* world) {
	for(int i = 0; i < world->width * world->height; i++) {
		world->oreMap[i] = ORE_NONE;
	}
	generateOre(world->oreMap, world->width, world->height, ORE_IRON,   randi(10, 13));
	generateOre(world->oreMap, world->width, world->height, ORE_COPPER, randi(10, 13));
	generateOre(world->oreMap, world->width, world->height, ORE_COAL,   randi(6 , 8 ));
	
	world->exit = randi(0, world->width * 2 + world->height * 2);
	
	for(int i = 0; i < world->width * world->height; i++) {
		if(world->oreMap[i] == ORE_NONE) {
			world->pipeMap[i] = PIPE_NONE;
		} else {
			world->pipeMap[i] = PIPE_OCCUPIED;
		}
	}
}

#include "draw.c"

bool input(int c, Cursor* cursor, World* world) {
	switch(c) {
		case KEY_UP: case 'k':
			cursor->cell.y--;
			cursor->cell.y = imax(0, cursor->cell.y);
		break;
		case KEY_DOWN: case 'j':
			cursor->cell.y++;
			cursor->cell.y = imin(world->height - 1, cursor->cell.y);
		break;
		case KEY_LEFT: case 'h':
			cursor->cell.x--;
			cursor->cell.x = imax(0, cursor->cell.x);
		break;
		case KEY_RIGHT: case 'l':
			cursor->cell.x++;
			cursor->cell.x = imin(world->width - 1, cursor->cell.x);
		break;
		case 'q':
			cursor->building = BUILD_PIPE;
		break;
		case 'w':
			cursor->building = BUILD_TUNNEL;
		break;
		case 'e':
			cursor->building = BUILD_SPLITTER;
		break;
		case 'a':
			cursor->building = BUILD_ASSEMBLER;
		break;
		case 's':
			cursor->building = BUILD_SMELTER;
		break;
		case 'd':
			//cursor->building = BUILD_MINER;
		break;
		case 'r':
			cursor->rotation++;
		break;
		case 'f':
			int x = cursor->cell.x;
			int y = cursor->cell.y;
			if(cursor->building == BUILD_PIPE || cursor->building == BUILD_TUNNEL || cursor->building == BUILD_SPLITTER) {
				if(cursor->building == BUILD_TUNNEL) {
					if(world->pipeMap[x - 1 + y * world->width] != PIPE_OCCUPIED &&
					   world->pipeMap[x + 1 + y * world->width] != PIPE_OCCUPIED) {
						if(cursor->rotation == 0) {
							world->pipeMap[x - 1 + y * world->width] = TUNNEL_ENTER_LEFT;
							world->pipeMap[x + 1 + y * world->width] = TUNNEL_EXIT_RIGHT;
						} else {
							world->pipeMap[x - 1 + y * world->width] = TUNNEL_EXIT_LEFT;
							world->pipeMap[x + 1 + y * world->width] = TUNNEL_ENTER_RIGHT;
						}
					}
				} else if(world->pipeMap[x + y * world->width] != PIPE_OCCUPIED) {
					if(cursor->building == BUILD_PIPE) {
						world->pipeMap[x + y * world->width] = cursor->rotation + 2;
					} else {
						world->pipeMap[x + y * world->width] = PIPE_SPLIT;
					}
				}
				break;
			}
		break;
		case 'v':
			//delete
		break;
		case 'x':
			return true;
		break;
	}

	switch(cursor->building) {
		case BUILD_PIPE:
			cursor->rotation = cursor->rotation % 4;
		break;
		case BUILD_TUNNEL:
			cursor->rotation = cursor->rotation % 2;
			cursor->cell.x = imin(world->width - 2, imax(1, cursor->cell.x));
		break;
		case BUILD_SPLITTER:
			cursor->rotation = 0;
		break;
	}

	return false;
}

void cleanup(World* world) {
	endwin();
	cleanupWorld(world);
	exit(0);
}

int main(int argc, char** argv) {
	const int width = 18;
	const int height = 10;

	State state = STATE_BUILD;
	World* world = initWorld(width, height);
	Cursor cursor = {};
	
	srand(time(NULL));
	setupWorld(world);
	setupNcurses();
	findAnchor(world);
	
	while(state != STATE_STOP) {
		switch(state) {
			case STATE_BUILD:
				beginDraw();
				drawBase(world);
				drawPipes(world);
				drawBuildings(world);
				drawCursor(&cursor);
				finishDraw();

				int c = getch();
				if(input(c, &cursor, world)) {
					cleanup(world);
				}
				break;
			case STATE_RUN:
				//process factory
				//draw
				//process input
				break;
			default:
				break;
		}
	}
	
	cleanup(world);
}

#include <ncurses.h>

void setupNcurses() {
        initscr();
        cbreak(); //change to raw later maybe
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);
}

void mvvaddstr(int y, int x, char* s) {
	int i = 0;
	while(s[i] != '\0') {
		mvaddch(y + i, x, s[i]);
		i++;
	}
}

void mvvaddstrt(int y, int x, char* s) {
	int i = 0;
	while(s[i] != '\0') {
		if(s[i] != ' ') {
			mvaddch(y + i, x, s[i]);
		}
		i++;
	}
}

void mvaddstrt(int y, int x, char* s) {
	int i = 0;
	while(s[i] != '\0') {
		if(s[i] != ' ') {
			mvaddch(y, x + i, s[i]);
		}
		i++;
	}
}

static int anchorx;
static int anchory;
static int width;
static int height;
void findAnchor(World* world) {
	anchorx = COLS/2 - (world->width * 5)/2;
	anchory = LINES/2 - (world->height * 3)/2;
	if(anchorx - 1 < 0 || anchory - 1 < 0) {
		fprintf(stderr, "screen is to small");
		exit(1);
	}
	width = world->width;
	height = world->height;
}

void beginDraw() {
	erase();
}

void drawEmpty(int e) {
	mvhline(anchory - 1    , anchorx - 1, '~', width * 5 + 2);
	mvhline(anchory + height * 3, anchorx - 1, '~', width * 5 + 2);
	mvvline(anchory - 1, anchorx - 1    , '|', height * 3 + 2);
	mvvline(anchory - 1, anchorx + width * 5, '|', height * 3 + 2);
	
	mvaddch(anchory - 1    , anchorx - 1    , '+');	
	mvaddch(anchory + height * 3, anchorx - 1    , '+');	
	mvaddch(anchory - 1    , anchorx + width * 5, '+');	
	mvaddch(anchory + height * 3, anchorx + width * 5, '+');	
	
	if(e < width) {
		mvaddstr(anchory - 1, anchorx + e * 5 - 1, "|     |");
	} else if(e < width + height) {
		mvvaddstr(anchory + (e - width) * 3 - 1, anchorx + width * 5 , "l   T" );
	} else if(e < width * 2 + height) {
		mvaddstr(anchory + height * 3 , anchorx + (width - 1) * 5 - (e - height - width) * 5 - 1, "|     |");
	} else {
		mvvaddstr(anchory + (height - 1) * 3 - (e - width - width - height) * 3 - 1, anchorx - 1, "l   T" );
	}
}

void drawOres(uint8_t* oreMap) {
	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
			switch((Ore) oreMap[x + y * width]) {
				case ORE_NONE:
					mvaddch(anchory + y * 3 + 1, anchorx + x * 5 + 2, '.');
				break;
				case ORE_IRON:
					if(y % 2 == 0) {
						if(x % 2 == 0) {
							mvaddstrt(anchory + y * 3    , anchorx + x * 5, "\\ \\ \\");
							mvaddstrt(anchory + y * 3 + 1, anchorx + x * 5, "/ / /");
							mvaddstrt(anchory + y * 3 + 2, anchorx + x * 5, "\\ \\ \\");
						} else {
							mvaddstrt(anchory + y * 3    , anchorx + x * 5, " \\ \\ ");
							mvaddstrt(anchory + y * 3 + 1, anchorx + x * 5, " / / ");
							mvaddstrt(anchory + y * 3 + 2, anchorx + x * 5, " \\ \\ ");
						}
					} else {
						if(x % 2 == 0) {
							mvaddstrt(anchory + y * 3    , anchorx + x * 5, "/ / /");
							mvaddstrt(anchory + y * 3 + 1, anchorx + x * 5, "\\ \\ \\");
							mvaddstrt(anchory + y * 3 + 2, anchorx + x * 5, "/ / /");
						} else {
							mvaddstrt(anchory + y * 3    , anchorx + x * 5, " / / ");
							mvaddstrt(anchory + y * 3 + 1, anchorx + x * 5, " \\ \\ ");
							mvaddstrt(anchory + y * 3 + 2, anchorx + x * 5, " / / ");
						}
					}
				break;
				case ORE_COPPER:
					if(x % 2 == 0) {
						mvaddstrt(anchory + y * 3    , anchorx + x * 5, "* * *");
						mvaddstrt(anchory + y * 3 + 1, anchorx + x * 5, "* * *");
						mvaddstrt(anchory + y * 3 + 2, anchorx + x * 5, "* * *");
					} else {
						mvaddstrt(anchory + y * 3    , anchorx + x * 5, " * * ");
						mvaddstrt(anchory + y * 3 + 1, anchorx + x * 5, " * * ");
						mvaddstrt(anchory + y * 3 + 2, anchorx + x * 5, " * * ");
					}
				break;
				case ORE_COAL:
					mvaddstrt(anchory + y * 3    , anchorx + x * 5, "#####");
					mvaddstrt(anchory + y * 3 + 1, anchorx + x * 5, "#####");
					mvaddstrt(anchory + y * 3 + 2, anchorx + x * 5, "#####");
				break;
			}
		}
	}
}

void drawBase(World* world) {
	drawEmpty(world->exit);
	drawOres(world->oreMap);
}

void drawPipe(int y, int x, int pipe) {
	switch(pipe) {
		case PIPE_RIGHT:
			mvaddstrt(y    , x, "   \\ ");
		       	mvaddstrt(y + 1, x, "----}");
			mvaddstrt(y + 2, x, "   / ");
		break;
		case PIPE_DOWN:
			mvaddstrt(y    , x, "  |  ");
		       	mvaddstrt(y + 1, x, "\\ | / ");
			mvaddstrt(y + 2, x, " 'v' ");
		break;
		case PIPE_LEFT:
			mvaddstrt(y    , x, " /   ");
		       	mvaddstrt(y + 1, x, "{----");
			mvaddstrt(y + 2, x, " \\   ");
		break;
		case PIPE_UP:
			mvaddstrt(y    , x, " ,^, ");
		       	mvaddstrt(y + 1, x, "/ | \\");
			mvaddstrt(y + 2, x, "  |  ");
		break;
		case TUNNEL_ENTER_LEFT:
			mvaddstrt(y    , x, "    ]");
		       	mvaddstrt(y + 1, x, "----]");
			mvaddstrt(y + 2, x, "    ]");
		break;
		case TUNNEL_ENTER_RIGHT:
			mvaddstrt(y    , x, "[    ");
		       	mvaddstrt(y + 1, x, "[----");
			mvaddstrt(y + 2, x, "[    ");
		break;
		case TUNNEL_EXIT_LEFT:
			mvaddstrt(y    , x, " /  ]");
		       	mvaddstrt(y + 1, x, "{---]");
			mvaddstrt(y + 2, x, " \\  ]");
		break;
		case TUNNEL_EXIT_RIGHT:
			mvaddstrt(y    , x, "[  \\ ");
		       	mvaddstrt(y + 1, x, "[---}");
			mvaddstrt(y + 2, x, "[  / ");
		break;
		case PIPE_SPLIT:
			mvaddstrt(y    , x, "  ^  ");
		       	mvaddstrt(y + 1, x, "< 0 >");
			mvaddstrt(y + 2, x, "  v  ");
		break;
		default:
		break;
	}
}

void drawCursor(Cursor* cursor) {
//	mvaddch(anchory + cursor->cell.y * 3 - 1, anchorx + cursor->cell.x * 5 + 2, 'v');
//	mvaddch(anchory + cursor->cell.y * 3 + 1, anchorx + cursor->cell.x * 5 - 1, '>');
//	mvaddch(anchory + cursor->cell.y * 3 + 1, anchorx + cursor->cell.x * 5 + 5, '<');
//	mvaddch(anchory + cursor->cell.y * 3 + 3, anchorx + cursor->cell.x * 5 + 2, '^');
	switch(cursor->building) {
		case BUILD_PIPE:
			drawPipe(anchory + cursor->cell.y * 3, anchorx + cursor->cell.x * 5, cursor->rotation + 2);
		break;
		case BUILD_TUNNEL:
			if(cursor->rotation == 0) {
				drawPipe(anchory + cursor->cell.y * 3, anchorx + cursor->cell.x * 5 - 5, TUNNEL_ENTER_LEFT);
				drawPipe(anchory + cursor->cell.y * 3, anchorx + cursor->cell.x * 5 + 5, TUNNEL_EXIT_RIGHT);
			} else {
				drawPipe(anchory + cursor->cell.y * 3, anchorx + cursor->cell.x * 5 - 5, TUNNEL_EXIT_LEFT);
				drawPipe(anchory + cursor->cell.y * 3, anchorx + cursor->cell.x * 5 + 5, TUNNEL_ENTER_RIGHT);
			}
		break;
		case BUILD_SPLITTER:
			drawPipe(anchory + cursor->cell.y * 3, anchorx + cursor->cell.x * 5, PIPE_SPLIT);
		break;
	}
}

void drawPipes(World* world) {
	for(int x = 0; x < width; x++) {
	for(int y = 0; y < height; y++) {
		drawPipe(anchory + y * 3, anchorx + x * 5, world->pipeMap[x + y * width]);
	}
	}
}

void drawBuildings(World* world) {

}

void finishDraw() {
	refresh();
}

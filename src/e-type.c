/*
 * e-type - Tetris clone for your terminal
 * Copyright (C) 2017  Edgar Mendoza

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* C library */
#include <stdlib.h>
/* e-type */
#include "tetris.h"
#include "log.h"

int  init_ncurses(struct game_state *gs);
void handle_input(struct game_state *gs);

int
main(int argc, char **argv)
{
	/*
	 * I like using a big struct to hold everything because it makes agrument
	 * passing easier to handle since I can just pass the whole thing.
	 */
	struct game_state gs;

	/* Initialize everything */
	log_init("e-type.log");
	srand(time(NULL));
	new_game(&gs);
	init_ncurses(&gs);

	/* Game loop */
	refresh();
	while (!(gs.flags & BIT(QUIT))) {
		draw_game(&gs);
		handle_input(&gs);

		if (!(gs.flags & BIT(PAUSE))) {
			if (gs.flags & BIT(LBREAK)) {
				update_lbreak(&gs);
			} else {
				update_timing(&gs);
			}
		}
	}
	
	endwin();
	return 0;
}

int
init_ncurses(struct game_state *gs)
{
	/* Initialize Ncurses */
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	timeout(0);

	/* Initialize color */
	use_default_colors();
	start_color();
	init_pair(RED, COLOR_RED, -1);
	init_pair(GREEN, COLOR_GREEN, -1);
	init_pair(YELLOW, COLOR_YELLOW, -1);
	init_pair(BLUE, COLOR_BLUE, -1);
	init_pair(MAGENTA, COLOR_MAGENTA, -1);
	init_pair(CYAN, COLOR_CYAN, -1);
	init_pair(WHITE, COLOR_WHITE, -1);

	/* Create windows */
	gs->hold_win = newwin(8, 14, 0, 0);
	gs->board_win = newwin(BOARD_H + 2, BOARD_W * 2 + 2, 0, 14);
	gs->stats_win = newwin(BOARD_H + 2, BOARD_W * 2 + 2, 0, BOARD_W * 2 + 16);

	return 0;
}

/* TODO: Allow for customizable keys */
void
handle_input(struct game_state *gs)
{
	int c;

	c = getch();

	if (gs->flags & BIT(PAUSE)) {
		if (c == 'P' || c == 'p') {
			resume(gs);
		}

	} else {
		switch (c) {
		case 'W':
		case 'w':
			move_mino(gs, 0, -1, SOFT_DROP);
			break;
		case 'S':
		case 's':
			move_mino(gs, 0, 1, SOFT_DROP);
			break;
		case 'A':
		case 'a':
			move_mino(gs, -1, 0, SOFT_DROP);
			break;
		case 'D':
		case 'd':
			move_mino(gs, 1, 0, SOFT_DROP);
			break;
		case 'J':
		case 'j':
			rotate_mino(gs, CLOCKWISE);
			break;
		case 'K':
		case 'k':
			rotate_mino(gs, COUNTER_CLOCKWISE);
			break;
		case 'R':
		case 'r':
			spawn_mino(gs);
			break;
		case 'L':
		case 'l':
			hold_mino(gs);
			break;
		case ' ':
			hard_drop(gs);
			break;
		case 'P':
		case 'p':
			pause(gs);
			break;
		case 'Q':
		case 'q':
			game_over(gs);
			break;
		}
	}
}


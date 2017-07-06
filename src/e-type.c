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

void init_ncurses(void);
void handle_input(game_state *gs);

int
main(int argc, char **argv)
{
	game_state gs;

	init_ncurses();

	srand(time(NULL));

	new_game(&gs);
	/* Game loop */
	while (!(gs.flags & BIT(QUIT))) {
		update_timing(&gs);
		handle_input(&gs);

		if (gs.flags & BIT(DRAW)) {
			draw_board(&gs);
			gs.flags ^= BIT(DRAW);

			refresh();
		}
	}
	
	endwin();
	return 0;
}

void
init_ncurses(void)
{
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	timeout(0);

	/* Init color */
	use_default_colors();
	start_color();
	init_pair(RED, COLOR_RED, -1);
	init_pair(GREEN, COLOR_GREEN, -1);
	init_pair(YELLOW, COLOR_YELLOW, -1);
	init_pair(BLUE, COLOR_BLUE, -1);
	init_pair(MAGENTA, COLOR_MAGENTA, -1);
	init_pair(CYAN, COLOR_CYAN, -1);
	init_pair(WHITE, COLOR_WHITE, -1);
}

void
handle_input(game_state *gs)
{
	switch (getch()) {
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
	case ' ':
		hard_drop(gs);
		break;
	case 'Q':
	case 'q':
		game_over(gs);
		break;
	}
}


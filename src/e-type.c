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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "tetris.h"

void init_ncurses(void);

int
main(int argc, char **argv)
{
	game_state game;

	init_ncurses();

	srand(time(NULL));

	new_game(&game);
	/* Game loop */
	while (!(game.flags & BIT(QUIT))) {
		switch (getch()) {
		case 'W':
		case 'w':
			move_mino(&game, 0, -1);
			break;
		case 'S':
		case 's':
			move_mino(&game, 0, 1);
			break;
		case 'A':
		case 'a':
			move_mino(&game, -1, 0);
			break;
		case 'D':
		case 'd':
			move_mino(&game, 1, 0);
			break;
		case 'R':
		case 'r':
			spawn_mino(&game);
			break;
		case 'Q':
		case 'q':
			game.flags |= BIT(QUIT);
			break;
		}

		if (game.flags & BIT(DRAW)) {
			draw_board(&game);
			game.flags ^= BIT(DRAW);

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
	timeout(0);

	/* Init color */
	start_color();
	init_pair(RED, COLOR_RED, COLOR_BLACK);
	init_pair(GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(BLUE, COLOR_BLUE, COLOR_BLACK);
	init_pair(MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(WHITE, COLOR_WHITE, COLOR_BLACK);
}


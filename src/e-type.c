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
#include <stdint.h>

#include "tetris.h"

int
main(int argc, char **argv)
{
	game_state game;

	/* Initialize ncurses */
	initscr();
	cbreak();
	noecho();
	timeout(0);

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
		case 'Q':
		case 'q':
			game.flags |= BIT(QUIT);
			break;
		}

		if (game.flags & BIT(DRAW)) {
			draw_board(&game);
			refresh();
		}
	}
	
	endwin();
	return 0;
}


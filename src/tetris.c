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

/* Header file */
#include "tetris.h"
/* C library */
#include <string.h>

void
new_game(game_state *game)
{
	memset(game->board[0], 0, sizeof(game->board));
	game->flags = BIT(DRAW);

	game->current_mino.x = BOARD_WIDTH / 2;
	game->current_mino.y = 0;
	game->board[current_mino.y][current_mino.x] = 1;
}

void
draw_board(game_state *game)
{
	int i, j;

	mvprintw(0, 0, " --------------------\n");
	for (i = 0; i != BOARD_HEIGHT; ++i) {
		addch('|');
		for (j = 0; j != BOARD_WIDTH; ++j) {
			printw("%s", game->board[i][j] ? "[]" : "  ");
		}
		printw("|\n");
	}
	printw(" --------------------");
}

void
move_mino(game_state *game, int dx, int dy)
{
	dx += game->current_mino.x;
	dy += game->current_mino.y;

	/* Check if moving mino causes it to go out of bounds */
	if (dx < 0 || dx >= BOARD_WIDTH ||
	    dy < 0 || dy >= BOARD_HEIGHT) {
		return;
	}

	/* Move mino */
	game->board[game->current_mino.y][game->current_mino.x] = 0;
	game->current_mino.x = dx;
	game->current_mino.y = dy;
	game->board[game->current_mino.y][game->current_mino.x] = 1;

	game->flags |= BIT(DRAW);
}


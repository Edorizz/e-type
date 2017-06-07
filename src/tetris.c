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
#include <stdlib.h>

const tetromino minos[7] = { { '<', '>',
			       { { 0, 0 }, { 0, 1 },
				 { 0, 2 }, { 0, 3 } },
			       { 0, 0 },
			       RED,
			       0 },
			     
			     { '{', '}',
			       { { 0, 0 }, { 0, 1 },
				 { 0, 2 }, { 1, 2 } },
			       { 0, 0 },
			       GREEN,
			       0 },
			     
			     { '(', ')',
			       { { 1, 0 }, { 1, 1 },
				 { 1, 2 }, { 0, 2 } },
			       { 0, 0 },
			       YELLOW,
			       0 },
			     
			     { '[', ']',
			       { { 0, 0 }, { 1, 0 },
				 { 0, 1 }, { 1, 1 } },
			       { 0, 0 },
			       BLUE,
			       0 },
			     
			     { '%', '%',
			       { { 0, 0 }, { 1, 0 },
				 { 0, 1 }, { -1, 1 } },
			       { 0, 0 },
			       MAGENTA,
			       0 },
			     
			     { '@', '@',
			       { { 0, 0 }, { -1, 0 },
				 { 0, 1 }, { 1, 1 } },
			       { 0, 0 },
			       CYAN,
			       0 },
			     
			     { '#', '#',
			       { { 0, 0 }, { -1, 0 },
				 { 1, 0 }, { 0, 1 } },
			       { 0, 0 },
			       WHITE,
			       0 } };
	
void
new_game(game_state *game)
{
	memset(game->board[0], 0, sizeof(game->board));
	game->flags = BIT(DRAW);

	spawn_mino(game);
}

void
draw_board(game_state *game)
{
	int i, j, x, y;

	/* Draw board */
	mvprintw(0, 0, "*--------------------*\n");
	for (i = 0; i != BOARD_HEIGHT; ++i) {
		addch('|');
		for (j = 0; j != BOARD_WIDTH; ++j) {
			printw("%s", game->board[i][j] ? "[]" : "  ");
		}
		printw("|\n");
	}
	printw("*--------------------*");

	/* Draw current tetromino */
	attron(COLOR_PAIR(game->mino.color));
	for (i = 0; i != 4; ++i) {
		x = 1 + (game->mino_pos.x + game->mino.block_pos[i].x) * 2;
		y = 1 + game->mino_pos.y + game->mino.block_pos[i].y;

		mvaddch(y, x, game->mino.block_left);
		mvaddch(y, x + 1, game->mino.block_right);
	}
	attroff(COLOR_PAIR(game->mino.color));
}

void
spawn_mino(game_state *game)
{
	game->mino_pos.x = BOARD_WIDTH / 2;
	game->mino_pos.y = BOARD_HEIGHT / 2;

	game->flags |= BIT(DRAW);

	memcpy(&game->mino, &minos[rand() % 7], sizeof(tetromino));
}

void
clear_mino(game_state *game)
{
	int i;

	for (i = 0; i != 4; ++i) {
		game->board[game->mino_pos.y + game->mino.block_pos[i].y]
			[game->mino_pos.x + game->mino.block_pos[i].x] = 0;
	}
}

void
move_mino(game_state *game, int dx, int dy)
{
	int i, x, y;

	for (i = 0; i != 4; ++i) {
		x = game->mino_pos.x + game->mino.block_pos[i].x + dx;
		y = game->mino_pos.y + game->mino.block_pos[i].y + dy;
		
		/* Check if moving mino causes it to go out of bounds */
		if (x < 0 || x >= BOARD_WIDTH ||
		    y < 0 || y >= BOARD_HEIGHT) {
			return;
		}
	}
		
	game->mino_pos.x += dx;
	game->mino_pos.y += dy;

	game->flags |= BIT(DRAW);
}


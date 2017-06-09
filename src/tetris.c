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
			       { { 0, 0 }, { 0, 1 },		/* I */
				 { 0, 2 }, { 0, 3 } },
			       { 0, 2 },
			       RED,
			       0 },
			     
			     { '{', '}',
			       { { 0, 0 }, { 0, 1 },		/* L */
				 { 0, 2 }, { 1, 2 } },
			       { 0, 1 },
			       GREEN,
			       0 },
			     
			     { '(', ')',
			       { { 1, 0 }, { 1, 1 },		/* J */
				 { 1, 2 }, { 0, 2 } },
			       { 1, 1 },
			       YELLOW,
			       0 },
			     
			     { '[', ']',
			       { { 0, 0 }, { 1, 0 },		/* O */
				 { 0, 1 }, { 1, 1 } },
			       { 0, 0 },
			       BLUE,
			       0 },
			     
			     { '%', '%',
			       { { 0, 0 }, { 1, 0 },		/* S */
				 { 0, 1 }, { -1, 1 } },
			       { 0, 0 },
			       MAGENTA,
			       0 },
			     
			     { '@', '@',
			       { { 0, 0 }, { -1, 0 },		/* Z */
				 { 0, 1 }, { 1, 1 } },
			       { 0, 0 },
			       CYAN,
			       0 },
			     
			     { '#', '#',
			       { { 0, 0 }, { -1, 0 },		/* T */
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
	int i, j, x, y, c;

	/* Draw board */
	mvprintw(0, 0, "*--------------------*\n");
	for (i = 0; i != BOARD_HEIGHT; ++i) {
		addch('|');
		for (j = 0; j != BOARD_WIDTH; ++j) {
			if ((c = game->board[i][j])) {
				attron(COLOR_PAIR(c));
				printw("%c%c", minos[c - 1].block_left,
					minos[c - 1].block_right);
				attroff(COLOR_PAIR(c));
			} else {
				printw("%s", "  ");
			}
		}
		printw("|\n");
	}
	printw("*--------------------*");

	/* Draw current tetromino */
	attron(COLOR_PAIR(game->mino.color));
	for (i = 0; i != 4; ++i) {
		x = game->mino_pos.x + game->mino.block_pos[i].x;
		y = game->mino_pos.y + game->mino.block_pos[i].y;

		if (in_range(x, y)) {
			x = x * 2 + 1;
			++y;
			
			mvaddch(y, x, game->mino.block_left);
			mvaddch(y, x + 1, game->mino.block_right);
		}
	}
	attroff(COLOR_PAIR(game->mino.color));
}

int
in_range(int x, int y)
{
	return x >= 0 && x < BOARD_WIDTH && y >= 0 && y < BOARD_HEIGHT;
}

void
line_down(game_state *game, int y)
{
	int i;

	for (i = 0; i != BOARD_WIDTH; ++i) {
		game->board[y + 1][i] = game->board[y][i];
		game->board[y][i] = 0;
	}
}

void
clear_lines(game_state *game)
{
	int i, j;

	for (i = BOARD_HEIGHT - 1; i >= 0; --i) {
		for (j = 0; j != BOARD_WIDTH && game->board[i][j]; ++j)
			;

		if (j == BOARD_WIDTH) {
			for (j = i - 1; j >= 0; --j) {
				line_down(game, j);
			}

			++i;
			continue;
		}
	}
}

void
spawn_mino(game_state *game)
{
	game->mino_pos.x = BOARD_WIDTH / 2;
	game->mino_pos.y = -2;

	game->flags |= BIT(DRAW);

	memcpy(&game->mino, &minos[rand() % 7], sizeof(tetromino));
}

void
move_mino(game_state *game, int dx, int dy)
{
	int i, x, y;

	for (i = 0; i != 4; ++i) {
		x = game->mino_pos.x + game->mino.block_pos[i].x;
		y = game->mino_pos.y + game->mino.block_pos[i].y;
		
		/* Check if moving mino causes it to go out of bounds */
		if (!in_range(x + dx, 0) || y + dy >= BOARD_HEIGHT ||
		    (y >= 0 && game->board[y + dy][x + dx] && !game->board[y][x]) ||
		    dy == -1) {
			if (dx == 0 && dy == 1) {
				for (i = 0; i != 4; ++i) {
					game->board[game->mino.block_pos[i].y + game->mino_pos.y]
						[game->mino.block_pos[i].x + game->mino_pos.x] = game->mino.color;
				}
				clear_lines(game);
				spawn_mino(game);
			}
			return;
		}
	}
		
	game->mino_pos.x += dx;
	game->mino_pos.y += dy;

	game->flags |= BIT(DRAW);
}

void
rotate_mino(game_state *game, int dir)
{
	tetromino tmp;
	point *p;
	int abs_x, abs_y, z;

	memcpy(&tmp, &game->mino, sizeof(tetromino));
	
	for (int i = 0; i != 4; ++i) {
		p = &tmp.block_pos[i];
		p->x -= tmp.pivot.x;
		p->y -= tmp.pivot.y;

		if (dir == CLOCKWISE) {
			z = p->x;
			p->x = -p->y;
			p->y = z;
		} else if (dir == COUNTER_CLOCKWISE) {
			z = p->y;
			p->y = -p->x;
			p->x = z;
		}

		p->x += tmp.pivot.x;
		p->y += tmp.pivot.y;

		abs_x = p->x + game->mino_pos.x;
		abs_y = p->y + game->mino_pos.y;

		if (!in_range(abs_x, abs_y) || game->board[abs_y][abs_x]) {
			return;
		}
	}

	memcpy(&game->mino, &tmp, sizeof(tetromino));
	game->flags |= BIT(DRAW);
}


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

/*
 * This gets applied to the standard Tetris scoring formula
 * '(n * 1) + score_mult[i - 1]' where 'n' is the current level and 'i' the
 * number of lines broken.
 */
const int score_mult[4] = { 40, 100, 300, 1200 };

/*
 * Tetromino specific information, which includes (in order):
 *	- Characters used for opening and closing mino blocks.
 *	- Block coordinates.
 *	- Pivot point.
 *	- Color.
 *	- Special flags. (Mainly rotation)
 */
const tetromino minos[7] = { { '<', '>',
			       { { 0, 0 }, { 0, 1 },		/* I */
				 { 0, 2 }, { 0, 3 } },
			       { 0, 2 },
			       RED,
			       BIT(ROTATE_TWICE) },
			     
			     { '{', '}',
			       { { -1, 0 }, { 0, 0 },		/* L */
				 { 1, 0 }, { -1, 1 } },
			       { 0, 0 },
			       GREEN,
			       0 },
			     
			     { '(', ')',
			       { { -1, 0 }, { 0, 0 },		/* J */
				 { 1, 0 }, { 1, 1 } },
			       { 0, 0 },
			       YELLOW,
			       0 },
			     
			     { '[', ']',
			       { { 0, 0 }, { 1, 0 },		/* O */
				 { 0, 1 }, { 1, 1 } },
			       { 0, 0 },
			       BLUE,
			       BIT(ROTATE_NONE) },
			     
			     { '%', '%',
			       { { 0, 0 }, { 1, 0 },		/* S */
				 { 0, 1 }, { -1, 1 } },
			       { 0, 0 },
			       MAGENTA,
			       BIT(ROTATE_TWICE) },
			     
			     { '@', '@',
			       { { 0, 0 }, { -1, 0 },		/* Z */
				 { 0, 1 }, { 1, 1 } },
			       { 0, 0 },
			       CYAN,
			       BIT(ROTATE_TWICE) },
			     
			     { '#', '#',
			       { { 0, 0 }, { -1, 0 },		/* T */
				 { 1, 0 }, { 0, 1 } },
			       { 0, 0 },
			       WHITE,
			       0 } };
	
void
new_game(game_state *game)
{
	memset(game, 0, sizeof(game_state));
	game->clock = clock();
	game->flags = BIT(DRAW);
	game->fpc = INITIAL_SPEED;

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
	printw("*--------------------*\n");

	/* Draw statistics */
	mvprintw(1, BOARD_WIDTH * 2 + 3, "lines: %d\n", game->lines);
	mvprintw(2, BOARD_WIDTH * 2 + 3, "level: %d\n", game->level);
	mvprintw(3, BOARD_WIDTH * 2 + 3, "score: %d\n", game->score);

	/* Draw current tetromino */
	attron(COLOR_PAIR(game->mino.color));
	for (i = 0; i != 4; ++i) {
		x = game->mino_pos.x + game->mino.block_pos[i].x;
		y = game->mino_pos.y + game->mino.block_pos[i].y;

		if (in_range(x, y) && y >= 0) {
			x = x * 2 + 1;
			++y;
			
			mvaddch(y, x, game->mino.block_left);
			mvaddch(y, x + 1, game->mino.block_right);
		}
	}
	attroff(COLOR_PAIR(game->mino.color));
}

void
update_timing(game_state *game)
{
	if ((double)(clock() - game->clock) / CLOCKS_PER_SEC > (game->fpc / 60.0)) {
		game->clock = clock();
		if (move_mino(game, 0, 1) == SUCCESS) {
			--game->drop_score;
		}
	}
}

int
in_range(int x, int y)
{
	return x >= 0 && x < BOARD_WIDTH && y < BOARD_HEIGHT;
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
	int i, j, lines;

	lines = 0;
	for (i = BOARD_HEIGHT - 1; i >= 0; --i) {
		for (j = 0; j != BOARD_WIDTH && game->board[i][j]; ++j)
			;

		if (j == BOARD_WIDTH) {
			for (j = i - 1; j >= 0; --j) {
				line_down(game, j);
			}

			++i;
			++lines;
			continue;
		}
	}

	if (lines) {
		game->score += (game->level + 1) * score_mult[lines - 1];
		game->lines += lines;
		game->level = game->lines / 10;
	}
}

void
spawn_mino(game_state *game)
{
	game->mino_pos.x = (BOARD_WIDTH - 1) / 2;
	game->mino_pos.y = 0;

	game->flags |= BIT(DRAW);

	memcpy(&game->mino, &minos[rand() % 7], sizeof(tetromino));
}

int
move_mino(game_state *game, int dx, int dy)
{
	int i, x, y;

	for (i = 0; i != 4; ++i) {
		x = game->mino_pos.x + game->mino.block_pos[i].x;
		y = game->mino_pos.y + game->mino.block_pos[i].y;
		
		/* Check if moving mino causes it to go out of bounds */
		if (!in_range(x + dx, y + dy) ||
		    (y >= 0 && game->board[y + dy][x + dx] && !game->board[y][x]) ||
		    dy == -1) {
			/* If collided with something while going downwards */
			if (dx == 0 && dy == 1) {
				for (i = 0; i != 4; ++i) {
					game->board[game->mino.block_pos[i].y + game->mino_pos.y]
						[game->mino.block_pos[i].x + game->mino_pos.x] = game->mino.color;
				}

				clear_lines(game);
				spawn_mino(game);

				game->score += game->drop_score;
				game->drop_score = 0;

				/* Update falling speed */
				if (game->level <= 8) {
					game->fpc = 48 - (game->level * 5);
				} else if (game->level == 9) {
					game->fpc = 6;
				} else if (game->level <= 12) {
					game->fpc = 5;
				} else if (game->level <= 15) {
					game->fpc = 4;
				} else if (game->level <= 18) {
					game->fpc = 3;
				} else if (game->level <= 28) {
					game->fpc = 2;
				} else {
					game->fpc = 1;
				}
			}

			return FAILURE;
		}
	}

	if (dy == 1) {
		++game->drop_score;
	}
		
	game->mino_pos.x += dx;
	game->mino_pos.y += dy;

	game->flags |= BIT(DRAW);

	return SUCCESS;
}

int
rotate_mino(game_state *game, int dir)
{
	tetromino tmp;
	point *p;
	int abs_x, abs_y, z;

	if (game->mino.flags & BIT(ROTATE_NONE)) {
		return FAILURE;
	}

	memcpy(&tmp, &game->mino, sizeof(tetromino));

	if (game->mino.flags & BIT(ROTATE_TWICE)) {
		dir = tmp.flags & BIT(ROTATION);
		tmp.flags ^= BIT(ROTATION);
	}

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
			return FAILURE;
		}
	}

	memcpy(&game->mino, &tmp, sizeof(tetromino));
	game->flags |= BIT(DRAW);

	return SUCCESS;
}


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
			       'I',
			       { { 0, 0 }, { 0, 1 },		/* I */
				 { 0, 2 }, { 0, 3 } },
			       { 0, 2 },
			       RED,
			       BIT(ROTATE_TWICE) },
			     
			     { '{', '}',
			       'L',
			       { { -1, 0 }, { 0, 0 },		/* L */
				 { 1, 0 }, { -1, 1 } },
			       { 0, 0 },
			       GREEN,
			       0 },
			     
			     { '(', ')',
			       'J',
			       { { -1, 0 }, { 0, 0 },		/* J */
				 { 1, 0 }, { 1, 1 } },
			       { 0, 0 },
			       YELLOW,
			       0 },
			     
			     { '[', ']',
			       'O',
			       { { 0, 0 }, { 1, 0 },		/* O */
				 { 0, 1 }, { 1, 1 } },
			       { 0, 0 },
			       BLUE,
			       BIT(ROTATE_NONE) },
			     
			     { '%', '%',
			       'S',
			       { { 0, 0 }, { 1, 0 },		/* S */
				 { 0, 1 }, { -1, 1 } },
			       { 0, 0 },
			       MAGENTA,
			       BIT(ROTATE_TWICE) },
			     
			     { '@', '@',
			       'Z',
			       { { 0, 0 }, { -1, 0 },		/* Z */
				 { 0, 1 }, { 1, 1 } },
			       { 0, 0 },
			       CYAN,
			       BIT(ROTATE_TWICE) },
			     
			     { '#', '#',
			       'T',
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

	if (game->scores = fopen(HI_SCORES, "rb")) {
		fread(&game->hi_score, sizeof(game->hi_score), 1, game->scores);
		fclose(game->scores);
	}

	spawn_mino(game);
}

void
game_over(game_state *game)
{
	if (game->score > game->hi_score) {
		game->hi_score = game->score;
	}

	if (game->scores = fopen(HI_SCORES, "wb")) {
		fwrite(&game->hi_score, sizeof(game->hi_score), 1, game->scores);
		fclose(game->scores);
	}

	game->flags |= BIT(QUIT);
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
	mvprintw(1, BOARD_WIDTH * 2 + 3, "score: %d\n", game->score);
	mvprintw(2, BOARD_WIDTH * 2 + 3, "hi-score: %d\n", game->hi_score);
	mvprintw(4, BOARD_WIDTH * 2 + 3, "lines: %d\n", game->lines);
	mvprintw(5, BOARD_WIDTH * 2 + 3, "level: %d\n", game->level);

	for (i = 0; i != 7; ++i) {
		attron(COLOR_PAIR(minos[i].color));
		mvprintw(7 + i, BOARD_WIDTH * 2 + 3, "%c%c%c:\t%d",
			 minos[i].block_left, minos[i].symbol , minos[i].block_right,
			 game->mino_count[i]);
		attroff(COLOR_PAIR(minos[i].color));
	}

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
	/* Loop though all lines in the board */
	for (i = BOARD_HEIGHT - 1; i >= 0; --i) {
		for (j = 0; j != BOARD_WIDTH && game->board[i][j]; ++j)
			;

		/* If a full line was found, clear it and move all lines above it down by 1 */
		if (j == BOARD_WIDTH) {
			for (j = i - 1; j >= 0; --j) {
				line_down(game, j);
			}

			++i;
			++lines;
			continue;
		}
	}

	/* If at least 1 line was cleared, update score */
	if (lines) {
		game->score += (game->level + 1) * score_mult[lines - 1];
		game->lines += lines;
		game->level = game->lines / 10;
	}
}

void
spawn_mino(game_state *game)
{
	int r;

	/* Initial tetromino position */
	game->mino_pos.x = (BOARD_WIDTH - 1) / 2;
	game->mino_pos.y = 0;

	/* Choose random tetromino */
	r = rand() % 7;
	memcpy(&game->mino, &minos[r], sizeof(tetromino));
	++game->mino_count[r];

	/* Quit game if spawn location is already occupied. (calling move_mino(game, 0, 0) might be better) */
	for (r = 0; r != 4; ++r) {
		if (game->board[game->mino.block_pos[r].y + game->mino_pos.y][game->mino.block_pos[r].x + game->mino_pos.x]) {
			game_over(game);
		}
	}

	game->flags |= BIT(DRAW);
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
				} else if (game->level <= 18) {
					game->fpc = 9 - (game->level / 3);
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
		dir = !(tmp.flags & BIT(ROTATION));
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


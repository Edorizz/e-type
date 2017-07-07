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
const mino minos[7] = { { '<', '>',
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
new_game(game_state *gs)
{
	memset(gs, 0, sizeof(game_state));
	gs->clock = clock();
	gs->flags = BIT(DRAW);
	gs->fpc = INITIAL_SPEED;

	bag_init(&gs->magic_bag);

	if ((gs->scores = fopen(HI_SCORES, "rb"))) {
		fread(&gs->hi_score, sizeof(gs->hi_score), 1, gs->scores);
		fclose(gs->scores);
	}

	spawn_mino(gs);
}

void
game_over(game_state *gs)
{
	if (gs->score > gs->hi_score) {
		gs->hi_score = gs->score;
	}

	if ((gs->scores = fopen(HI_SCORES, "wb"))) {
		fwrite(&gs->hi_score, sizeof(gs->hi_score), 1, gs->scores);
		fclose(gs->scores);
	}

	gs->flags |= BIT(QUIT);
}

void
pause(game_state *gs)
{
	gs->flags |= BIT(PAUSE);
}

void
draw_mino(const mino *m, int x, int y, uint8_t flags)
{
	int i, rx, ry;

	attron(COLOR_PAIR(m->color));

	for (i = 0; i != 4; ++i) {
		rx = x + m->block_pos[i].x * 2;
		ry = y + m->block_pos[i].y;

		mvprintw(ry, rx, "%c%c", m->block_left, m->block_right);
			 
	}

	attroff(COLOR_PAIR(m->color));
}

void
bdraw_mino(const mino *m, int x, int y, uint8_t flags)
{
	int i, rx, ry, attr;

	attr = flags & BIT(DRAW_GHOST) ? A_BOLD : COLOR_PAIR(m->color);
	attron(attr);

	for (i = 0; i != 4; ++i) {
		rx = x + m->block_pos[i].x;
		ry = y + m->block_pos[i].y;

		if (in_range(x, y) && y >= 0) {
			rx *= 2;

			mvprintw(ry + BOARD_SY, rx + BOARD_SX,
				 "%c%c", m->block_left, m->block_right);
		}
	}

	attroff(attr);
}

void
draw_board(game_state *gs)
{
	int i, j, c;
	const mino *next_mino;

	/* Draw board */
	mvprintw(0, 0, "*--------------------*\n");
	for (i = 0; i != BOARD_H; ++i) {
		addch('|');
		for (j = 0; j != BOARD_W; ++j) {
			if ((c = gs->board[i][j])) {
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
	mvprintw(1, BOARD_W * 2 + 3, "score: %d\n", gs->score);
	mvprintw(2, BOARD_W * 2 + 3, "hi-score: %d\n", gs->hi_score);
	mvprintw(4, BOARD_W * 2 + 3, "lines: %d\n", gs->lines);
	mvprintw(5, BOARD_W * 2 + 3, "level: %d\n", gs->level);

	for (i = 0; i != 7; ++i) {
		attron(COLOR_PAIR(minos[i].color));
		mvprintw(7 + i, BOARD_W * 2 + 3, "%c%c%c:\t%d",
			 minos[i].block_left, minos[i].symbol , minos[i].block_right,
			 gs->mino_count[i]);
		attroff(COLOR_PAIR(minos[i].color));
	}

	/* Draw ghost tetromino */
	bdraw_mino(&gs->curr_mino, gs->curr_mino_pos.x, gs->ghost_pos, BIT(DRAW_GHOST));

	/* Draw current tetromino */
	bdraw_mino(&gs->curr_mino, gs->curr_mino_pos.x, gs->curr_mino_pos.y, 0);

	/* Draw next tetromino */
	next_mino = &minos[bag_peek(&gs->magic_bag)];
	draw_mino(next_mino, (BOARD_W + BOARD_SX) * 2 + 4, 15, 0);
}

void
update_timing(game_state *gs)
{
	if ((double)(clock() - gs->clock) / CLOCKS_PER_SEC > (gs->fpc / 60.0)) {
		gs->clock = clock();
		if (move_mino(gs, 0, 1, AUTO_DROP) == SUCCESS) {
			--gs->drop_score;
		}
	}
}

void
update_ghost(game_state *gs)
{
	int i, j, x, y;

	i = 0;
	do {
		for (j = 0; j != 4; ++j) {
			x = gs->curr_mino_pos.x + gs->curr_mino.block_pos[j].x;
			y = gs->curr_mino_pos.y + gs->curr_mino.block_pos[j].y + i;

			if (y >= 0 && (!in_range(x, y) || gs->board[y][x])) {
				break;
			}
		}
	} while (j == 4 && ++i);

	gs->ghost_pos = gs->curr_mino_pos.y + i - 1;
}

int
in_range(int x, int y)
{
	return x >= 0 && x < BOARD_W && y < BOARD_H;
}

void
line_down(game_state *gs, int y)
{
	int i;

	for (i = 0; i != BOARD_W; ++i) {
		gs->board[y + 1][i] = gs->board[y][i];
		gs->board[y][i] = 0;
	}
}

void
clear_lines(game_state *gs)
{
	int i, j, lines;

	lines = 0;
	/* Loop though all lines in the board */
	for (i = BOARD_H - 1; i >= 0; --i) {
		for (j = 0; j != BOARD_W && gs->board[i][j]; ++j)
			;

		/* If a full line was found, clear it and move all lines above it down by 1 */
		if (j == BOARD_W) {
			for (j = i - 1; j >= 0; --j) {
				line_down(gs, j);
			}

			++i;
			++lines;
			continue;
		}
	}

	/* If at least 1 line was cleared, update score */
	if (lines) {
		gs->score += (gs->level + 1) * score_mult[lines - 1];
		gs->lines += lines;
		gs->level = gs->lines / 10;
	}
}

void
hard_drop(game_state *gs)
{
	while (move_mino(gs, 0, 1, HARD_DROP))
		;
}

void
spawn_mino(game_state *gs)
{
	int r;

	/* Initial tetromino position */
	gs->curr_mino_pos.x = (BOARD_W - 1) / 2;
	gs->curr_mino_pos.y = 0;

	/* Choose random tetromino */
	r = bag_next(&gs->magic_bag);
	memcpy(&gs->curr_mino, &minos[r], sizeof(mino));
	++gs->mino_count[r];

	/* Quit game if spawn location is already occupied. (calling move_mino(game, 0, 0) might be better) */
	for (r = 0; r != 4; ++r) {
		if (gs->board[gs->curr_mino.block_pos[r].y + gs->curr_mino_pos.y][gs->curr_mino.block_pos[r].x + gs->curr_mino_pos.x]) {
			game_over(gs);
		}
	}

	update_ghost(gs);

	gs->flags |= BIT(DRAW);
}

int
move_mino(game_state *gs, int dx, int dy, uint8_t flags)
{
	int i, x, y;

	for (i = 0; i != 4; ++i) {
		x = gs->curr_mino_pos.x + gs->curr_mino.block_pos[i].x;
		y = gs->curr_mino_pos.y + gs->curr_mino.block_pos[i].y;
		
		/* Check if moving mino causes it to go out of bounds */
		if (!in_range(x + dx, y + dy) ||
		    (y >= 0 && gs->board[y + dy][x + dx] && !gs->board[y][x]) ||
		    dy == -1) {
			/* If collided with something while going downwards */
			if (dx == 0 && dy == 1) {
				if (gs->immune) {
					if (((double)clock() - gs->immune) / CLOCKS_PER_SEC < IMMUNITY_TIMER) {
						return SUCCESS;
					}
				} else if (flags == SOFT_DROP) {
					gs->immune = clock();
					return SUCCESS;
				}

				gs->immune = 0;

				for (i = 0; i != 4; ++i) {
					gs->board[gs->curr_mino.block_pos[i].y + gs->curr_mino_pos.y]
						[gs->curr_mino.block_pos[i].x + gs->curr_mino_pos.x] = gs->curr_mino.color;
				}

				clear_lines(gs);
				spawn_mino(gs);

				gs->score += gs->drop_score;
				gs->drop_score = 0;

				/* Update falling speed */
				if (gs->level <= 8) {
					gs->fpc = 48 - (gs->level * 5);
				} else if (gs->level <= 18) {
					gs->fpc = 9 - (gs->level / 3);
				} else if (gs->level <= 28) {
					gs->fpc = 2;
				} else {
					gs->fpc = 1;
				}
			}

			return FAILURE;
		}
	}

	if (dy == 1) {
		++gs->drop_score;
	}

	gs->curr_mino_pos.x += dx;
	gs->curr_mino_pos.y += dy;

	update_ghost(gs);

	gs->flags |= BIT(DRAW);

	return SUCCESS;
}

int
rotate_mino(game_state *gs, int dir)
{
	mino tmp;
	point *p;
	int abs_x, abs_y, i, z;

	if (gs->curr_mino.flags & BIT(ROTATE_NONE)) {
		return FAILURE;
	}

	memcpy(&tmp, &gs->curr_mino, sizeof(mino));

	if (gs->curr_mino.flags & BIT(ROTATE_TWICE)) {
		dir = !(tmp.flags & BIT(ROTATION));
		tmp.flags ^= BIT(ROTATION);
	}

	for (i = 0; i != 4; ++i) {
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

		abs_x = p->x + gs->curr_mino_pos.x;
		abs_y = p->y + gs->curr_mino_pos.y;

		if (abs_y >= 0 && (!in_range(abs_x, abs_y) || gs->board[abs_y][abs_x])) {
			return FAILURE;
		}
	}

	memcpy(&gs->curr_mino, &tmp, sizeof(mino));
	update_ghost(gs);

	gs->flags |= BIT(DRAW);

	return SUCCESS;
}


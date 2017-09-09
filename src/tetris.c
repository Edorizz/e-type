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
/* e-type */
#include "log.h"

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
const struct mino minos[7] = { { '<', '>',
				 'I',
				 { { -1, 0 }, { 0, 0 },			/* I */
				   { 1, 0 }, { 2, 0 } },
				 { 0, 0 },
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
				 { { 0, 0 }, { 1, 0 },			/* O */
				   { 0, 1 }, { 1, 1 } },
				 { 0, 0 },
				 BLUE,
				 BIT(ROTATE_NONE) },
			       
			       { '%', '%',
				 'S',
				 { { 0, 0 }, { 1, 0 },			/* S */
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
new_game(struct game_state *gs)
{
	memset(gs, 0, sizeof(struct game_state));
	gs->clock = clock();
	gs->flags = BIT(DRAW);
	gs->fpc = INITIAL_SPEED;

	config_default(&gs->prof);
	config_read("e-type.conf", &gs->prof);

	printf("config done\n");
	gs->prof.rand_init(gs->prof.rng);

	if ((gs->scores = fopen(HI_SCORES, "rb"))) {
		fread(&gs->hi_score, sizeof(gs->hi_score), 1, gs->scores);
		fclose(gs->scores);
	}

	spawn_mino(gs);
}

void
game_over(struct game_state *gs)
{
	if (gs->score > gs->hi_score) {
		gs->hi_score = gs->score;
	}

	if ((gs->scores = fopen(HI_SCORES, "wb"))) {
		fwrite(&gs->hi_score, sizeof(gs->hi_score), 1, gs->scores);
		fclose(gs->scores);
	}

	config_free(&gs->prof);

	gs->flags |= BIT(QUIT);
}

void
draw_mino(WINDOW *win, const struct mino *m, int x, int y, uint8_t flags)
{
	int i, rx, ry;

	if (!(flags & BIT(DRAW_GHOST))) {
			wattron(win, COLOR_PAIR(m->color));
	}

	for (i = 0; i != 4; ++i) {
		rx = x + m->block_pos[i].x * 2;
		ry = y + m->block_pos[i].y;

		mvwprintw(win, ry, rx, "%c%c", m->block_left, m->block_right);
	}

	if (!(flags & BIT(DRAW_GHOST))) {
		wattroff(win, COLOR_PAIR(m->color));
	}
}

void
bdraw_mino(const struct mino *m, int x, int y, uint8_t flags)
{
	int i, rx, ry, attr;

	attr = flags & BIT(DRAW_GHOST) ? A_BOLD : COLOR_PAIR(m->color);
	attron(attr);

	for (i = 0; i != 4; ++i) {
		rx = x + m->block_pos[i].x;
		ry = y + m->block_pos[i].y;

		if (in_range(rx, ry) && ry >= 0) {
			rx *= 2;

			mvprintw(ry + BOARD_SY, rx + BOARD_SX,
				 "%c%c", m->block_left, m->block_right);
		}
	}

	attroff(attr);
}

void
draw_board(struct game_state *gs)
{
	int i, j, c;
	const struct mino *next_mino;

	wclear(gs->board_win);
	wclear(gs->stats_win);

	/* Draw board */
	for (i = 0; i != BOARD_H; ++i) {
		wmove(gs->board_win, i + 1, 1);
		for (j = 0; j != BOARD_W; ++j) {
			if ((c = gs->board[i][j])) {
				wattron(gs->board_win, COLOR_PAIR(c));
				wprintw(gs->board_win, "%c%c", minos[c - 1].block_left,
					minos[c - 1].block_right);
				wattroff(gs->board_win, COLOR_PAIR(c));
			} else {
				wprintw(gs->board_win, "%s", "  ");
			}
		}
	}

	/* Draw statistics */
	mvwprintw(gs->stats_win, 1, 1, "score: %d", gs->score);
	mvwprintw(gs->stats_win, 2, 1, "hi-score: %d", gs->hi_score);
	mvwprintw(gs->stats_win, 4, 1, "lines: %d", gs->lines);
	mvwprintw(gs->stats_win, 5, 1, "level: %d", gs->level);

	for (i = 0; i != 7; ++i) {
		wattron(gs->stats_win, COLOR_PAIR(minos[i].color));
		mvwprintw(gs->stats_win, 7 + i, 1, "%c%c%c:\t%d",
			  minos[i].block_left, minos[i].symbol , minos[i].block_right,
			  gs->mino_count[i]);
		wattroff(gs->stats_win, COLOR_PAIR(minos[i].color));
	}

	if (!(gs->flags & BIT(LBREAK))) {
		/* Draw ghost tetromino */
		if (gs->prof.flags & BIT(CONFIG_FGHOST)) {
			draw_mino(gs->board_win, &gs->curr_mino, gs->curr_mino_pos.x * 2 + 1, gs->ghost_pos + 1, BIT(DRAW_GHOST));
		}
		
		/* Draw current tetromino */
		draw_mino(gs->board_win, &gs->curr_mino, gs->curr_mino_pos.x * 2 + 1, gs->curr_mino_pos.y + 1, 0);
	}
			
	/* Draw next tetromino */
	next_mino = &minos[gs->prof.rand_peek(gs->prof.rng)];
	draw_mino(gs->stats_win, next_mino, 5, 15, 0);

	box(gs->board_win, 0, 0);
	box(gs->stats_win, 0, 0);

	wrefresh(gs->board_win);
	wrefresh(gs->stats_win);
}

void
update_timing(struct game_state *gs)
{
	if ((double)(clock() - gs->clock) / CLOCKS_PER_SEC > (gs->fpc / 60.0)) {
		gs->clock = clock();
		if (move_mino(gs, 0, 1, AUTO_DROP) == SUCCESS) {
			--gs->drop_score;
		}
	}
}

void
update_ghost(struct game_state *gs)
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

void
update_lbreak(struct game_state *gs)
{
	int i;
	
	if ((double)(clock() - gs->lbreak_timer) / CLOCKS_PER_SEC >= LINE_BREAK_BLOCK_TIMER) {
		if (gs->lbreak_block == BOARD_W / 2) {
			clear_lines(gs);
			spawn_mino(gs);
			gs->flags ^= BIT(LBREAK);
		} else {
			log_write("lines: %d\n", gs->lbreak_count);

			for (i = 0; i != gs->lbreak_count; ++i) {
				log_write("\tl%d: %d\n", i, gs->lbreak_lines[i]);
				gs->board[gs->lbreak_lines[i]][(BOARD_W - 1) / 2 - gs->lbreak_block] = 0;
				gs->board[gs->lbreak_lines[i]][(BOARD_W) / 2 + gs->lbreak_block] = 0;
			}

			++gs->lbreak_block;
			gs->flags |= BIT(DRAW);
			gs->lbreak_timer = clock();
		}
	}
}

int
in_range(int x, int y)
{
	return x >= 0 && x < BOARD_W && y < BOARD_H;
}

void
line_down(struct game_state *gs, int y)
{
	int i;

	log_write("d: %d\n", y);
	for (i = 0; i != BOARD_W; ++i) {
		gs->board[y + 1][i] = gs->board[y][i];
		gs->board[y][i] = 0;
	}
}

void
clear_lines(struct game_state *gs)
{
	int i, j;

	if (gs->lbreak_count) {
		/* Loop though all lines in the board */
		for (i = 0; i != gs->lbreak_count; ++i) {
			log_write("-------!\n");
			for (j = gs->lbreak_lines[i] - 1; j >= 0; --j) {
				line_down(gs, j);
			}
			log_write("-------!\n");
		}
		
		/* If at least 1 line was cleared, update score */
		gs->score += (gs->level + 1) * score_mult[gs->lbreak_count - 1];
		gs->lines += gs->lbreak_count;
		gs->level = gs->lines / 10;
	}
}

void
hard_drop(struct game_state *gs)
{
	while (move_mino(gs, 0, 1, HARD_DROP))
		;
}

void
spawn_mino(struct game_state *gs)
{
	int r;

	/* Initial tetromino position */
	gs->curr_mino_pos.x = (BOARD_W - 1) / 2;
	gs->curr_mino_pos.y = 0;

	/* Choose random tetromino */
	r = gs->prof.rand_next(gs->prof.rng);
	memcpy(&gs->curr_mino, &minos[r], sizeof(struct mino));
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
move_mino(struct game_state *gs, int dx, int dy, uint8_t flags)
{
	int i, j, k, x, y;

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

				/* clear_lines(gs); */
				gs->lbreak_count = 0;
				for (j = 0; j != BOARD_H; ++j) {
					for (k = 0; k != BOARD_W && gs->board[j][k]; ++k)
						;

					if (k == BOARD_W) {
						gs->lbreak_lines[gs->lbreak_count++] = j;
					}
				}

				if (gs->lbreak_count > 0) {
					gs->lbreak_timer = clock();
					gs->lbreak_block = 0;
					gs->flags |= BIT(LBREAK);
				} else {
					spawn_mino(gs);
				}

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
rotate_mino(struct game_state *gs, int dir)
{
	struct mino tmp;
	struct point *p;
	int abs_x, abs_y, i, z;

	if (gs->curr_mino.flags & BIT(ROTATE_NONE)) {
		return FAILURE;
	}

	memcpy(&tmp, &gs->curr_mino, sizeof(struct mino));

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

	memcpy(&gs->curr_mino, &tmp, sizeof(struct mino));
	update_ghost(gs);

	gs->flags |= BIT(DRAW);

	return SUCCESS;
}


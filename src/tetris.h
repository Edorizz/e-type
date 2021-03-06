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

#ifndef TETRIS_H
#define TETRIS_H

/* Standard Tetris */
#define BOARD_H			20
#define BOARD_W			10
#define BOARD_SX		1
#define BOARD_SY		1
#define INITIAL_SPEED		48
#define IMMUNITY_TIMER		0.2
#define LINE_BREAK_BLOCK_TIMER	0.1

/* Rotation */
#define CLOCKWISE		0
#define COUNTER_CLOCKWISE	1

/* Tetromino movement/rotation status */
#define SUCCESS			1
#define FAILURE			0

/* Special directories */
#define HI_SCORES		"e-type.dat"


/* C library */
#include <stdio.h>
#include <stdint.h>
#include <time.h>

/* Ncurses */
#include <ncurses.h>

/* e-type */
#include "config.h"
#include "utils.h"


/* Choose one of this colors for each tetromino */
typedef enum { RESET, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE } char_colors;

/* Drop types, used when calling move_mino() */
typedef enum { HARD_DROP, SOFT_DROP, AUTO_DROP } drop_type;

/* Special tetromino attributes */
typedef enum { ROTATION, ROTATE_NONE, ROTATE_TWICE } mino_flags;

/* Drawing flags */
typedef enum { DRAW_GHOST } draw_flags;

/* Flags used to tell the status of the game */
typedef enum { QUIT, PAUSE, DRAW_BOARD, DRAW_STATS, DRAW_HOLD, LBREAK, BLOCK_HOLD } status;


/*
 * -==+ 2D Point +==- 
 * Basic coordinate, reduces ammount of loose variables
 */
struct point {
	int8_t x, y;
};

/*
 * -==+ Tetromino +==-
 * Servers as a blueprint for tetromino creation.
 */
struct mino {
	/* [Printing] */
	char block_left, block_right;
	char symbol;
	/* [Position data] */
	struct point block_pos[4];
	struct point pivot;
	/* [Attributes] */
	uint8_t color;
	uint8_t	flags;
	uint8_t id;
};

/*
 * -==+ Current game state +==-
 * Contain all necessary information of the current game state,
 * it basically packs everything together to avoid having alot
 * of different variables.
 */
struct game_state {
	/* [Board state] */
	uint8_t board[BOARD_H][BOARD_W];
	uint8_t flags;
	uint8_t ghost_pos;
	struct mino curr_mino;
	const struct mino *hold_mino;
	struct point curr_mino_pos;
	/* [Line break animation] */
	clock_t lbreak_timer;
	int lbreak_block;
	int lbreak_lines[4];
	int lbreak_count;
	/* [Statistics] */
	uint8_t	level;
	uint32_t mino_count[7];
	uint32_t lines;
	uint32_t hi_score;
	uint32_t score;
	uint32_t drop_score;
	FILE *scores;
	/* [Timing] */
	clock_t pause_clock;
	clock_t	clock;
	clock_t	immune;
	double fpc;
	/* [Config] */
	struct config_prof prof;
	/* [Drawing] */
	WINDOW *board_win, *stats_win, *hold_win;
};


/* -==+ Start/End +==- */
void new_game(struct game_state *gs);
void game_over(struct game_state *gs);

/* -==+ Drawing +==- */
void draw_mino(WINDOW *win, const struct mino *m, int x, int y, uint8_t flags);
void draw_game(struct game_state *gs);
void draw_stats(struct game_state *gs);
void draw_board(struct game_state *gs);

/* -==+ Timing +==- */
void pause_game(struct game_state *gs);
void resume_game(struct game_state *gs);
void update_timing(struct game_state *gs);
void update_lbreak(struct game_state *gs);

/* -==+ Check/Update Board state +==- */
int  in_range(int x, int y);
void line_down(struct game_state *gs, int y);
void clear_lines(struct game_state *gs);
void hard_drop(struct game_state *gs);

/* -==+ Manipulate Tetromino +==- */
void update_ghost(struct game_state *gs);
void spawn_mino(struct game_state *gs);
void hold_mino(struct game_state *gs);
int  move_mino(struct game_state *gs, int dx, int dy, uint8_t flags);
int  rotate_mino(struct game_state *gs, int dir);

#endif /* TETRIS_H */


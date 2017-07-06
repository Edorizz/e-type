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
#define BOARD_HEIGHT		20
#define BOARD_WIDTH		10
#define INITIAL_SPEED		48
#define IMMUNITY_TIMER		0.2

/* Rotation */
#define CLOCKWISE		0
#define COUNTER_CLOCKWISE	1

/* Tetromino movement/rotation status */
#define SUCCESS			1
#define FAILURE			0

/* Bit manipulation */
#define BIT(n)			(1 << n)

/* Special directories */
#define HI_SCORES		"e-type.dat"

/* C library */
#include <stdio.h>
#include <stdint.h>
#include <time.h>
/* Ncurses */
#include <ncurses.h>

/* Choose one of this colors for each tetromino */
typedef enum { RESET, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE } char_colors;
/* Drop types, used when calling move_mino() */
typedef enum { HARD_DROP, SOFT_DROP, AUTO_DROP } drop_type;
/* Special tetromino attributes */
typedef enum { ROTATION, ROTATE_NONE, ROTATE_TWICE } mino_flags;
/* Game signals used to control the game */
typedef enum { QUIT, DRAW, PAUSE } signals;

typedef struct _point {
	int x, y;
} point;

typedef struct _mino {
	char		block_left, block_right;
	char		symbol;
	point		block_pos[4];
	point		pivot;
	int		color;
	uint8_t		flags;
} mino;

typedef struct _game_state {
	/* [Board state] */
	uint8_t		board[BOARD_HEIGHT][BOARD_WIDTH];
	uint8_t 	flags;
	mino 		curr_mino;
	point 		curr_mino_pos;
	uint8_t 	ghost_pos;
	/* [Stats] */
	uint8_t		level;
	uint32_t	mino_count[7];
	uint32_t	lines;
	uint32_t	hi_score;
	uint32_t	score;
	uint32_t	drop_score;
	/* [Timing] */
	clock_t		clock;
	clock_t		immune;
	double		fpc;
	/* [Extra] */
	FILE		*scores;
} game_state;

void new_game(game_state *gs);
void game_over(game_state *gs);
void pause(game_state *gs);
void draw_board(game_state *gs);
void update_timing(game_state *gs);
void update_ghost(game_state *gs);

int  in_range(int x, int y);
void line_down(game_state *gs, int y);
void clear_lines(game_state *gs);
void hard_drop(game_state *gs);

void spawn_mino(game_state *gs);
int  move_mino(game_state *gs, int dx, int dy, uint8_t flags);
int  rotate_mino(game_state *gs, int dir);

#endif /* TETRIS_H */


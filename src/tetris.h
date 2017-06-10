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

/* Ncurses colors */
#define RESET			0
#define RED 			1
#define GREEN 			2
#define YELLOW 			3
#define BLUE 			4
#define MAGENTA			5
#define CYAN 			6
#define WHITE 			7

/* Standard Tetris */
#define BOARD_HEIGHT		20
#define BOARD_WIDTH		10
#define INITIAL_SPEED		48

/* Rotation */
#define CLOCKWISE		0
#define COUNTER_CLOCKWISE	1

/* Game state flags */
#define QUIT			0
#define DRAW			1

/* Tetromino flags */
#define ROTATION		0
#define ROTATE_NONE		1
#define ROTATE_TWICE		2

/* Tetromino movement/rotation status */
#define SUCCESS			1
#define FAILURE			0

/* Bit manipulation */
#define BIT(n)			(1 << n)

#include <stdint.h>
#include <time.h>
#include <ncurses.h>

typedef struct {
	int x, y;
} point;

typedef struct {
	char block_left, block_right;
	point block_pos[4];
	point pivot;
	int color;
	uint8_t flags;
} tetromino;

typedef struct {
	/* Game state */
	uint8_t board[BOARD_HEIGHT][BOARD_WIDTH];
	uint8_t flags;
	tetromino mino;
	point mino_pos;
	/* Score */
	uint8_t level;
	uint32_t lines;
	uint32_t score;
	uint32_t drop_score;
	/* Timing */
	clock_t clock;
	double fpc;
} game_state;

void new_game(game_state *game);
void draw_board(game_state *game);
void update_timing(game_state *game);

int  in_range(int x, int y);
void line_down(game_state *game, int y);
void clear_lines(game_state *game);

void spawn_mino(game_state *game);
int  move_mino(game_state *game, int dx, int dy);
int  rotate_mino(game_state *game, int dir);

#endif /* TETRIS_H */


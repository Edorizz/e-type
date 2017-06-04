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

/* Standard Tetris board dimensions */
#define BOARD_HEIGHT	20
#define BOARD_WIDTH	10

/* Bit flags */
#define QUIT		0
#define DRAW		1

/* Bit manipulation */
#define BIT(n)		(1 << n)

#include <stdint.h>
#include <ncurses.h>

typedef struct {
	uint8_t x, y;
} point;

typedef struct {
	uint8_t board[BOARD_HEIGHT][BOARD_WIDTH];
	uint8_t flags;
	point current_mino;
} game_state;

void new_game(game_state *game);
void draw_board(game_state *game);
void move_mino(game_state *game, int dx, int dy);

#endif /* TETRIS_H */

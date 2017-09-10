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

/* C library */
#include <stdlib.h>
#include <string.h>

/* e-type */
#include "tetris.h"
#include "log.h"


int  init_ncurses(struct game_state *gs);
void handle_input(struct game_state *gs);


int
main(int argc, char **argv)
{
	/*
	 * I like using a big struct to hold everything because it makes agrument
	 * passing easier to handle since I can just pass the whole thing.
	 */
	struct game_state gs;
	char *opts[3] = { "Single Player", "Multi Player", "Quit" };
	int opt_i, i, quit, draw;
	WINDOW *board, *stats, *hold;

	/* Initialize everything */
	log_init("e-type.log");
	srand(time(NULL));
	init_ncurses(&gs);

	/* Create windows */
	hold = newwin(8, 14, (LINES - BOARD_H - 2) / 2, COLS / 2 - 28);
	board = newwin(BOARD_H + 2, BOARD_W * 2 + 2, (LINES - BOARD_H - 2) / 2, COLS / 2 - 14);
	stats = newwin(BOARD_H + 2, BOARD_W * 2 + 2, (LINES - BOARD_H - 2) / 2, COLS / 2 + BOARD_W * 2 - 12);

	draw = 1;
	opt_i = quit = 0;
	while (!quit) {
		if (draw) {
			for (i = 0; i != 3; ++i) {
				if (opt_i == i) {
					attron(COLOR_PAIR(GREEN));
					mvprintw(LINES / 2 - 1 + i, (COLS - strlen(opts[0])) / 2, "[ %s ]", opts[i]);
					attroff(COLOR_PAIR(GREEN));
					
				} else {
					mvprintw(LINES / 2 - 1 + i, (COLS - strlen(opts[0])) / 2 + 2, opts[i]);
				}
			}

			draw = 0;
		}

		switch (getch()) {
		case 'w': case 'W':
		case 'k': case 'K':
		case KEY_UP:
			move(LINES / 2 - 1 + opt_i, 0);
			clrtoeol();
			draw = 1;

			if (opt_i == 0) {
				opt_i = 2;

			} else {
				--opt_i;
			}
			
			break;

		case 's': case 'S':
		case 'j': case 'J':
		case KEY_DOWN:
			move(LINES / 2 - 1 + opt_i, 0);
			clrtoeol();
			draw = 1;

			if (opt_i == 2) {
				opt_i = 0;

			} else {
				++opt_i;
			}

			break;

		case 'q': case 'Q':
			quit = 1;
			break;

		case '\n':
			switch (opt_i) {
			case 0:
				new_game(&gs, board, stats, hold);
				
				while (!(gs.flags & BIT(QUIT))) {
					draw_game(&gs);
					handle_input(&gs);
					
					if (!(gs.flags & BIT(PAUSE))) {
						if (gs.flags & BIT(LBREAK)) {
							update_lbreak(&gs);
						} else {
							update_timing(&gs);
						}
					}
				}

				clear();
				refresh();
				draw = 1;

				break;

			case 1:
				break;

			case 2:
				quit = 1;
				break;
			}
			
			break;
		}
	}
	
	endwin();
	return 0;
}

int
init_ncurses(struct game_state *gs)
{
	/* Initialize Ncurses */
	initscr();
	cbreak();
	noecho();
	curs_set(0);
	timeout(0);

	/* Initialize color */
	use_default_colors();
	start_color();
	init_pair(RED, COLOR_RED, -1);
	init_pair(GREEN, COLOR_GREEN, -1);
	init_pair(YELLOW, COLOR_YELLOW, -1);
	init_pair(BLUE, COLOR_BLUE, -1);
	init_pair(MAGENTA, COLOR_MAGENTA, -1);
	init_pair(CYAN, COLOR_CYAN, -1);
	init_pair(WHITE, COLOR_WHITE, -1);

	return 0;
}

/* TODO: Allow for customizable keys */
void
handle_input(struct game_state *gs)
{
	int c;

	c = getch();

	if (gs->flags & BIT(PAUSE)) {
		if (c == 'P' || c == 'p') {
			resume(gs);
		}

	} else {
		switch (c) {
		case 'W': case 'w':
			move_mino(gs, 0, -1, SOFT_DROP);
			break;

		case 'S': case 's':
			move_mino(gs, 0, 1, SOFT_DROP);
			break;

		case 'A': case 'a':
			move_mino(gs, -1, 0, SOFT_DROP);
			break;

		case 'D': case 'd':
			move_mino(gs, 1, 0, SOFT_DROP);
			break;

		case 'J': case 'j':
			rotate_mino(gs, CLOCKWISE);
			break;

		case 'K': case 'k':
			rotate_mino(gs, COUNTER_CLOCKWISE);
			break;

		case 'R': case 'r':
			spawn_mino(gs);
			break;

		case 'L': case 'l':
			hold_mino(gs);
			break;

		case ' ':
			hard_drop(gs);
			break;

		case 'P': case 'p':
			pause(gs);
			break;

		case 'Q': case 'q':
			game_over(gs);
			break;
		}
	}
}


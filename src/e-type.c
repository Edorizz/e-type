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

/* POSIX */
#include <unistd.h>

/* Sockets */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/* e-type */
#include "tetris.h"
#include "log.h"


#define MENU_ROOT	0
#define MENU_DRAW	1
#define MENU_QUIT	2


struct selection {
	char *title;
	struct selection *dropdown;
	struct selection *parent;
	int cnt, opt_i, select, drop_color;
	void (*func) (struct game_state*);
};


int  init_ncurses(struct game_state *gs);
void handle_input(struct game_state *gs);

void print_logo(void);
int  print_menu(struct selection *menu, int y, int x);
void input_menu(struct selection *menu, struct game_state *gs, uint8_t *flags);

/* Menu selection functions */
void single_player(struct game_state *gs);
void join_game(struct game_state *gs);
void host_game(struct game_state *gs);
void quit(struct game_state *gs);


int
main(int argc, char **argv)
{
	/*
	 * I like using a big struct to hold everything since it makes agrument
	 * passing easier to handle.
	 */
	struct game_state gs;
	struct selection menu, sub_menu[3], sub_mp[2];
	uint8_t flags;

	/* Initialize everything */
	log_init("e-type.log");
	srand(time(NULL));
	init_ncurses(&gs);

	/* Create sub-menu for the 'Multiplayer' option */
	sub_mp[0].title = "Join";
	sub_mp[0].dropdown = NULL;
	sub_mp[0].parent = NULL;
	sub_mp[0].cnt = 0;
	sub_mp[0].opt_i = 0;
	sub_mp[0].select = 0;
	sub_mp[0].func = join_game;

	sub_mp[1].title = "Host";
	sub_mp[1].dropdown = NULL;
	sub_mp[1].parent = NULL;
	sub_mp[1].cnt = 0;
	sub_mp[1].opt_i = 0;
	sub_mp[1].select = 0;
	sub_mp[1].func = host_game;

	/* Create main menu */
	sub_menu[0].title = "Singleplayer";
	sub_menu[0].dropdown = NULL;
	sub_menu[0].parent = &menu;
	sub_menu[0].cnt = 0;
	sub_menu[0].opt_i = 0;
	sub_menu[0].select = 0;
	sub_menu[0].func = single_player;

	sub_menu[1].title = "Multiplayer";
	sub_menu[1].dropdown = sub_mp;
	sub_menu[1].parent = &menu;
	sub_menu[1].cnt = 2;
	sub_menu[1].opt_i = 0;
	sub_menu[1].select = 0;
	sub_menu[1].drop_color = BLUE;
	sub_menu[1].func = NULL;

	sub_menu[2].title = "Quit";
	sub_menu[2].dropdown = NULL;
	sub_menu[2].parent = &menu;
	sub_menu[2].cnt = 0;
	sub_menu[2].opt_i = 0;
	sub_menu[2].select = 0;
	sub_menu[2].func = quit;

	menu.title = "Main menu";
	menu.dropdown = sub_menu;
	menu.parent = NULL;
	menu.cnt = 3;
	menu.opt_i = 1;
	menu.select = 0;
	menu.drop_color = GREEN;
	menu.func = NULL;

	/* Create GUI windows */
	gs.hold_win = newwin(8, 14, (LINES - BOARD_H - 2) / 2, COLS / 2 - 28);
	gs.board_win = newwin(BOARD_H + 2, BOARD_W * 2 + 2, (LINES - BOARD_H - 2) / 2, COLS / 2 - 14);
	gs.stats_win = newwin(BOARD_H + 2, BOARD_W * 2 + 2, (LINES - BOARD_H - 2) / 2, COLS / 2 + BOARD_W * 2 - 12);

	flags |= BIT(MENU_DRAW);

	/* Main loop */
	while (!(flags & BIT(MENU_QUIT))) {
		flags |= BIT(MENU_ROOT);
		input_menu(&menu, &gs, &flags);

		if (flags & BIT(MENU_DRAW)) {
			clear();
			print_logo();
			print_menu(&menu, LINES / 2 - 1, (COLS - 15) / 2);
			refresh();
			flags ^= BIT(MENU_DRAW);
		}
	}
	
	quit(&gs);
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
			resume_game(gs);
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
			pause_game(gs);
			break;

		case 'Q': case 'q':
			game_over(gs);
			break;
		}
	}
}

void
print_logo(void)
{
	move(LINES / 2 - 3, (COLS - 6) / 2);

	attron(COLOR_PAIR(RED));
	addch('T');

	attron(COLOR_PAIR(BLUE));
	addch('E');

	attron(COLOR_PAIR(YELLOW));
	addch('T');

	attron(COLOR_PAIR(GREEN));
	addch('R');

	attron(COLOR_PAIR(CYAN));
	addch('I');

	attron(COLOR_PAIR(MAGENTA));
	addch('S');
	attroff(COLOR_PAIR(MAGENTA));
}

int
print_menu(struct selection *menu, int y, int x)
{
	int i, line;

	for (i = line = 0; i != menu->cnt; ++i) {
		if (menu->opt_i == i) {
			attron(COLOR_PAIR(menu->drop_color));
			mvprintw(y + line++, x, "[ %s ]", menu->dropdown[i].title);
			attroff(COLOR_PAIR(menu->drop_color));

			if (menu->select) {
				line += print_menu(&menu->dropdown[i], y + line, x + 2);
			}

		} else {
			mvprintw(y + line++, x, "  %s", menu->dropdown[i].title);
		}
	}

	return line;
}

void
input_menu(struct selection *menu, struct game_state *gs, uint8_t *flags)
{
	if (menu->select) {
		*flags &= ~BIT(MENU_ROOT);
		input_menu(&menu->dropdown[menu->opt_i], gs, flags);

	} else {
		switch (getch()) {
		case 'w': case 'W':
		case 'k': case 'K':
		case KEY_UP:
			*flags |= BIT(MENU_DRAW);

			if (menu->opt_i == 0) {
				menu->opt_i = menu->cnt - 1;

			} else {
				--menu->opt_i;
			}
			
			break;

		case 's': case 'S':
		case 'j': case 'J':
		case KEY_DOWN:
			*flags |= BIT(MENU_DRAW);

			if (menu->opt_i == menu->cnt - 1) {
				menu->opt_i = 0;

			} else {
				++menu->opt_i;
			}

			break;

		case 'q': case 'Q':
			*flags |= BIT(MENU_DRAW);

			if (menu->parent) {
				menu->parent->select = 0;

			} else {
				*flags |= BIT(MENU_QUIT);
			}

			break;

		case '\n':
			*flags |= BIT(MENU_DRAW);

			if (menu->dropdown[menu->opt_i].dropdown) {
				menu->select = 1;

			} else if (menu->dropdown[menu->opt_i].func) {
				menu->dropdown[menu->opt_i].func(gs);
			}

			break;
		}
	}
}

void
single_player(struct game_state *gs)
{
	new_game(gs);

	while (!(gs->flags & BIT(QUIT))) {
		draw_game(gs);
		handle_input(gs);
		
		if (!(gs->flags & BIT(PAUSE))) {
			if (gs->flags & BIT(LBREAK)) {
				update_lbreak(gs);
			} else {
				update_timing(gs);
			}
		}
	}
}

void
join_game(struct game_state *gs)
{
	struct addrinfo *res, hints;
	int host_fd, err;
	char ch, host_in, host_ip[] = "localhost";

	clear();
	refresh();

	memset(&hints, 0, sizeof (struct addrinfo));
	hints.ai_flags = AF_UNSPEC;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((err = getaddrinfo(host_ip, "1234", &hints, &res))) {
		fprintf(stderr, "%s", gai_strerror(err));
		return;
	}

	if ((host_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return;
	}

	if ((connect(host_fd, res->ai_addr, res->ai_addrlen)) == -1) {
		perror("connect");
		return;
	}

	while ((ch = getch())) {
		if (ch != ERR && send(host_fd, &ch, 1, 0) == -1) {
			perror("send");
			break;
		}
	}

	close(host_fd);
}

void
host_game(struct game_state *gs)
{
	struct addrinfo *res, hints;
	int sock_fd, client_fd, err;
	char client_in;

	clear();
	refresh();

	memset(&hints, 0, sizeof (struct addrinfo));
	hints.ai_flags = AF_UNSPEC;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((err = getaddrinfo(NULL, "1234", &hints, &res))) {
		fprintf(stderr, "%s\n", gai_strerror(err));
		return;
	}

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return;
	}

	err = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &err, sizeof err) == -1) {
		perror("setsockopt");
		return;
	}

	if (bind(sock_fd, res->ai_addr, res->ai_addrlen) == -1) {
		perror("bind");
		return;
	}

	if (listen(sock_fd, 5) == -1) {
		perror("listen");
		return;
	}

	if ((client_fd = accept(sock_fd, NULL, NULL)) == -1) {
		perror("accept");
		return;
	}

	while (recv(client_fd, &client_in, 1, 0) > 0) {
		mvprintw(LINES / 2, COLS / 2, "%c", client_in);
		refresh();
	}

	close(sock_fd);
}

void
quit(struct game_state *gs)
{
	endwin();
	exit(0);
}


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

#ifndef CONFIG_H
#define CONFIG_H

#define RAND_COUNT	2

/* Config flags */
#define CONFIG_FGHOST	0

/* C library */
#include <stdint.h>
#include <stddef.h>

struct rand_prof {
	const char *name;
	void (*init)(void*);
	int (*next)(void*);
	int (*peek)(void*);
	size_t mem_size;
};

struct config_prof {
	/* [Random Number Generator] */
	void *rng;
	void (*rand_init)(void*);
	int (*rand_next)(void*);
	int (*rand_peek)(void*);
	/* [Flags] */
	uint8_t flags;
};

void config_default(struct config_prof *prof);
int  config_read(const char *path, struct config_prof *prof);
void config_free(struct config_prof *prof);
int  parse_line(const char *line, struct config_prof *prof);

#endif /* CONFIG_H */

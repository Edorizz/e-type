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
#include "rng_simple.h"
/* C library */
#include <stdlib.h>

void
simple_init(void *rng)
{
	simple_next(rng);
}

int
simple_next(void *rng)
{
	struct rng_simple *simple;
	int tmp;

	simple = rng;
	tmp = simple->next;
	simple->next = rand() % 7;

	return tmp;
}

int
simple_peek(void *rng)
{
	struct rng_simple *simple;

	simple = rng;

	return simple->next;
}


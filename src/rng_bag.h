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

#ifndef RNG_BAG_H
#define RNG_BAG_H

/* C library */
#include <stdint.h>

struct rng_bag {
	uint8_t	bag[7 + 1];
	int bag_ind;
};

void bag_init(void *arg);
void bag_refill(void *arg);
int  bag_next(void *arg);
int  bag_peek(void *arg);

#endif /* RNG_BAG_H */


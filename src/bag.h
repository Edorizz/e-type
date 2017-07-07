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

#ifndef RAND_BAG_H
#define RAND_BAG_H

#define BAG_SIZE	7

/* C library */
#include <stdint.h>

typedef struct _rand_bag {
	uint8_t	bag[BAG_SIZE + 1];
	int	bag_ind;
} rand_bag;

/* Ramdomize the bag */
void bag_init(rand_bag *bag);

/* Put all items back in the bag */
void bag_refill(rand_bag *bag);

/* Takes the next item out of the bag and return it */
int  bag_next(rand_bag *bag);

/* Shows the next item in the bag without taking the item out. */
int  bag_peek(rand_bag *bag);

#endif /* RAND_BAG_H */


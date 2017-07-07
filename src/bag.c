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
#include "bag.h"
/* C library */
#include <stdlib.h>

void
bag_init(rand_bag *bag)
{
	int i;

	bag->bag_ind = 0;
	for (i = 0; i != BAG_SIZE; ++i) {
		bag->bag[i] = i;
	}
	bag->bag[BAG_SIZE] = rand() % BAG_SIZE;
	bag_refill(bag);
}

void
swap(uint8_t *a, uint8_t *b)
{
	uint8_t c;

	c = *a;
	*a = *b;
	*b = c;
}

void
scramble(uint8_t *arr, int size)
{
	int i;

	for (i = 0; i != size; ++i) {
		swap(&arr[i], &arr[rand() % size]);
	}
}

void
bag_refill(rand_bag *bag)
{
	int i;
	
	for (i = 0; bag->bag[i] != bag->bag[BAG_SIZE]; ++i)
		;

	swap(&bag->bag[0], &bag->bag[i]);
	scramble(bag->bag + 1, BAG_SIZE - 1);

	bag->bag[BAG_SIZE] = rand() % BAG_SIZE;
	bag->bag_ind = 0;
}

int
bag_next(rand_bag *bag)
{
	if (bag->bag_ind == BAG_SIZE) {
		bag_refill(bag);
	}

	return bag->bag[bag->bag_ind++];
}

int
bag_peek(rand_bag *bag)
{
	if (bag->bag_ind == BAG_SIZE) {
		bag_refill(bag);
	}
	
	return bag->bag[bag->bag_ind];
}


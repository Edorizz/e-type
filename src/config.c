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
#include "config.h"
/* C library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
/* e-type */
#include "rng_bag.h"
#include "rng_simple.h"
#include "utils.h"
#include "log.h"

#define LINE_SIZE	64

const struct rand_prof rand_profiles[2] = { { "simple",
					      simple_init, simple_next, simple_peek,
					      sizeof(struct rng_simple) },
					    
					    { "bag",
					      bag_init, bag_next, bag_peek,
					      sizeof(struct rng_bag) } };
void
load_rng(struct config_prof *prof, int rng_ind)
{
	prof->rand_init = rand_profiles[rng_ind].init;
	prof->rand_next = rand_profiles[rng_ind].next;
	prof->rand_peek = rand_profiles[rng_ind].peek;
	
	if (prof->rng) {
		free(prof->rng);
	}
	prof->rng = malloc(rand_profiles[rng_ind].mem_size);
}

int
line_empty(const char *str)
{
	while (isblank(*str) || *str == '\n')
		++str;

	return *str == '\0';
}

int
grab_word(const char *str, const char **word)
{
	while (*str == ' ')
		++str;

	*word = str;
	while (isalpha(*str) || *str == '_')
		++str;

	return str - *word;
}

void
config_default(struct config_prof *prof)
{
	load_rng(prof, 0);
	prof->flags |= BIT(CONFIG_FGHOST);
}


int
config_read(const char *path, struct config_prof *prof)
{
	FILE *fp;
	char buf[LINE_SIZE];

	if ((fp = fopen(path, "r")) == NULL) {
		perror("fopen");
		return -1;
	}

	while (fgets(buf, LINE_SIZE, fp)) {
		if (!line_empty(buf) && parse_line(buf, prof) == -1) {
			return -1;
		}
	}

	return 0;
}

void
config_free(struct config_prof *prof)
{
	prof->rand_init = NULL;
	prof->rand_next = NULL;
	prof->rand_peek = NULL;

	if (prof->rng) {
		free(prof->rng);
	}
}

int
parse_line(const char *line, struct config_prof *prof)
{
	const char *split, *var, *value;
	int var_size, value_size, i;
	
	if ((split = strchr(line, ':'))) {
		var = value = NULL;

		if ((var_size = grab_word(line, &var)) &&
		    (value_size = grab_word(split + 1, &value))) {
			if (strncmp(var, "rand_engine", var_size) == 0) {
				for (i = 0; i != RAND_COUNT; ++i) {
					if (strncmp(value, rand_profiles[i].name, value_size) == 0) {
						load_rng(prof, i);
					}
				}

			} else if (strncmp(var, "ghost_piece", var_size) == 0) {
				if (strncmp(value, "on", value_size) == 0) {
					prof->flags |= BIT(CONFIG_FGHOST);
				} else if (strncmp(value, "off", value_size) == 0) {
					prof->flags &= ~BIT(CONFIG_FGHOST);
				} else {
					log_write("Invalid value %s in ghost_piece\n", value);
					return -1;
				}
			}
		}

		return 0;
	} else {
		log_write("Wrong format; expected ':'\n");
		return -1;
	}
}


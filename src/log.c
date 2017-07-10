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
#include "log.h"
/* C library */
#include <stdio.h>
#include <stdarg.h>

FILE *fp_log = NULL;

int
log_init(const char *log_path)
{
	if (fp_log == NULL) {
		fp_log = fopen(log_path, "w");
		if (fp_log == NULL) {
			perror("fopen");
			return -1;
		}
		return 1;
	}

	return 0;
}

void
log_write(const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vfprintf(fp_log, format, ap);
	va_end(ap);
}


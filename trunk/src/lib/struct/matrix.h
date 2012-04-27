/*
 *  Libstruct
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MATRIX_H
#define MATRIX_H


struct matrix_t;

struct matrix_t *matrix_create(int x_size, int y_size);
void matrix_free(struct matrix_t *matrix);

void matrix_set(struct matrix_t *matrix, int x, int y, void *value);
void *matrix_get(struct matrix_t *matrix, int x, int y);

void matrix_clear(struct matrix_t *matrix);

#define MATRIX_FOR_EACH(matrix, x, y) \
	for ((x) = 0; (x) < matrix_get_x_size((matrix)); (x)++) \
	for ((y) = 0; (y) < matrix_get_y_size((matrix)); (y)++)

#endif

/*
 *  Libstruct
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#ifndef LIB_UTIL_MATRIX_H
#define LIB_UTIL_MATRIX_H


struct matrix_t;


#define MATRIX_FOR_EACH(matrix, row, col) \
	for ((row) = 0; (row) < matrix_get_num_rows((matrix)); (row)++) \
	for ((col) = 0; (col) < matrix_get_num_cols((matrix)); (col)++)

#define MATRIX_FOR_EACH_ROW(matrix, row) \
	for ((row) = 0; (row) < matrix_get_num_rows((matrix)); (row)++)

#define MATRIX_FOR_EACH_COLUMN(matrix, col) \
	for ((col) = 0; (col) < matrix_get_num_cols((matrix)); (col)++)


struct matrix_t *matrix_create(int num_rows, int num_cols);
void matrix_free(struct matrix_t *matrix);

void matrix_set(struct matrix_t *matrix, int row, int col, void *value);
void *matrix_get(struct matrix_t *matrix, int row, int col);

void matrix_clear(struct matrix_t *matrix);

int matrix_get_num_rows(struct matrix_t *matrix);
int matrix_get_num_cols(struct matrix_t *matrix);

#endif

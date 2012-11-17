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


#include <lib/mhandle/mhandle.h>

#include "debug.h"
#include "matrix.h"


struct matrix_t
{
	int num_rows;
	int num_cols;

	void **elem;
};


struct matrix_t *matrix_create(int num_rows, int num_cols)
{
	struct matrix_t *matrix;

	/* Check sizes */
	matrix = xcalloc(1, sizeof(struct matrix_t));
	if (num_rows < 0 || num_cols < 0)
		fatal("%s: invalid sizes", __FUNCTION__);

	/* Initialize */
	matrix->num_rows = num_rows;
	matrix->num_cols = num_cols;

	/* Allocate elements array */
	if (num_rows > 0 && num_cols > 0)
		matrix->elem = xcalloc(num_rows * num_cols, sizeof(void *));

	/* Return */
	return matrix;
}


void matrix_free(struct matrix_t *matrix)
{
	if (matrix->elem)
		free(matrix->elem);
	free(matrix);
}


void matrix_set(struct matrix_t *matrix, int row, int col, void *value)
{
	/* Check range */
	if (row < 0 || row >= matrix->num_rows)
		return;
	if (col < 0 || col >= matrix->num_cols)
		return;

	/* Assign value */
	matrix->elem[row * matrix->num_cols + col] = value;
}


void *matrix_get(struct matrix_t *matrix, int row, int col)
{
	/* Check range */
	if (row < 0 || row >= matrix->num_rows)
		return NULL;
	if (col < 0 || col >= matrix->num_cols)
		return NULL;

	/* Return value */
	return matrix->elem[row * matrix->num_cols + col];
}


void matrix_clear(struct matrix_t *matrix)
{
	if (matrix->elem)
		memset(matrix->elem, 0, matrix->num_rows * matrix->num_cols * sizeof(void *));
}


int matrix_get_num_rows(struct matrix_t *matrix)
{
	return matrix->num_rows;
}


int matrix_get_num_cols(struct matrix_t *matrix)
{
	return matrix->num_cols;
}

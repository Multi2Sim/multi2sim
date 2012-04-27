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

#include <stdlib.h>

#include <debug.h>
#include <matrix.h>
#include <mhandle.h>


struct matrix_t
{
	int x_size;
	int y_size;

	void **elem;
};


struct matrix_t *matrix_create(int x_size, int y_size)
{
	struct matrix_t *matrix;

	/* Allocate */
	matrix = calloc(1, sizeof(struct matrix_t));
	if (!matrix)
		fatal("%s: out of memory", __FUNCTION__);

	/* Check sizes */
	if (x_size < 0 || y_size < 0)
		fatal("%s: invalid sizes", __FUNCTION__);

	/* Initialize */
	matrix->x_size = x_size;
	matrix->y_size = y_size;

	/* Allocate elements array */
	if (x_size > 0 && y_size > 0)
	{
		matrix->elem = calloc(x_size * y_size, sizeof(void *));
		if (!matrix->elem)
			fatal("%s: out of memory", __FUNCTION__);
	}

	/* Return */
	return matrix;
}


void matrix_free(struct matrix_t *matrix)
{
	if (matrix->elem)
		free(matrix->elem);
	free(matrix);
}


void matrix_set(struct matrix_t *matrix, int x, int y, void *value)
{
	/* Check range */
	if (x < 0 || x >= matrix->x_size)
		return;
	if (y < 0 || y >= matrix->y_size)
		return;

	/* Assign value */
	matrix->elem[x * matrix->y_size + y] = value;
}


void *matrix_get(struct matrix_t *matrix, int x, int y)
{
	/* Check range */
	if (x < 0 || x >= matrix->x_size)
		return NULL;
	if (y < 0 || y >= matrix->y_size)
		return NULL;

	/* Return value */
	return matrix->elem[x * matrix->y_size + y];
}


void matrix_clear(struct matrix_t *matrix)
{
	if (matrix->elem)
		memset(matrix->elem, 0, matrix->x_size * matrix->y_size * sizeof(void *));
}

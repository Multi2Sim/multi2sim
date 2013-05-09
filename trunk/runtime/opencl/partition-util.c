#include <time.h>
#include <string.h>
#include <limits.h>
#include "mhandle.h"
#include "partition-util.h"

#define MIN(a, b) ((a) < (b)? (a): (b))

long long get_time()
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);

	return 1000000000LL * t.tv_sec + t.tv_nsec;	
}


struct cube_t *cube_init(int dims, const unsigned int *size)
{
	unsigned int *whole;
	struct cube_t *cube;

	cube = xcalloc(1, sizeof (struct cube_t));
	cube->dims = dims;	
	cube->list = list_create();

	whole = xcalloc(2 * dims, sizeof (unsigned int));
	memset(whole, 0, dims * sizeof (unsigned int));
	memcpy(whole + dims, size, dims * sizeof (unsigned int));
	list_add(cube->list, whole);
	return cube;
}


void cube_remove_region(struct cube_t *cube, const unsigned int *start, const unsigned int *size)
{
	int i, j;
	struct list_t *splits = list_create();
	unsigned int *rem = xcalloc(2 * cube->dims, sizeof (unsigned int));
	memcpy(rem, start, 3 * sizeof (unsigned int));
	memcpy(rem + cube->dims, size, 3 * sizeof (unsigned int));

	/* remove the cube from the rest of the cubes */
	LIST_FOR_EACH(cube->list, i)
		remove_from_cube(cube->dims, splits, list_get(cube->list, i), rem);

	/* go backward through the list so that we don't skip elemnts when they're removed */
	for (int i = list_count(splits) - 1; i >= 0; i--)
	{
		LIST_FOR_EACH(splits, j)
		{
			if (i != j && covers_cube(cube->dims, (unsigned int *)list_get(splits, j), (unsigned int *)list_get(splits, i)))
			{
				list_remove_at(splits, i);
				break;
			}
		}
	}

	/* now set the new list */
	LIST_FOR_EACH(cube->list, i)
		free(list_get(cube->list, i));
	list_free(cube->list);
	cube->list = splits;

	free(rem);
	list_free(splits);
}


void cube_destroy(struct cube_t *cube)
{
	int i;
	LIST_FOR_EACH(cube->list, i)
		free(list_get(cube->list, i));

	list_free(cube->list);
	free(cube);
}


int get_intersection(int dims, unsigned int *inter, const unsigned int *a, const unsigned int *b)
{
	const unsigned int *a_start = a;
	const unsigned int *a_size = a + dims;
	const unsigned int *b_start = b;
	const unsigned int *b_size = b + dims;
	unsigned int *inter_start = inter;
	unsigned int *inter_size = inter + dims;

	int has = 1;
	int i;

	/* consider each dimension seperately */
	for (i = 0; i < dims; i++)
	{
		has = has && get_linear_intersection(inter_start, inter_size, *a_start, *a_size, *b_start, *b_size);

		a_start++;
		a_size++;
		b_start++;
		b_size++;
		inter_start++;
		inter_size++;
	}
	return has;
}


int get_linear_intersection(
	unsigned int *start, 
	unsigned int *len, 
	unsigned int ainit, 
	unsigned int alen, 
	unsigned int binit, 
	unsigned int blen)
{
	if (ainit <= binit && ainit + alen > binit)
	{
		*start = binit;
		*len = MIN(ainit + alen, binit + blen) - binit;
		return 1;
	}
	else if (binit < ainit && binit + blen > ainit)
	{
		*start = ainit;
		*len = MIN(ainit + alen, binit + blen) - ainit;
		return 1;
	}
	else
	{
		*start = 0;
		*len = 0;
		return 0;
	}
}

void remove_from_cube(
	int dims,
	struct list_t *result,
	const unsigned int *cube,
	const unsigned int *remove)
{
	int i, j;
	unsigned int *inter = xcalloc(2 * dims, sizeof (unsigned int));
	if (get_intersection(dims, inter, cube, remove))
	{
		for (i = 0; i < dims; i++)
		{
			unsigned int inter_start = inter[i];
			unsigned int inter_size = inter[dims + i];
			unsigned int cube_start = cube[i];
			unsigned int cube_size = cube[dims + i];

			/* Add cube before beginning */
			if (inter_start != cube_start)
			{
				unsigned int *rem = xcalloc(2 * dims, sizeof (unsigned int));
	
				for (j = 0; j < dims; j++)
				{
					/* start is original cube start */
					rem[j] = cube[j];

					/* set the size to either cube size or the cube remainder */
					if (j == i)
						rem[dims + j] = inter_start - cube_start;
					else
						rem[dims + j] = cube_size;
				}

				list_add(result, rem); /* record the new cube */
			}
			
			/* Add cube before end */
			if (inter_start + inter_size != cube_start + cube_size)
			{
				unsigned int *rem = xcalloc(2 * dims, sizeof (unsigned int));
				
				for (j = 0; j < dims; j++)
				{
					/* start is after subcube end */
					rem[j] = inter_start + inter_size;
					
					if (j == i)
						rem[dims + j] = cube_size - inter_size;
					else
						rem[dims + j] = cube_size;

				}

				list_add(result, rem);
			}
		}

	}
	else /* no intersection? then the whole cube is left */
	{
		unsigned int *rem = xcalloc(2 * dims, sizeof (unsigned int));
		memcpy(rem, cube, 2 * dims * sizeof (unsigned int));
		list_add(result, rem);
	}
	free(inter);
}


int covers_cube(int dims, unsigned int *cube, unsigned int *other)
{
	int i;
	for (i = 0; i < dims; i++)
		if (cube[i] > other[i] || cube[i] + cube[dims + i] < other[i] + other[dims + i])
			return 0;

	return 1;
}


unsigned int get_centroid_distance(int dims, unsigned int *cube, unsigned int *point)
{
	int i;
	int dist = 0;
	/* calculate Manhattan distance for now */
	for (i = 0; i < dims; i++)
		dist += abs((cube[i] + cube[dims + i]) / 2 - point[i]);
	return dist;
}

void cube_get_region(
	struct cube_t *cube,
	unsigned int *start_out, 
	unsigned int *size_out, 
	const unsigned int *target_size, 
	const unsigned int *target_loc)
{
	
}

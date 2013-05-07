#include <time.h>
#include <string.h>
#include "mhandle.h"
#include "partition-util.h"

long long get_time()
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);

	return 1000000000LL * t.tv_sec + t.tv_nsec;	
}

struct cube_t *cube_init(int dims, unsigned int *size)
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

void cube_remove_region(unsigned int *start, unsigned int *size)
{
	
}

void cube_destroy(struct cube_t *cube)
{
	int i;
	LIST_FOR_EACH(cube->list, i)
		free(list_get(cube->list, i));

	list_free(cube->list);
	free(cube);
}

int get_intersection(int dims, unsigned int *inter, unsigned int *a, unsigned int *b)
{
	unsigned int *a_start = a;
	unsigned int *a_size = a + dims;
	unsigned int *b_start = b;
	unsigned int *b_size = b + dims;
	unsigned int *inter_start = inter;
	unsigned int *inter_size = inter + dims;

	int has = 1;
	int i;

	/* consider each dimension seperately */
	for (i = 0; i < dims; i++)
	{
		unsigned int ainit = *a_start;
//		unsigned int afin = *a_start + *a_size;
		unsigned int binit = *b_start;
		unsigned int bfin = *b_start + *b_size;

		if (ainit >= binit && ainit < bfin)
		{
			inter_start[i] = ainit;
			inter_size[i] = ainit - bfin;
		}

		a_start++;
		a_size++;
		b_start++;
		b_size++;
	}
	return has;
}

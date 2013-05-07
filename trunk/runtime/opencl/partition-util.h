#ifndef __PARTITION_UTIL_H__
#define __PARTITION_UTIL_H__

#include "list.h"

long long get_time();

struct cube_t
{
	int dims;
	struct list_t *list; /* each int *, allocated 2 * dims ints. */
};

struct cube_t *cube_init(int dims, unsigned int *size);
void cube_remove_region(unsigned int *start, unsigned int *size);
void cube_destroy(struct cube_t *cube);

/* non-member helper functions */
int get_intersection(int dims, unsigned int *inter, unsigned int *a, unsigned int *b);


#endif

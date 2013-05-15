#ifndef __PARTITION_UTIL_H__
#define __PARTITION_UTIL_H__

#include "list.h"

long long get_time();

struct partition_info_t
{
	int num_devices;
	unsigned int dims;
	unsigned int *groups;
};

struct partition_info_t *partition_info_create(int num_devices, unsigned int dims, const unsigned int *groups);
void partition_info_free(struct partition_info_t *info);
unsigned int closest_multiple_not_more(unsigned int value, unsigned int factor, unsigned int max);

struct cube_t
{
	int dims;
	struct list_t *list; /* each int *, allocated 2 * dims ints. */
};

struct cube_t *cube_init(int dims, const unsigned int *size);
void cube_remove_region(struct cube_t *cube, const unsigned int *start, const unsigned int *size);
void cube_destroy(struct cube_t *cube);
void cube_get_region(
	struct cube_t *cube,
	unsigned int *start_out, 
	unsigned int *size_out, 
	const unsigned int *target_size, 
	const unsigned int *target_loc);

/* non-member helper functions */
int get_intersection(
	int dims, 
	unsigned int *inter, 
	const unsigned int *a, 
	const unsigned int *b);

int get_linear_intersection(
	unsigned int *start, 
	unsigned int *len, 
	unsigned int ainit, 
	unsigned int alen, 
	unsigned int binit, 
	unsigned int blen);

void remove_from_cube(
	int dims,
	struct list_t *result,
	const unsigned int *cube,
	const unsigned int *remove);

int covers_cube(int dims, unsigned int *cube, unsigned int *other);

unsigned int get_centroid_distance(int dims, unsigned int *cube, unsigned int *point);

#endif

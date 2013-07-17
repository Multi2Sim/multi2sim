#ifndef __PARTITION_UTIL_H__
#define __PARTITION_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
#include "partition-util-time.h"

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
int cube_get_region(
	struct cube_t *cube,
	unsigned int *start_out, 
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

/* functions for getting proportions */
unsigned int proportions_from_string(const char *str, unsigned int count, unsigned int *prop);
void normalize_proportions(unsigned int new_sum, unsigned int count, unsigned int *prop);
unsigned int convert_fraction(unsigned int num, unsigned int den, unsigned int new_den);

void shape_local_size(unsigned int work_dim, size_t total, const size_t *global, size_t *local);
size_t get_factor(size_t *num);

#ifdef __cplusplus
}
#endif


#endif

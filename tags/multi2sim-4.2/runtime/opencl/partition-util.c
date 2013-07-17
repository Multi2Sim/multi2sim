#include <time.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "partition-util.h"
#include "partition-util-time.h"

#define MIN(a, b) ((a) < (b)? (a): (b))
#define MAX(a, b) ((a) > (b)? (a): (b))

static unsigned int primes[] =
{2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 
31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 
73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 
127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 
179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 
233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 
283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 
353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 
419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 
467, 479, 487, 491, 499, 503, 509, 521, 523, 541, 
547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 
607, 613, 617, 619, 631, 641, 643, 647, 653, 659, 
661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 
739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 
811, 821, 823, 827, 829, 839, 853, 857, 859, 863, 
877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 
947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013, 
1019, 1021, 1031};

struct partition_info_t *partition_info_create(int num_devices, unsigned int dims, const unsigned int *groups)
{
	struct partition_info_t *info = (struct partition_info_t *)calloc(1, sizeof (struct partition_info_t));
	info->num_devices = num_devices;
	info->dims = dims;
	info->groups = (unsigned int *)calloc(dims, sizeof (unsigned int));
	memcpy(info->groups, groups, dims * sizeof (unsigned int));
	return info;
}


void partition_info_free(struct partition_info_t *info)
{
	free(info->groups);
	free(info);
}

unsigned int closest_multiple_not_more(unsigned int value, unsigned int factor, unsigned int max)
{
	unsigned int rem = value % factor;
	/* if the remander is equal to or more than half, go bigger */
	if (rem >= factor / 2)
		return MIN(value + factor - rem, max);
	/* max sure we're not just retuning zero if there's still work to do */
	else if (value != rem)
		return MIN(value - rem, max);
	else
		return MIN(factor, max);
}


struct cube_t *cube_init(int dims, const unsigned int *size)
{
	unsigned int *whole;
	struct cube_t *cube;

	cube = calloc(1, sizeof (struct cube_t));
	cube->dims = dims;	
	cube->list = list_create();

	whole = calloc(2 * dims, sizeof (unsigned int));
	memset(whole, 0, dims * sizeof (unsigned int));
	memcpy(whole + dims, size, dims * sizeof (unsigned int));
	list_add(cube->list, whole);
	return cube;
}


void cube_remove_region(struct cube_t *cube, const unsigned int *start, const unsigned int *size)
{
	int i, j;
	struct list_t *splits = list_create();
	unsigned int *rem = calloc(2 * cube->dims, sizeof (unsigned int));
	memcpy(rem, start, cube->dims * sizeof (unsigned int));
	memcpy(rem + cube->dims, size, cube->dims * sizeof (unsigned int));

	/* remove the cube from the rest of the cubes */
	LIST_FOR_EACH(cube->list, i)
	{
		unsigned int *region = list_get(cube->list, i);
		remove_from_cube(cube->dims, splits, region, rem);
		free(region);
	}
	list_free(cube->list);

	/* now set the new list */
	cube->list = splits;
	free(rem);
	
	/* go backward through the list so that we don't skip elemnts when they're removed */
	for (i = list_count(splits) - 1; i >= 0; i--)
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
	unsigned int *inter = calloc(2 * dims, sizeof (unsigned int));

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
				unsigned int *rem = calloc(2 * dims, sizeof (unsigned int));
	
				for (j = 0; j < dims; j++)
				{
					/* start is original cube start */
					rem[j] = cube[j];

					/* set the size to either cube size or the cube remainder */
					if (j == i)
						rem[dims + j] = inter_start - cube_start;
					else
						rem[dims + j] = cube[dims + j];
				}
				list_add(result, rem); /* record the new cube */
			}
			
			/* Add cube before end */
			if (inter_start + inter_size != cube_start + cube_size)
			{
				unsigned int *rem = calloc(2 * dims, sizeof (unsigned int));
				
				for (j = 0; j < dims; j++)
				{
					if (j == i)
					{
						rem[j] = inter_start + inter_size;
						rem[dims + j] = cube_size - inter_size - (inter_start - cube_start);
					}
					else
					{
						rem[j] = cube[j];
						rem[dims + j] = cube[dims + j];
					}

				}
				list_add(result, rem);
			}
		}

	}
	else /* no intersection? then the whole cube is left */
	{
		unsigned int *rem = calloc(2 * dims, sizeof (unsigned int));
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


unsigned int get_centroid_distance_sq(int dims, const unsigned int *cube, const unsigned int *point)
{
	int i;
	int dist = 0;
	for (i = 0; i < dims; i++)
	{
		int d = (cube[i] + cube[dims + i]) / 2 - point[i];
		dist += d * d;
	}
	return dist;
}

void get_closest_point(int dims, unsigned int *out, const unsigned int *point, const unsigned int *cube)
{
	int inside = 1;
	int i;

	for (i = 0; i < dims; i++)
		if (cube[i] > point[i] || cube[i] + cube[dims + i] <= point[i])
			inside = 0;
	
	if (inside)
		memcpy(out, point, dims * sizeof (unsigned int));
	else
	{
		int j;
		int normal_dim = -1;
		unsigned int close_dist = UINT_MAX;
		unsigned int *closest = calloc(2 * dims, sizeof (unsigned int));
		unsigned int *face = calloc(2 * dims, sizeof (unsigned int));
		unsigned int cur_dist;

		/* go through each of 2n (n - 1)-dimensional surface that the n dimensional cube has */
		for (i = 0; i < dims; i++)
		{
			/* for each dimension there is a near face and a far face */
			memcpy(face, cube, 2 * dims * sizeof (unsigned int));
			for (j = 0; j < 2; j++)
			{
				if (j)
					face[i] += face[dims + i];
				face[dims + i] = 0;
			}

			/* find the closest face */
			cur_dist = get_centroid_distance_sq(dims, face, point);
			if (cur_dist < close_dist)
			{
				close_dist = cur_dist;
				memcpy(closest, face, 2 * dims * sizeof (unsigned int));
				normal_dim = i;
			}
		}
		free(face);

		/* now closest contains the face closest to the point.
		   go through the dimensions again, looking at all 
		   the dimensions of the face */
		

		/* initialize the point to the starting point of the face */
		memcpy(out, closest, dims * sizeof (unsigned int));

		for (i = 0; i < dims; i++)
		{
			if (i != normal_dim)
			{
				/* distance to the centroid from the face */
				int centroid = (cube[i] + cube[dims + i]) / 2;
				int centroid_dist = abs((int)closest[i] - centroid);
				/* distance to the point from the face */
				int point_dist = abs((int)closest[i] - point[i]);

				/* total distance between the two (point is outside of cube) */
				int total = centroid_dist + point_dist;

				int centroid_weight = total - point_dist;
				int point_weight = total - centroid_dist;
				out[i] = (centroid_weight * centroid + point_weight * point[i]) / total;
			}
		}

		free(closest);
	}		
}

unsigned int distance_squared(int dims, const unsigned int *a, const unsigned int *b)
{
	int i;
	unsigned int dist = 0;
	for (i = 0; i < dims; i++)
	{
		int d = a[i] - b[i];
		dist += d * d;
	}
	return dist;
}


int can_contain(int dims, const unsigned int *me, const unsigned int *that)
{
	int i;
	for (i = 0; i < dims; i++)
		if (me[i] < that[i])
			return 0;
	return 1;
}

unsigned int squared(int x)
{
	return x * x;
}

int cube_get_region(
	struct cube_t *cube,
	unsigned int *start_out, 
	const unsigned int *target_size, 
	const unsigned int *target_loc)
{
	int i;
	int found = 0;
	unsigned int min_dist_squared = UINT_MAX;
	unsigned int *cur_start = calloc(cube->dims, sizeof (unsigned int));

	/* consider each cube in the list */
	LIST_FOR_EACH(cube->list, i)
	{
		unsigned int *cur = list_get(cube->list, i);
		/* if the cube is big enough to hold the target */
		if (can_contain(cube->dims, cur + cube->dims, target_size))
		{
			int j;
			/* go dimension by dimension and figure out which corner is best
			   we limit the possibilities to corners to reduce fragmentation */
			unsigned int cur_dist_squared = 0;
			for (j = 0; j < cube->dims; j++)
			{
				int near_center = cur[j] + target_size[j] / 2;
				int far_center = cur[cube->dims + j] + cur[j] - target_size[j] / 2;
				
				if (abs(target_loc[j] - near_center) < abs(target_loc[j] - far_center))
				{
					cur_start[j] = cur[j];
					cur_dist_squared += squared(target_loc[j] - near_center);
				}
				else
				{
					cur_start[j] = cur[j] + cur[cube->dims + j] - target_size[j];
					cur_dist_squared += squared(target_loc[j] - far_center);
				}
			}

			if (cur_dist_squared < min_dist_squared)
			{
				min_dist_squared = cur_dist_squared;
				memcpy(start_out, cur_start, cube->dims * sizeof (unsigned int));
				found = 1;
			}
		}
	}
	free(cur_start);
	return found;
}


unsigned int proportions_from_string(const char *str, unsigned int count, unsigned int *prop)
{
	char *mstr;
	char *cur;
	unsigned int i;
	unsigned int sum = 0;

	/* copy string into multable copy */
	mstr = (char *)malloc(strlen(str) + 1);
	strcpy(mstr, str);

	/* tokenize on colon */
	cur = strtok(mstr, ":");
	for (i = 0; i < count; i++)
	{
		if (cur)
		{
			prop[i] = atoi(cur);
			sum += prop[i];
			cur = strtok(NULL, ":");
		}
		else /* if we run out of tokens, just put zero */
			prop[i] = 0;
	}

	free(mstr);
	return sum;
}

void normalize_proportions(unsigned int new_sum, unsigned int count, unsigned int *prop)
{
	unsigned int i;
	unsigned int old_sum = 0;
	unsigned int so_far;

	for (i = 0; i < count; i++)
		old_sum += prop[i];

	so_far = 0;
	for (i  = 0; i < count; i++)
	{
		unsigned int start;
		unsigned int end;

		start = convert_fraction(so_far, old_sum, new_sum);
		so_far += prop[i];
		end = convert_fraction(so_far, old_sum, new_sum);
		prop[i] = end - start;
	}
}

unsigned int convert_fraction(unsigned int num, unsigned int den, unsigned int new_den)
{
	unsigned long long product = num * new_den;
	return product / den;
}

size_t get_factor(size_t *num)
{
	int num_primes = sizeof primes / sizeof primes[0];
	int i;
	size_t ret;

	for (i = 0; i < num_primes && *num >= primes[i] * primes[i]; i++)
	{
		if (*num % primes[i] == 0)
		{
			*num /= primes[i];
			return primes[i];
		}
	}

	ret = *num;
	*num = 1;
	return ret;
}

void shape_local_size(unsigned int work_dim, size_t total, const size_t *global, size_t *local)
{
	unsigned int i;
	unsigned int dim_done;
	size_t gl[3];

	for (i = 0; i < work_dim; i++)
	{
		gl[i] = global[i];
		local[i] = 1;
	}
	
	dim_done = 0;

	while (dim_done < work_dim)
	{
		for (i = 0; i < work_dim; i++)
		{
			size_t fact = get_factor(gl + i);
			if (fact <= total)
			{
				total /= fact;
				local[i] *= fact;
			}
			else
				dim_done++;
		}
	}
}

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "relative-runtime-partition-strategy.h"
#include "partition-util.h"

struct per_device_info_t
{
	long long start;
	unsigned int groups;
	float speed;
};

struct relative_runtime_strategy_t
{
	struct partition_info_t *info;
	struct per_device_info_t *device_info;
	unsigned int groups_left;
	struct cube_t *cube;
};

void *relative_runtime_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups)
{
	int i;

	struct relative_runtime_strategy_t *info = (struct relative_runtime_strategy_t *)calloc(1, sizeof (struct relative_runtime_strategy_t));
	info->info = partition_info_create(num_devices, dims, groups);
	info->device_info = (struct per_device_info_t *)calloc(num_devices, sizeof (struct per_device_info_t));
	info->cube = cube_init(dims, groups);
	
	/* calculate the total number of groups */
	info->groups_left = 1;
	for (i = 0; i < (int)dims; i++)
		info->groups_left *= groups[i];
	return info;
}

int relative_runtime_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count)
{
	int i;
	float total_speed;
	unsigned int target_groups;
	unsigned int row_size;
	unsigned int *pos;
	struct relative_runtime_strategy_t *info = (struct relative_runtime_strategy_t *)inst;
	struct per_device_info_t *dev = info->device_info + id;
	long long now;
	int found;

	if (!info->groups_left)
		return 0;

	/* if the start has been set, update the speed */
	now = get_time();
	if (dev->start)
		dev->speed = (float)dev->groups / (now - dev->start);

	/* now update the start date */
	dev->start = now;

	/* calculate the total throughput */
	total_speed = 0;
	for (i = 0; i < info->info->num_devices; i++)
	{
		struct per_device_info_t *other = info->device_info + i;
		/* if another device hasn't completed yet, assume it's just about to complete. */
		if (other->speed == 0)
		{
			/* that device hasn't even been scheduled anything yet..just say it's as fast as us */
			if (other->groups == 0)
				total_speed += dev->speed;
			else
				total_speed += (float)other->groups / (now - other->start);
		}
		else
			total_speed += other->speed;
	}

	/* allocate groups proportional to performance.  Allocate half the remainder */
	target_groups = (unsigned int)(info->groups_left * dev->speed / total_speed / 2);
	target_groups = closest_multiple_not_more(target_groups, desired_groups, info->groups_left);

	/* for now, just assume row major order and only vary the highest dimension */

	/* how big is a highest-dimension 'row' */
	row_size = 1;
	for (i = 0; i < (int)info->info->dims - 1; i++)
	{
		group_count[i] = info->info->groups[i];
		row_size *= info->info->groups[i];
	}
	/* target groups should be a multiple of the row size */
	target_groups = closest_multiple_not_more(target_groups, row_size, info->groups_left);
	assert(target_groups % row_size == 0);
	/* now we know the shape in every dimension */
	group_count[info->info->dims - 1] = target_groups / row_size;
	info->groups_left -= target_groups;

	/* decide what point in the NDRange to prefer */
	pos = (unsigned int *)calloc(info->info->dims, sizeof (unsigned int));
	memset(pos, 0, info->info->dims * sizeof (unsigned int));

	/* make even devices prefer the top, and odd devices prefer the bottom */
	pos[info->info->dims - 1] = info->info->groups[info->info->dims - 1] * (id % 2);

	found = cube_get_region(info->cube, group_offset, group_count, pos);
	assert(found);
	free(pos);

	cube_remove_region(info->cube, group_offset, group_count);
	
	return 1;	
}

void relative_runtime_strategy_destroy(void *inst)
{
	struct relative_runtime_strategy_t *info = (struct relative_runtime_strategy_t *)inst;
	partition_info_free(info->info);
	cube_destroy(info->cube);
	free(info->device_info);
	free(info);
}

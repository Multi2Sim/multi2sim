#include "mhandle.h"
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

	struct relative_runtime_strategy_t *info = xcalloc(1, sizeof (struct relative_runtime_strategy_t));
	info->info = partition_info_create(num_devices, dims, groups);
	info->device_info = xcalloc(num_devices, sizeof (struct per_device_info_t));
	info->cube = cube_init(dims, groups);
	
	/* calculate the total number of groups */
	info->groups_left = 1;
	for (i = 0; i < dims; i++)
		info->groups_left *= groups[i];
	return info;
}

int relative_runtime_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count)
{
	int i;
	float total_speed;
	unsigned int target_groups;
	struct relative_runtime_strategy_t *info = inst;
	struct per_device_info_t *dev = info->device_info + id;
	
	/* if the start has been set, update the speed */
	long long now = get_time();
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
	target_groups = info->groups_left * dev->speed / total_speed / 2;
	target_groups = closest_multiple_not_more(target_groups, desired_groups, info->groups_left);
	
	return 0;	
}

void relative_runtime_strategy_destroy(void *inst)
{
	struct relative_runtime_strategy_t *info = inst;
	partition_info_free(info->info);
	cube_destroy(info->cube);
	free(info->device_info);
	free(info);
}

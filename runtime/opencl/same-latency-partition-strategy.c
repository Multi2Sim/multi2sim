#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "same-latency-partition-strategy.h"
#include "partition-util.h"

struct same_latency_device_t
{
	struct stop_watch_t time;
	unsigned int num_parts_done;
	unsigned int num_parts_running;
	unsigned int num_parts_assigned;
};

struct same_latency_strategy_info_t
{
	struct partition_info_t *part;
	struct same_latency_device_t *devices;
	unsigned int num_parts_assigned;
	unsigned int part_dim;
	unsigned int all_done;
};

void *same_latency_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups)
{
	struct same_latency_strategy_info_t *info = (struct same_latency_strategy_info_t *)calloc(1, sizeof *info);
	info->part = partition_info_create(num_devices, dims, groups);
	info->devices = (struct same_latency_device_t *)calloc(num_devices, sizeof (struct same_latency_device_t));
	info->part_dim = pick_partition_dimension(dims, groups);
	return info;
}


int same_latency_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count, long long now)
{
	struct same_latency_strategy_info_t *info = (struct same_latency_strategy_info_t *)inst;
	struct same_latency_device_t *device = info->devices + id;
	unsigned int total_parts = info->part->groups[info->part_dim];
	unsigned int parts_left = total_parts - info->num_parts_assigned;
	unsigned int size = 0;

	device->num_parts_done += device->num_parts_running;
	device->num_parts_running = 0;

	stop_watch_new_interval(&device->time, now);

	if (device->num_parts_assigned != 0)
	{
		size = device->num_parts_assigned;
		device->num_parts_assigned = 0;
	}
	else if (!info->all_done)
	{
		unsigned int i;
		info->all_done = 1;
		for (i = 0; i < info->part->num_devices; i++)
			if (info->devices[i].num_parts_done == 0)
				info->all_done = 0;

		if (info->all_done)
		{
			/* allocate the work for all devices */
			long long latency[2];
			unsigned int running[2];
			long long start[2];
			int work[2];

			assert(info->part->num_devices == 2);
			
			for (i = 0; i < info->part->num_devices; i++)
			{
				latency[i] = info->devices[i].time.total / info->devices[i].num_parts_done; /* integer divide should be ok */
				running[i] = info->devices[i].num_parts_running;
				start[i] = info->devices[i].time.start;
				assert(latency[i] > 0);
				if (start[i] <= 0 || start[i] > now)
				{
					fprintf(stderr, "Start: %lld.  Now: %lld\n", start[i], now);
					abort();
				}

			}
			work[1] = latency[0] * (parts_left + running[0]) - running[1] * latency[1] + start[0] - start[1];
			work[1] /= latency[1] + latency[0];

			if (work[1] < 0)
				work[1] = 0;
			if (work[1] > parts_left)
				work[1] = parts_left;

			work[0] = parts_left - work[1];

			for (i = 0; i < info->part->num_devices; i++)
			{
				if (i == id)
					size = work[i];
				else
					info->devices[i].num_parts_assigned = work[i];
			}
			
		}
		else
		{
			size = total_parts / 8;
			if (size > parts_left)
				size = parts_left;
			if (size == 0 && parts_left > 0)
				size = 1;
		}
	}

	populate_partition_info(
		info->num_parts_assigned, 
		size, 
		info->part_dim, 
		info->part, 
		group_offset, 
		group_count);

	info->num_parts_assigned += size;
	device->num_parts_running = size;

	return size != 0;
}


void same_latency_strategy_destroy(void *inst)
{
	struct same_latency_strategy_info_t *info = (struct same_latency_strategy_info_t *)inst;
	partition_info_free(info->part);
	free(info->devices);
	free(info);
}

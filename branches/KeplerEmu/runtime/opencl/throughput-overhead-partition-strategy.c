#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "throughput-overhead-partition-strategy.h"
#include "partition-util.h"

static const char *M2S_OPENCL_DEVICE_OVERHEAD_NS = "M2S_OPENCL_DEVICE_OVERHEAD_NS";
static int to_partition_times = 3;

struct to_device_info_t
{
	long long overhead;
	struct stop_watch_t time;
	unsigned int num_planes_done;
	unsigned int num_planes_running;
	int num_launches;
};


struct to_partition_info_t
{
	struct partition_info_t *part;
	struct to_device_info_t *devices;
	unsigned int part_dim;
	int num_launches_left;
	unsigned int num_planes_assigned;
	unsigned int groups_per_plane;
};


void to_partition_get_overheads(int num_devices, struct to_device_info_t *devices)
{
	const char *overhead_data;
	unsigned int *overheads;
	int i;

	overhead_data = getenv(M2S_OPENCL_DEVICE_OVERHEAD_NS);
	overheads = (unsigned int *)calloc(num_devices, sizeof (unsigned int)); /* zero-fill */
	if (overhead_data) /* keep overheds 0 by default */
		proportions_from_string(overhead_data, num_devices, overheads);	

	/* populate overheads for devices */
	for (i = 0; i < num_devices; i++)
		devices[i].overhead = overheads[i];

	free(overheads);
}


void *throughput_overhead_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups)
{
	int i;

	struct to_partition_info_t *info = (struct to_partition_info_t *)calloc(1, sizeof (struct to_partition_info_t));
	info->part = partition_info_create(num_devices, dims, groups);
	info->devices = (struct to_device_info_t *)calloc(num_devices, sizeof (struct to_device_info_t));
	

	to_partition_get_overheads(num_devices, info->devices);

	/* determine which dimension to partition */
	for (i = 1; i < dims; i++)
		if (groups[i] >= groups[info->part_dim])
			info->part_dim = i;

	/* now that it's calculated, how many groups per plane are there? */
	info->groups_per_plane = 1;
	for (i = 0; i < dims; i++)
		if (i != info->part_dim)
			info->groups_per_plane *= groups[i];

	info->num_launches_left = num_devices * to_partition_times;
		
	return info;
}


long long to_device_latency_per_plane(struct to_device_info_t *dev, struct to_device_info_t *safe, long long now)
{
	if (dev->num_planes_done == 0)
	{
		if (dev->num_planes_running == 0)
			return to_device_latency_per_plane(safe, NULL, now);
		else
			return (now - dev->time.start) / dev->num_planes_running;
	}
	else
	{
		long long overhead = dev->overhead * dev->num_launches;
		if (overhead > dev->time.total)
			return 0;
		else
			return (overhead - dev->time.total) / dev->num_planes_done;
	
	}
}


unsigned int to_partition_determine_planes(struct to_partition_info_t *info, struct to_device_info_t *device, int desired_groups, long long now)
{
	unsigned int num_planes;
	unsigned int num_planes_left = info->part->groups[info->part_dim] - info->num_planes_assigned;
	int i;

	if (device->num_launches == 0)
	{
		/* Get 10% / to_partition_times of the work to start */
		num_planes = info->part->groups[info->part_dim] / (10 * to_partition_times);
		if (num_planes > num_planes_left)
			num_planes = num_planes_left;
		else if (num_planes < desired_groups)
			num_planes = round_up_not_more(
				desired_groups, 
				info->groups_per_plane, 
				info->groups_per_plane * num_planes_left) / info->groups_per_plane;
	}
	else
	{
		unsigned int num_planes_left = info->part->groups[info->part_dim] - info->num_planes_assigned;
		unsigned int segments_left;
		unsigned int num_planes_allocatable;

		if (info->num_launches_left < info->part->num_devices)
			segments_left = 1;
		else
			segments_left = info->num_launches_left / info->part->num_devices;

		num_planes_allocatable = convert_fraction(1, segments_left, num_planes_left);

		/* calculate projected latency */
		float device_throughput;
		float total_throughput = 0;
		for (i = 0; i < info->part->num_devices; i++)
		{
			struct to_device_info_t *dev = info->devices + i;
			float cur_throughput = 1.0 / (to_device_latency_per_plane(dev, device, now) + dev->overhead);
			
			if (dev == device)
				device_throughput = cur_throughput;
			total_throughput += cur_throughput;
		}
		float f = device_throughput * num_planes_allocatable / total_throughput;
		if ((int)f < f)
			return f + 1;
		else
			return f;
	}
	return num_planes;
}


int throughput_overhead_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count, long long now)
{
	int i;
	unsigned int num_planes;
	struct to_partition_info_t *info = (struct to_partition_info_t *)inst;
	struct to_device_info_t *device = info->devices + id;

	/* we're only going to partition down one dimension */
	for (i = 0; i < info->part->dims; i++)
	{
		if (i != info->part_dim)
		{
			group_offset[i] = 0;
			group_count[i] = info->part->groups[i];
		}
	}
	group_offset[info->part_dim] = info->num_planes_assigned; /* just start where we left off */
	
	/* record the effects of the last group */
	stop_watch_new_interval(&device->time, now);
	device->num_planes_done += device->num_planes_running;
	device->num_planes_running = 0;

	num_planes = to_partition_determine_planes(info, device, desired_groups, now);

	/* this is the first time for this device */


	if (num_planes != 0)
	{
		group_count[info->part_dim] = num_planes;
		device->num_planes_running = num_planes;
		device->num_launches++;
		info->num_planes_assigned += num_planes;
		if (info->num_launches_left > 1)
			info->num_launches_left--;
		return 1;
	}
	else
		return 0;
}

void throughput_overhead_strategy_destroy(void *inst)
{
	struct to_partition_info_t *info = (struct to_partition_info_t *)inst;
	partition_info_free(info->part);
	free(info->devices);
	free(info);
}

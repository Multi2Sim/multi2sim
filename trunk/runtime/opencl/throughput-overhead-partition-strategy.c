#include <stdlib.h>
#include <string.h>
#include "throughput-overhead-partition-strategy.h"
#include "partition-util.h"

static const char *M2S_OPENCL_DEVICE_OVERHEAD_NS = "M2S_OPENCL_DEVICE_OVERHEAD_NS";
static int to_partition_times = 3;

struct to_device_info_t
{
	long long overhead;
	long long start;
	long long total_time;
	size_t groups_running;
	size_t groups_done;
	int times;

};

struct to_partition_info_t
{
	int num_devices;
	struct to_device_info_t *devices;
	unsigned int *groups;
	unsigned int dims;
	unsigned int part_dim;
	int max_invokes;
	int num_invokes;
	size_t num_done;
};


void *throughput_overhead_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups)
{
	int i;
	const char *overhead_data;
	unsigned int *overheads;

	struct to_partition_info_t *info = (struct to_partition_info_t *)calloc(1, sizeof (struct to_partition_info_t));
	info->devices = (struct to_device_info_t *)calloc(num_devices, sizeof (struct to_device_info_t));
	info->groups = (unsigned int *)calloc(dims, sizeof (unsigned int));
	memcpy(info->groups, groups, dims * sizeof (unsigned int));
	
	overhead_data = getenv(M2S_OPENCL_DEVICE_OVERHEAD_NS);

	overheads = (unsigned int *)calloc(num_devices, sizeof (unsigned int)); /* zero-fill */
	if (overhead_data) /* keep overheds 0 by default */
		proportions_from_string(overhead_data, num_devices, overheads);	

	/* populate overheads for devices */
	for (i = 0; i < num_devices; i++)
		info->devices[i].overhead = overheads[i];

	free(overheads);

	/* determine which dimension to partition */
	for (i = 1; i < dims; i++)
		if (groups[i] >= groups[info->part_dim])
			info->part_dim = i;

	info->max_invokes = num_devices * to_partition_times;
	
	return info;
}

int throughput_overhead_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count)
{
	int i;
	unsigned int job;
	struct to_partition_info_t *info = (struct to_partition_info_t *)inst;
	struct to_device_info_t *device = info->devices + id;
	unsigned int groups_left = info->groups[info->part_dim] - info->num_done;

	/* we're only going to partition down one dimension */
	unsigned int size = 1;
	for (i = 0; i < info->dims; i++)
	{
		if (i != info->part_dim)
		{
			group_offset[i] = 0;
			group_count[i] = info->groups[i];
			size *= group_count[i];
		}
	}
	

	/* this is the first time for this device */
	if (device->times == 0) 		
	{
		job = closest_multiple_not_more(size, desired_groups, size * groups_left);
	}
	else
	{
		long long now = get_time();
		device->times++;
		device->groups_done += device->groups_running;
		device->total_time += now - device->start;
		

		/* calculate total throughput */
		double total_rate = 0;
		double self_rate = 0;

		for (i = 0; i < info->num_devices; i++)
		{
			struct to_device_info_t *d = info->devices + i;
			double rate;
			if (d->times > 0)
				rate = (double)d->groups_done / (d->total_time - d->times * d->overhead);
			else
				rate = (double)d->groups_running / (now - d->start);

			total_rate += rate;
			if (i == id)
				self_rate = rate;
		}

		unsigned int consider = groups_left * (info->max_invokes - info->num_invokes) / info->max_invokes;
		job = convert_fraction(self_rate, total_rate, consider);
		job = closest_multiple_not_more(size, job, size * groups_left);
		
	}

	group_count[info->part_dim] = job;
	group_offset[info->part_dim] = info->num_done;
	info->num_done += job;
	device->groups_running = job;
	info->num_invokes++;

	device->start = get_time();
	return 1;
}

void throughput_overhead_strategy_destroy(void *inst)
{
	struct to_partition_info_t *info = (struct to_partition_info_t *)inst;
	free(info->groups);
	free(info->devices);
	free(info);
}

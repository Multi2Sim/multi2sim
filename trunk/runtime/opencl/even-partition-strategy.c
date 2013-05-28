#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <stdio.h>
#include "even-partition-strategy.h"
#include "partition-util-time.h"
#include "partition-util.h"

static const char *M2S_OPENCL_EVEN_PARTITION_RATIO = "M2S_OPENCL_EVEN_PARTITION_RATIO";
static const int even_strategy_parts_per_device = 1;

struct even_strategy_info_t
{
	int num_devices;
	unsigned int dims;
	unsigned int *groups;
	int *done;
	unsigned int total_rows;
	unsigned int *device_rows;
	unsigned int *previous_rows;
};

void *even_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups)
{
	int i;
	char *ratio_data;
	struct even_strategy_info_t *info = (struct even_strategy_info_t *)calloc(1, sizeof (struct even_strategy_info_t));
	info->num_devices = num_devices;
	info->dims = dims;
	info->groups = (unsigned int *)calloc(dims, sizeof (unsigned int));
	memcpy(info->groups, groups, dims * sizeof (unsigned int));
	info->done = (int *)calloc(num_devices, sizeof (int)); /* initialized to zero */

	/* Set the ratio with which to partition stuff */
	info->device_rows = (unsigned int *)calloc(num_devices, sizeof (unsigned int));
	info->previous_rows = (unsigned int *)calloc(num_devices, sizeof (unsigned int));

	ratio_data = getenv(M2S_OPENCL_EVEN_PARTITION_RATIO);
	if (ratio_data)
		proportions_from_string(ratio_data, num_devices, info->device_rows);
	else /* no environmental variable? even partitioning */
		for (i = 0; i < num_devices; i++)
			info->device_rows[i] = 1;

	normalize_proportions(groups[dims - 1], num_devices, info->device_rows);
	for (i = 0; i < num_devices - 1; i++)
		info->previous_rows[i + 1] = info->previous_rows[i] + info->device_rows[i];

	info->total_rows = groups[dims - 1];
	return info;
}


int even_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count)
{
	struct even_strategy_info_t *info = (struct even_strategy_info_t *)inst;
	if (info->done[id] < even_strategy_parts_per_device)
	{
		/* partition with highest dimension */
		unsigned int start_offset = convert_fraction(info->done[id], even_strategy_parts_per_device, info->device_rows[id]);
		unsigned int size = convert_fraction(info->done[id] + 1, even_strategy_parts_per_device, info->device_rows[id]) - start_offset;
	
		unsigned int i;

		for (i = 0; i < info->dims - 1; i++)
		{
			group_offset[i] = 0;
			group_count[i] = info->groups[i];
		}

		group_offset[info->dims - 1] = info->previous_rows[id] + start_offset;
		group_count[info->dims - 1] = size;
		info->done[id]++;
		return 1;
	}
	else
		return 0;
}


void even_strategy_destroy(void *inst)
{
	struct even_strategy_info_t *info = (struct even_strategy_info_t *)inst;
	free(info->done);
	free(info->groups);
	free(info->device_rows);
	free(info->previous_rows);
	free(info);
}

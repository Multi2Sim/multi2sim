#include <stdlib.h>
#include "numa-partition-strategy.h"
#include "partition-util.h"

struct numa_strategy_info_t
{
	struct partition_info_t *part;
	unsigned int part_dim;
	unsigned int num_parts_assigned;
};

void *numa_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups)
{
	struct numa_strategy_info_t *info = (struct numa_strategy_info_t *)calloc(1, sizeof *info);
	info->part = partition_info_create(num_devices, dims, groups);
	info->part_dim = pick_partition_dimension(dims, groups);
	return info;
}

int numa_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count)
{
	struct numa_strategy_info_t *info = (struct numa_strategy_info_t *)inst;
	unsigned int num_parts_left = info->part->groups[info->part_dim] - info->num_parts_assigned;
	unsigned int size = round_up_not_more(num_parts_left, 2 * info->part->num_devices, num_parts_left);

	if (size != 0)
	{
		populate_partition_info(info->num_parts_assigned, size, info->part_dim, info->part, group_offset, group_count);
		info->num_parts_assigned += size;
		return 1;
	}
	else
		return 0;
}

void numa_strategy_destroy(void *inst)
{
	struct numa_strategy_info_t *info = (struct numa_strategy_info_t *)inst;
	partition_info_free(info->part);
	free(info);
}

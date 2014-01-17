#include <stdlib.h>
#include "first-done-partition-strategy.h"
#include "partition-util.h"

static const char *M2S_OPENCL_FIRST_DONE_TRIAL = "M2S_OPENCL_FIRST_DONE_TRIAL";

struct first_done_strategy_info_t
{
	struct partition_info_t *part;
	unsigned int part_dim;
	unsigned int trial_num;
	unsigned int trial_den;
	unsigned int num_parts_assigned;
	int *dev_done;
	int done;
};

void *first_done_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups)
{
	struct first_done_strategy_info_t *info = (struct first_done_strategy_info_t *)calloc(1, sizeof *info);
	info->part = partition_info_create(num_devices, dims, groups);
	info->dev_done = (int *)calloc(num_devices, sizeof (int));
	info->part_dim = pick_partition_dimension(dims, groups);
	
	const char *trial_str = getenv(M2S_OPENCL_FIRST_DONE_TRIAL);
	if (trial_str == NULL)
	{
		info->trial_num = 3;
		info->trial_den = 16;
	}
	else
	{
		unsigned int trial_frac[2];
		proportions_from_string(trial_str, 2, trial_frac);
		info->trial_num = trial_frac[0];
		info->trial_den = trial_frac[1];
	}

	return info;
}


int first_done_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count, long long now)
{
	struct first_done_strategy_info_t *info = (struct first_done_strategy_info_t *)inst;
	unsigned int total_parts = info->part->groups[info->part_dim];
	if (info->dev_done[id])
	{
		if (info->done)
			return 0;
		else
		{
			unsigned int size = total_parts - info->num_parts_assigned;
			populate_partition_info(info->num_parts_assigned, size, info->part_dim, info->part, group_offset, group_count);
			info->done = 1;
			info->num_parts_assigned = total_parts; /* in case fater device complets twice before other devices are scheduled */
			return 1;
		}
	}
	else
	{
		unsigned int size = info->trial_num * total_parts / info->trial_den;
		unsigned int left = total_parts - info->num_parts_assigned;
		if (size == 0)
			size = 1;

		if (size > left)
			size = left; /* could be zero again. that is ok. */
		populate_partition_info(info->num_parts_assigned, size, info->part_dim, info->part, group_offset, group_count);
		info->num_parts_assigned += size;
		info->dev_done[id] = 1;
		return 1;
	}
}


void first_done_strategy_destroy(void *inst)
{
	struct first_done_strategy_info_t *info = (struct first_done_strategy_info_t *)inst;
	partition_info_free(info->part);
	free(info->dev_done);
	free(info);
}

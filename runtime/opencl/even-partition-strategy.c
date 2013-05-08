#include "mhandle.h"
#include "even-partition-strategy.h"

struct even_strategy_info_t
{
	int num_devices;
	unsigned int dims;
	unsigned int *groups;
	int *done;
};

void *even_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups)
{
	struct even_strategy_info_t *info = xcalloc(1, sizeof (struct even_strategy_info_t));
	info->num_devices = num_devices;
	info->dims = dims;
	info->groups = xcalloc(dims, sizeof (unsigned int));
	memcpy(info->groups, groups, dims * sizeof (unsigned int));
	info->done = xcalloc(num_devices, sizeof (int)); /* initialized to zero */
	return info;
}


int even_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count)
{
	struct even_strategy_info_t *info = inst;
	if (!info->done[id])
	{
		/* partition with highest dimension */
		unsigned int max_dim_groups = info->groups[info->dims - 1];
		unsigned int start = id * max_dim_groups / info->num_devices;
		unsigned int size = (id + 1) * max_dim_groups / info->num_devices - start;
		int i;

		for (i = 0; i < info->dims - 1; i++)
		{
			group_offset[i] = 0;
			group_count[i] = info->groups[i];
		}

		group_offset[info->dims - 1] = start;
		group_count[info->dims - 1] = size;
		info->done[id] = 1;
		return 1;
	}
	else
		return 0;
}


void even_strategy_destroy(void *inst)
{
	struct even_strategy_info_t *info = inst;
	free(info->done);
	free(info->groups);
	free(info);
}

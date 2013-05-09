#include "mhandle.h"
#include "even-partition-strategy.h"

int even_strategy_parts_per_device = 1;

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
	if (info->done[id] < even_strategy_parts_per_device)
	{
		/* partition with highest dimension */
		unsigned int max_dim_groups = info->groups[info->dims - 1];
		unsigned int divisions = even_strategy_parts_per_device * info->num_devices;

		unsigned int chunk = id * even_strategy_parts_per_device + info->done[id];
		unsigned int start = chunk * max_dim_groups / divisions;
		unsigned int size = (chunk + 1) * max_dim_groups / divisions - start;
		int i;

		for (i = 0; i < info->dims - 1; i++)
		{
			group_offset[i] = 0;
			group_count[i] = info->groups[i];
		}

		group_offset[info->dims - 1] = start;
		group_count[info->dims - 1] = size;
		info->done[id]++;
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

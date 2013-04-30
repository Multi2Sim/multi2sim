#include <stdlib.h>
#include "partition-strategy.h"
#include "mhandle.h"

/* forward declartion for default strategy.  Put more declarations here */
void *default_strategy_create(int num_devices, unsigned int dims, unsigned int *groups);
int default_strategy_get_partition(void *inst, int desired_groups, unsigned int *group_offset, unsigned int *group_count);
void default_strategy_destroy(void *inst);

static struct opencl_partition_strategy strats[] = {{default_strategy_create, default_strategy_get_partition, default_strategy_destroy}};

struct opencl_partition_strategy *get_strategy()
{
	char *value = getenv("M2S_OPENCL_PARTITION_STRATEGY_ID");
	if (!value)
		return strats; // first "default" strategy.
	else
		return strats + atoi(value);
}

// default strategy.  Schedule everything onto device 0.

struct default_strategy_info_t
{
	int num_devices;
	unsigned int dims;
	unsigned int *groups;
	int done;
};

void *default_strategy_create(int num_devices, unsigned int dims, unsigned int *groups)
{
	struct default_strategy_info_t *info = xcalloc(1, sizeof (struct default_strategy_info_t));
	info->num_devices = num_devices;
	info->dims = dims;
	info->groups = xcalloc(dims, sizeof (unsigned int));
	memcpy(info->groups, groups, sizeof (unsigned int) * dims);
	info->done = 0;
	return info;
}


int default_strategy_get_partition(void *inst, int desired_groups, unsigned int *group_offset, unsigned int *group_count)
{
	int i;
	struct default_strategy_info_t *info = inst;

	if (info->done)
		return 0;
	else
	{
		for (i = 0; i < info->dims; i++)
		{
			group_offset[i] = 0;
			group_count[i] = info->groups[i];
		}
		info->done = 1;
		return 1;
	}
}


void default_strategy_destroy(void *inst)
{
	struct default_strategy_info_t *info = inst;
	free(info->groups);
	free(info);
}

#include <stdlib.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "opencl.h"
#include "partition-strategy.h"

/* include strategies here */
#include "even-partition-strategy.h"
#include "relative-runtime-partition-strategy.h"

/* forward declartion for default strategy. */
void *default_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups);
int default_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count);
void default_strategy_destroy(void *inst);

static struct opencl_partition_strategy strats[] = {
	{default_strategy_create, default_strategy_get_partition, default_strategy_destroy},
	{even_strategy_create, even_strategy_get_partition, even_strategy_destroy},
	{relative_runtime_strategy_create, relative_runtime_strategy_get_partition, relative_runtime_strategy_destroy}};

const struct opencl_partition_strategy *get_strategy()
{
	char *value = getenv("M2S_OPENCL_PARTITION_STRATEGY_ID");
	if (!value)
	{
		opencl_debug("[%s] using default strategy", __FUNCTION__);
		return strats; // first "default" strategy.
	}
	else
	{
		int idx = atoi(value);
		assert(idx >= 0 && idx < sizeof strats / sizeof strats[0]);
		opencl_debug("[%s] using strategy %d", __FUNCTION__, idx);
		return strats + idx;
	}
}

// default strategy.  Schedule everything onto device 0.

struct default_strategy_info_t
{
	int num_devices;
	unsigned int dims;
	unsigned int *groups;
	int done;
};

void *default_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups)
{
	struct default_strategy_info_t *info = (struct default_strategy_info_t *)calloc(1, sizeof (struct default_strategy_info_t));
	info->num_devices = num_devices;
	info->dims = dims;
	info->groups = (unsigned int *)calloc(dims, sizeof (unsigned int));
	memcpy(info->groups, groups, sizeof (unsigned int) * dims);
	info->done = 0;
	return info;
}


int default_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count)
{
	int i;
	struct default_strategy_info_t *info = (struct default_strategy_info_t *)inst;

	if (info->done || id)
		return 0;
	else
	{
		for (i = 0; i < (int)info->dims; i++)
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

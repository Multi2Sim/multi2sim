#include <stdlib.h>
#include "block-by-block-partition-strategy.h"
#include "partition-util.h"

const char *M2S_BLOCK_PARTITION_DIVISIONS = "M2S_BLOCK_PARTITION_DIVISIONS";

struct block_by_block_info_t
{
	struct partition_info_t *part;
	int divisions;
	unsigned int part_dim;
	unsigned int parts_assigned;
};


void *block_by_block_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups)
{
	const char *div_str;
	struct block_by_block_info_t *info = (struct block_by_block_info_t *)calloc(1, sizeof *info);
	info->part = partition_info_create(num_devices, dims, groups);
	div_str = getenv(M2S_BLOCK_PARTITION_DIVISIONS);

	if (div_str == NULL)
		info->divisions = 16;
	else
		info->divisions = atoi(div_str);



	info->part_dim = pick_partition_dimension(dims, groups);

	if (info->divisions > info->part->groups[info->part_dim])
		info->divisions = info->part->groups[info->part_dim];
	
	return info;
}


int block_by_block_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count, long long now)
{
	struct block_by_block_info_t *info = (struct block_by_block_info_t *)inst;
	if (info->parts_assigned < info->divisions)
	{
		unsigned int num_parts = info->part->groups[info->part_dim];
		unsigned int start = info->parts_assigned * num_parts / info->divisions;
		unsigned int end = (info->parts_assigned + 1) * num_parts / info->divisions;
		unsigned int size = end - start;
		populate_partition_info(start, size, info->part_dim, info->part, group_offset, group_count);
		info->parts_assigned++;
		return 1;
	}
	else
		return 0;
}


void block_by_block_strategy_destroy(void *inst)
{
	struct block_by_block_info_t *info = (struct block_by_block_info_t *)inst;
	partition_info_free(info->part);
	free(info);
}

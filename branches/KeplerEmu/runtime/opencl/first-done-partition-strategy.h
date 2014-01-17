#ifndef __FIRST_DONE_PARTITION_STRATEGY_H__
#define __FIRST_DONE_PARTITION_STRATEGY_H__

void *first_done_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups);
int first_done_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count, long long now);
void first_done_strategy_destroy(void *inst);

#endif

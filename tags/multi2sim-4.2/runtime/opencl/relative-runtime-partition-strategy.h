#ifndef __RELATIVE_RUNTIME_PARTITION_STRATEGY_H__
#define __RELATIVE_RUNTIME_PARTITION_STRATEGY_H__

void *relative_runtime_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups);
int relative_runtime_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count);
void relative_runtime_strategy_destroy(void *inst);

#endif

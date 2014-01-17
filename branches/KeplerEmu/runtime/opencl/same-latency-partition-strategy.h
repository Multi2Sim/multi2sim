#ifndef __SAME_LATENCY_PARTITION_STRATEGY_H__
#define __SAME_LATENCY_PARTITION_STRATEGY_H__

void *same_latency_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups);
int same_latency_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count, long long now);
void same_latency_strategy_destroy(void *inst);

#endif

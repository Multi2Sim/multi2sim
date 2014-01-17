#ifndef __THROUGHPUT_OVERHEAD_PARTITION_STRATEGY_H__
#define __THROUGHPUT_OVERHEAD_PARTIITON_STRATEGY_H__

void *throughput_overhead_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups);
int throughput_overhead_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count, long long now);
void throughput_overhead_strategy_destroy(void *inst);

#endif

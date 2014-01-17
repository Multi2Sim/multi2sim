#ifndef __NUMA_PARTITION_STRATEGY_H__
#define __NUMA_PARTITION_STRATEGY_H__

/* Algorithm from 'Locality and Loop Scheduling on NUMA Multiprocessors' Li H, et al. 1993 */

void *numa_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups);
int numa_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count, long long now);
void numa_strategy_destroy(void *inst);

#endif

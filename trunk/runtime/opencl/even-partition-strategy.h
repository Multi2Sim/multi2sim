#ifndef __EVEN_STRATEGY_H__
#define __EVEN_STRATEGY_H__

void *even_strategy_create(int num_devices, unsigned int dims, const unsigned int *groups);
int even_strategy_get_partition(void *inst, int id, int desired_groups, unsigned int *group_offset, unsigned int *group_count);
void even_strategy_destroy(void *inst);

#endif


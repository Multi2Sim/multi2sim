#ifndef __PARTITION_ISSUE_LOG_H__
#define __PARTITION_ISSUE_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#define PARTITION_ISSUE_MAX_DIMS 3
#define PARTITION_NUM_DEVICES 2

struct partition_issue_log_entry_t
{
	unsigned int executed;
	unsigned int group_offset[PARTITION_ISSUE_MAX_DIMS];
	unsigned int group_size[PARTITION_ISSUE_MAX_DIMS];
};

struct partition_issue_device_entry_t
{
	unsigned int num_invocations;
	unsigned int num_groups;
};


struct partition_issue_kernel_entry_t
{
	unsigned int num_dims;
	unsigned int groups[PARTITION_ISSUE_MAX_DIMS];
	struct partition_issue_log_entry_t *first_part;
	struct partition_issue_device_entry_t devices[PARTITION_NUM_DEVICES];
	long long kernel_runtime;
};

struct partition_issue_log_t
{
	struct partition_issue_log_entry_t *parts;
	struct partition_issue_kernel_entry_t *kernels;
	struct partition_issue_device_entry_t devices[PARTITION_NUM_DEVICES];
	
	int num_entries;
	int num_kernels;
	volatile int lock;
};

void partition_issue_log_init(struct partition_issue_log_t *log);
void partition_issue_log_destroy(struct partition_issue_log_t *log);

void partition_issue_log_kernel(
	struct partition_issue_log_t *log, 
	unsigned int num_dims, 
	const unsigned int *groups,
	long long now);

void partition_issue_log_record(
	struct partition_issue_log_t *log,
	int device, 
	const unsigned int *group_offset,
	const unsigned int *group_size);
	
void partition_issue_log_done_kernel(struct partition_issue_log_t *log, long long now);

void partition_issue_log_write(struct partition_issue_log_t *log, FILE *file);

void partition_issue_log_verify(struct partition_issue_log_t *log, FILE *show_errors);

#ifdef __cplusplus
}
#endif

#endif

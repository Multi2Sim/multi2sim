#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "partition-issue-log.h"

#define MAX_ENTRIES (1024 * 1024)

void partition_issue_log_init(struct partition_issue_log_t *log)
{
	log->parts = (struct partition_issue_log_entry_t *)calloc(MAX_ENTRIES + 1, sizeof (struct partition_issue_log_entry_t));
	log->kernels = (struct partition_issue_kernel_entry_t *)calloc(MAX_ENTRIES, sizeof (struct partition_issue_kernel_entry_t));
	log->lock = 0;
	log->num_entries = 0;
	log->num_kernels = 0;
}

void partition_issue_log_destroy(struct partition_issue_log_t *log)
{
	free(log->parts);
	free(log->kernels);
}

int partition_issue_still_room(struct partition_issue_log_t *log)
{
	return log->num_kernels < MAX_ENTRIES && log->num_entries < MAX_ENTRIES;
}

void partition_issue_log_kernel(
	struct partition_issue_log_t *log, 
	unsigned int num_dims, 
	const unsigned int *groups,
	long long now)
{
	while (__sync_lock_test_and_set(&log->lock, 1));
	if (partition_issue_still_room(log))
	{
		log->kernels[log->num_kernels].num_dims = num_dims;
		log->kernels[log->num_kernels].first_part = log->parts + log->num_entries;
		log->kernels[log->num_kernels].kernel_runtime = -now;
		memcpy(log->kernels[log->num_kernels].groups, groups, sizeof (unsigned int) * num_dims);
		log->num_kernels++;
	}
	__sync_lock_release(&log->lock);
}

void partition_issue_log_record(
	struct partition_issue_log_t *log,
	int device,
	const unsigned int *group_offset,
	const unsigned int *group_size)
{

	while (__sync_lock_test_and_set(&log->lock, 1));
	if (partition_issue_still_room(log))
	{
		struct partition_issue_log_entry_t *entry = log->parts + log->num_entries;
		unsigned int num_dims;
		unsigned int total_groups = 1;
		unsigned int i;
		struct partition_issue_kernel_entry_t *kernel = log->kernels + (log->num_kernels - 1);

		entry->executed = 1;
		num_dims = kernel->num_dims;

		
		for (i = 0; i < num_dims; i++)
			total_groups *= group_size[i];
		log->devices[device].num_invocations++;
		log->devices[device].num_groups += total_groups;
		
		kernel->devices[device].num_invocations++;
		kernel->devices[device].num_groups += total_groups;
		
		memcpy(entry->group_offset, group_offset, sizeof (unsigned int) * num_dims);
		memcpy(entry->group_size, group_size, sizeof (unsigned int) * num_dims);
		log->num_entries++;
	}
	__sync_lock_release(&log->lock);	
}

void partition_issue_log_done_kernel(struct partition_issue_log_t *log, long long now)
{
	log->kernels[log->num_kernels - 1].kernel_runtime += now;
	__sync_fetch_and_add(&log->num_entries, 1);	
}

void partition_issue_log_print_vector(unsigned int dims, unsigned int *vec, FILE *f)
{
	unsigned int i;
	fprintf(f, "[");
	for (i = 0; i < dims; i++)
		fprintf(f, " %d", vec[i]);
	fprintf(f, " ]");
}

void partition_issue_log_write(struct partition_issue_log_t *log, FILE *file)
{
	int i;
	int j;
	fprintf(file, "PARTITION_LOG: devices %d\n", PARTITION_NUM_DEVICES);
	fprintf(file, "PARTITION_LOG: kernels %d\n", log->num_kernels);
	fprintf(file, "PARTITION_LOG: partitions %d\n", log->num_entries - log->num_kernels);
	for (i = 0; i < PARTITION_NUM_DEVICES; i++)
	{
		fprintf(file, "PARTITION_LOG: invocations%d %d\n", i, log->devices[i].num_invocations);
		fprintf(file, "PARTITION_LOG: groups%d %d\n", i, log->devices[i].num_groups);
	}
	for (i = 0; i < log->num_kernels; i++)
	{
		fprintf(file, "PARTITION_LOG: kernel%d time %lld\n", i, log->kernels[i].kernel_runtime);
		for (j = 0; j < PARTITION_NUM_DEVICES; j++)
		{
			fprintf(file, "PARTITION_LOG: kernel%d invocations%d %d\n", i, j, log->kernels[i].devices[j].num_invocations);
			fprintf(file, "PARTITION_LOG: kernel%d groups%d %d\n", i, j, log->kernels[i].devices[j].num_groups);				
		}
	}
}


/* convert an n-dimensional address into a linear address in dimension order */
unsigned int part_linear_address(int dim, const unsigned int *pos, const unsigned int *size)
{
	unsigned int stride = 1;
	unsigned int addr = 0;
	int i;
	for (i = 0; i < dim; i++)
	{
		addr += pos[i] * stride;
		stride *= size[i];
	}
	return addr;
}

static unsigned int part_nd_address_rec(unsigned int stride, int cur, int dim, unsigned int addr, const unsigned int *size, unsigned int *pos)
{
	if (cur == dim)
		return addr;
	else
	{
		unsigned int rem = part_nd_address_rec(size[cur] * stride, cur + 1, dim, addr, size, pos);
		pos[cur] = rem / stride;
		return rem % stride;
	}
}

/* convert a linear address into an n-dimensional address */
void part_nd_address(int dim, unsigned int addr, const unsigned int *size, unsigned int *pos)
{
	part_nd_address_rec(1, 0, dim, addr, size, pos);
}

void partition_issue_log_verify(struct partition_issue_log_t *log, FILE *show_errors)
{
	unsigned int i;
	for (i = 0; i < log->num_kernels; i++)
	{
		struct partition_issue_kernel_entry_t *kernel = log->kernels + i;
		unsigned int num_groups = 1;
		unsigned int j;
		char *wg;
		int aborted = 0;
		
		for (j = 0; j < kernel->num_dims; j++)
			num_groups *= kernel->groups[j];
		
		wg = (char *)calloc(1, num_groups);	/* allocate array for work-groups */

		struct partition_issue_log_entry_t *part = kernel->first_part;
		
		while (part->executed)
		{
			unsigned int part_size = 1;
			unsigned int cur_wg[PARTITION_ISSUE_MAX_DIMS];
			
			for (j = 0; j < kernel->num_dims; j++)
				part_size *= part->group_size[j];
				
			/* go through each of these groups */
			for (j = 0; j < part_size; j++)
			{
				unsigned int k;
				unsigned int id;
				part_nd_address(kernel->num_dims, j, part->group_size, cur_wg); /* get n-dimensional position of group within part */
				for (k = 0; k < kernel->num_dims; k++)
					cur_wg[k] += part->group_offset[k]; /* add the offset */
					
				id = part_linear_address(kernel->num_dims, cur_wg, kernel->groups); /* get linear position of group in NDRange */
				wg[id]++;
			}

			part++;
		}
		
		for (j = 0; j < num_groups; j++)
			if (wg[j] != 1)
			{
				fprintf(stderr, "Kernel %d, work_group %d executed %d times\n", i, j, wg[j]);
				aborted = 1;
			}
			
		if (aborted)
		{
			struct partition_issue_log_entry_t *part = kernel->first_part;
			fprintf(stderr, "Kernel partitioning information: %d\n", i);
			fprintf(stderr, "NDRange: ");
			partition_issue_log_print_vector(kernel->num_dims, kernel->groups, stderr);
			fprintf(stderr, "\n");
			while (part->executed)
			{
				fprintf(stderr, "At ");
				partition_issue_log_print_vector(kernel->num_dims, part->group_offset, stderr);
				fprintf(stderr, " Size ");
				partition_issue_log_print_vector(kernel->num_dims, part->group_size, stderr);
				fprintf(stderr, "\n");
				part++;
			}
		
			abort();
		}
		
		free(wg);
	}
}


#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "partition-issue-log.h"

#define MAX_ENTRIES (1024 * 1024)

void partition_issue_log_init(struct partition_issue_log_t *log)
{
	log->parts = (struct partition_issue_log_entry_t *)calloc(MAX_ENTRIES, sizeof (struct partition_issue_log_entry_t));
	log->kernels = (struct partition_issue_kernel_entry_t *)calloc(MAX_ENTRIES, sizeof (struct partition_issue_kernel_entry_t));
	log->lock = 0;
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
	const unsigned int *groups)
{
	while (__sync_lock_test_and_set(&log->lock, 1));
	if (partition_issue_still_room(log))
	{
		log->kernels[log->num_kernels].num_dims = num_dims;
		memcpy(log->kernels[log->num_kernels].groups, groups, sizeof (unsigned int) * num_dims);
		log->num_kernels++;
	}
	__sync_lock_release(&log->lock);
}

void partition_issue_log_record(
	struct partition_issue_log_t *log,
	int device,
	long long start, 
	const unsigned int *group_offset,
	const unsigned int *group_size)
{
	while (__sync_lock_test_and_set(&log->lock, 1));
	if (partition_issue_still_room(log))
	{
		struct partition_issue_log_entry_t *entry = log->parts + log->num_entries;
		unsigned int num_dims;

		entry->device = device;
		entry->start = start;
		entry->kernel_number = log->num_kernels - 1;

		num_dims = log->kernels[entry->kernel_number].num_dims;
		memcpy(entry->group_offset, group_offset, sizeof (unsigned int) * num_dims);
		memcpy(entry->group_size, group_size, sizeof (unsigned int) * num_dims);
		log->num_entries++;
	}
	__sync_lock_release(&log->lock);
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
	unsigned int i;
	struct partition_issue_log_entry_t *entry = log->parts;
	for (i = 0; i < log->num_kernels; i++)
	{
		struct partition_issue_kernel_entry_t *kernel = log->kernels + i;
		fprintf(file, "Kernel %d ", i);
		
		partition_issue_log_print_vector(kernel->num_dims, kernel->groups, file);
		fprintf(file, "\n");

		long long start = entry->start;
		while (entry - log->parts < log->num_entries && entry->kernel_number == i)
		{
			fprintf(file, "\tDevice %d Time: %10lld offset: ", entry->device, entry->start - start);
			partition_issue_log_print_vector(kernel->num_dims, entry->group_offset, file);
			fprintf(file, " size: ");
			partition_issue_log_print_vector(kernel->num_dims, entry->group_size, file);
			fprintf(file, "\n");
			entry++;
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

int part_is_in_range(int dims, const unsigned int *point, const unsigned int *start, const unsigned int *size)
{
	int i;
	for (i = 0; i < dims; i++)
		if (point[i] < start[i] || point[i] >= start[i] + size[i])
			return 0;
	return 1;
}

void partition_issue_log_verify(struct partition_issue_log_t *log, FILE *show_errors)
{
	unsigned int i, j;
	struct partition_issue_log_entry_t *entry = log->parts;
	for (i = 0; i < log->num_kernels; i++)
	{
		struct partition_issue_kernel_entry_t *kernel = log->kernels + i;
		unsigned int num_wg = 1;
		char *wg;

		for (j = 0; j < kernel->num_dims; j++)
			num_wg *= kernel->groups[j];

		wg = (char *)calloc(1, num_wg);		

		while (entry - log->parts < log->num_entries && entry->kernel_number == i)
		{
			unsigned int *offset = entry->group_offset;
			unsigned int *size = entry->group_size;

			for (j = 0; j < num_wg; j++)
			{
				unsigned int cur[PARTITION_ISSUE_MAX_DIMS];
				part_nd_address(kernel->num_dims, j, kernel->groups, cur);
				
				if (part_is_in_range(kernel->num_dims, cur, offset, size))
					wg[j] = 1;
			}
			entry++;
		}
		for (j = 0; j < num_wg; j++)
			assert(wg[j] != 0);
		
		free(wg);
	}
	
}


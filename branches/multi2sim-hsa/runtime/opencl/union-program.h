#ifndef __UNION_PROGRAM_H__
#define __UNION_PROGRAM_H__

#include "opencl.h"
#include "list.h"
#include "union-device.h"

struct opencl_union_program_t
{
	enum opencl_runtime_type_t type;  /* First field */

	struct opencl_program_t *parent;
	struct opencl_union_device_t *device;
	struct list_t *programs;
};

/* use this format for 'multiple binaries' */
struct opencl_union_binary_t
{
	enum opencl_runtime_type_t type;  /* First field */

	unsigned int num_entries;
	unsigned int entry_sizes[0];	
};

struct opencl_union_program_t *opencl_union_program_create(
	struct opencl_program_t *parent,
	struct opencl_union_device_t *device,
	void *binary,
	unsigned int length);

void opencl_union_program_free(struct opencl_union_program_t *program);

int opencl_union_program_valid_binary(void *dev, void *binary, unsigned int length);

#endif

#include <assert.h>
#include "union-program.h"
#include "mhandle.h"
#include "program.h"
#include "union-device.h"

struct opencl_union_program_t *opencl_union_program_create(
	struct opencl_program_t *parent,
	struct opencl_union_device_t *device,
	void *binary,
	unsigned int length)
{
	int total;
	int i, j;
	struct opencl_union_program_t *program;
	struct opencl_union_binary_t *bin;
	void **binaries;
	struct opencl_device_t *subdevice;

	bin = binary;
	assert(bin->num_entries >= list_count(device->devices));

	program = xcalloc(1, sizeof (struct opencl_union_program_t));
	program->parent = parent;
	program->programs = list_create();
	program->device = device;

	total = sizeof *bin + bin->num_entries * sizeof bin->entry_sizes[0];
	binaries = xcalloc(bin->num_entries, sizeof (void *));

	/* go through all the binaries an assign them to a two-dimensional array for easy access */
	for (i = 0; i < bin->num_entries; i++)
	{
		binaries[i] = (char *)binary + total;
		total += bin->entry_sizes[i];
	}
	assert(total == length);
	/* find the binary that corresponds to each device in the union */
	LIST_FOR_EACH(device->devices, i)
	{
		int match = 0;
		subdevice = list_get(device->devices, i);
		for (j = 0; j < bin->num_entries; j++)
		{
			/* if it's a match, create a subdevice-specific program object */
			if (subdevice->arch_program_valid_binary_func(subdevice->arch_device, binaries[j], bin->entry_sizes[j]))
			{
				list_add(
					program->programs, 
					subdevice->arch_program_create_func(
						NULL, 
						subdevice->arch_device, 
						binaries[j], 
						bin->entry_sizes[j]));
				match = 1;
			}
		}
		assert(match == 1);
	}

	free(binaries);
	return program;
}


void opencl_union_program_free(struct opencl_union_program_t *program)
{
	int i;
	assert(list_count(program->device->devices) == list_count(program->programs));
	LIST_FOR_EACH(program->device->devices, i)
	{
		struct opencl_device_t *subdevice = list_get(program->device->devices, i);
		subdevice->arch_program_free_func(list_get(program->programs, i));
	}

	list_free(program->programs);
	free(program);
}


int opencl_union_program_valid_binary(void *dev, void *binary, unsigned int length)
{
	int i;
	struct opencl_union_binary_t *bin;
	struct opencl_union_device_t *device = dev;

	bin = (struct opencl_union_binary_t *)binary;

	/* get each subdevice and check to see if it has a binary */
	LIST_FOR_EACH(device->devices, i)
	{
		int j;
		struct opencl_device_t *subdevice;
		subdevice = list_get(device->devices, i);

		int valid = 0;
		int offset = sizeof *bin + bin->num_entries * sizeof bin->entry_sizes[0];
		/* go through binaries. */
		for (j = 0; j < bin->num_entries; j++)
		{
			if (subdevice->arch_program_valid_binary_func(subdevice->arch_device, (char *)binary + offset, bin->entry_sizes[j]))
			{
				valid = 1;
				break;
			}

			offset += bin->entry_sizes[j];
		}

		/* make sure that a device acepts the binary */
		if (!valid)
			return 0;
	}

	return 1;
}

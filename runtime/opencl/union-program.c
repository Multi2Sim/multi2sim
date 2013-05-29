#include <assert.h>
#include "union-program.h"
#include "mhandle.h"
#include "program.h"
#include "union-device.h"
#include "elf-format.h"
#include "elf.h"

struct opencl_union_program_t *opencl_union_program_create(
	struct opencl_program_t *parent, struct opencl_union_device_t *device,
	void *binary, unsigned int length)
{
	int i, j;
	struct opencl_union_program_t *program;
	struct opencl_device_t *subdevice;
	struct elf_file_t *elf_file;

	program = xcalloc(1, sizeof (struct opencl_union_program_t));
	program->type = opencl_runtime_type_union;
	program->parent = parent;
	program->programs = list_create();
	program->device = device;

	elf_file = elf_file_create_from_buffer(binary, length, 
		"Fused OpenCL Binary");

	/* find the binary that corresponds to each device in the union */
	LIST_FOR_EACH(device->devices, i)
	{
		int match = 0;
		subdevice = list_get(device->devices, i);
		opencl_debug("[%s] device %d = %p", __FUNCTION__, i, subdevice);

		/* go through all the binaries.  When one matches, use it to 
		 * create a program for the current device */
		LIST_FOR_EACH(elf_file->section_list, j)
		{
			struct elf_section_t *section = 
				list_get(elf_file->section_list, j);
			if (!strcmp(section->name, ".text"))
			{
				if (subdevice->arch_program_valid_binary_func(
					subdevice->arch_device, 
					section->buffer.ptr, 
					section->buffer.size))
				{
					list_add(program->programs, 
						subdevice->arch_program_create_func(
							NULL, 
							subdevice->arch_device, 
							section->buffer.ptr,
							section->buffer.size));
					match = 1;
				}
			}
		}
		assert(match == 1);
	}
	elf_file_free(elf_file);
	return program;
}


void opencl_union_program_free(struct opencl_union_program_t *program)
{
	int i;
	assert(list_count(program->device->devices) == 
		list_count(program->programs));
	LIST_FOR_EACH(program->device->devices, i)
	{
		struct opencl_device_t *subdevice = 
			list_get(program->device->devices, i);
		subdevice->arch_program_free_func(
			list_get(program->programs, i));
	}

	list_free(program->programs);
	free(program);
}


int opencl_union_program_valid_binary(void *dev, void *binary, 
	unsigned int length)
{
	int i;
	struct opencl_union_device_t *device = dev;
	Elf32_Ehdr *h = (Elf32_Ehdr *) binary;
	struct elf_file_t *elf_file;

	if (h->e_ident[EI_MAG0] != ELFMAG0 || 
		h->e_ident[EI_MAG1] != ELFMAG1 || 
		h->e_ident[EI_MAG2] != ELFMAG2 || 
		h->e_ident[EI_MAG3] != ELFMAG3 ||
		h->e_machine != EM_NONE)
	{
		return 0;
	}
	elf_file = elf_file_create_from_buffer(binary, length, 
		"Fused OpenCL Binary");

	/* get each subdevice and check to see if it has a binary */
	LIST_FOR_EACH(device->devices, i)
	{
		int j;
		struct opencl_device_t *subdevice;
		subdevice = list_get(device->devices, i);

		int valid = 0;

		/* go through binaries. */
		LIST_FOR_EACH(elf_file->section_list, j)
		{
			struct elf_section_t *section = 
				list_get(elf_file->section_list, j);	
			if (subdevice->arch_program_valid_binary_func(
				subdevice->arch_device, section->buffer.ptr, 
				section->buffer.size))
			{
				valid = 1;
				break;
			}

		}

		/* make sure that there's a binary for every device */
		if (!valid)
			return 0;
	}
	elf_file_free(elf_file);

	return 1;
}

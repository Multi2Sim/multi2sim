/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This module is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This module is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this module; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef DRIVER_CUDA_MODULE_H
#define DRIVER_CUDA_MODULE_H

extern struct list_t *module_list;

struct cuda_module_t
{
	unsigned int id;
	int ref_count;

	/* Kernel binary */
	struct elf_file_t *elf_file;
};

struct cuda_module_t *cuda_module_create(char *cubin_path);
void cuda_module_free(struct cuda_module_t *module);

#endif


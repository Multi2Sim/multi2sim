/*
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <lib/mhandle/mhandle.h>
#include <lib/util/hash-table.h>

#include "arch.h"
#include "arch-list.h"


static struct hash_table_t *arch_list;


void arch_list_init(void)
{
	/* Create hash table with registered architectures */
	arch_list = hash_table_create(0, 0);
}


void arch_list_done(void)
{
	hash_table_free(arch_list);
}


void arch_list_dump(FILE *f)
{
	struct arch_t *arch;
	char *arch_name;

	HASH_TABLE_FOR_EACH(arch_list, arch_name, arch)
		arch_dump(arch, f);
}


void arch_list_register(char *arch_name)
{
}


void arch_list_set_sim_kind(char *arch_name, enum arch_sim_kind_t *sim_kind_ptr)
{
}

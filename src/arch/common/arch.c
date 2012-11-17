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
#include <lib/util/linked-list.h>
#include <lib/util/string.h>

#include "arch.h"


struct str_map_t arch_sim_kind_map =
{
	2,
	{
		{ "Functional", arch_sim_kind_functional },
		{ "Detailed", arch_sim_kind_detailed }
	}
};


struct arch_t *arch_create(char *name, char *prefix)
{
	struct arch_t *arch;

	/* Initialize */
	arch = xcalloc(1, sizeof(struct arch_t));
	arch->name = xstrdup(name);
	arch->prefix = xstrdup(prefix);
	arch->mem_entry_mod_list = linked_list_create();

	/* Return */
	return arch;
}


void arch_free(struct arch_t *arch)
{
	free(arch->name);
	free(arch->prefix);
	linked_list_free(arch->mem_entry_mod_list);
	free(arch);
}


void arch_dump(struct arch_t *arch, FILE *f)
{
	fprintf(f, "** Architecture '%s' **\n", arch->name);
	fprintf(f, "SimKind = %s\n", str_map_value(&arch_sim_kind_map, arch->sim_kind));
	fprintf(f, "\n");
}

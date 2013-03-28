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
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "arch.h"
#include "arch-list.h"


#if 0
void arch_list_init(void)
{
	arch_list = list_create();
}


void arch_list_done(void)
{
	/* Free architectures */
	while (list_count(arch_list))
		arch_free(list_pop(arch_list));
	list_free(arch_list);
}


void arch_list_dump(FILE *f)
{
	struct arch_t *arch;
	int i;

	LIST_FOR_EACH(arch_list, i)
	{
		arch = list_get(arch_list, i);
		arch_dump(arch, f);
	}
}


#endif


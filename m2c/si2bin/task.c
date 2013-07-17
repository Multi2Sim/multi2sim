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

#include <assert.h>

#include <arch/southern-islands/asm/asm.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/elf-encode.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>

#include "outer-bin.h"
#include "si2bin.h"
#include "symbol.h"
#include "task.h"


/*
 * Task Object
 */

struct si2bin_task_t *si2bin_task_create(int offset, struct si2bin_symbol_t *symbol)
{
	struct si2bin_task_t *task;
	
	/* Allocate */
	task = xcalloc(1, sizeof(struct si2bin_task_t));
	
	/* Initialize the task's offset and ID */
	task->offset = offset;
	task->symbol = symbol;
	
	/* Return */
	return task;

}

void si2bin_task_free(struct si2bin_task_t *task)
{
	free(task);
}


void si2bin_task_dump(struct si2bin_task_t *task, FILE *f)
{
	fprintf(f, "offset=%d, symbol={", task->offset);
	si2bin_symbol_dump(task->symbol, f);
	fprintf(f, "}");
}


void si2bin_task_process(struct si2bin_task_t *task)
{
	struct si2bin_symbol_t *label;
	union si_inst_microcode_t *inst;


	/* Check whether symbol is resolved */
	label = task->symbol;
	if (!label->defined)
		si2bin_yyerror_fmt("undefined label: %s", label->name);

	/* Resolve label */
	assert(IN_RANGE(task->offset, 0, si2bin_entry->text_section_buffer->offset - 4));
	inst = si2bin_entry->text_section_buffer->ptr + task->offset;
	inst->sopp.simm16 = (label->value - task->offset) / 4 - 1;
}




/*
 * Global Functions
 */

struct list_t *si2bin_task_list;

		
void si2bin_task_list_init(void)
{							  
	si2bin_task_list = list_create();
}


void si2bin_task_list_done(void)
{
	int index;
	
	LIST_FOR_EACH(si2bin_task_list, index)
		si2bin_task_free(list_get(si2bin_task_list, index));
	list_free(si2bin_task_list);
}


void si_task_list_dump(FILE *f)
{
	int index;
	
	fprintf(f, "Task list:\n");
	LIST_FOR_EACH(si2bin_task_list, index)
	{
		fprintf(f, "\ttask %d: ", index);
		si2bin_task_dump(list_get(si2bin_task_list, index), f);
		fprintf(f, "\n");
	}
}


void si2bin_task_list_process(void)
{
	int index;

	LIST_FOR_EACH(si2bin_task_list, index)
		si2bin_task_process(list_get(si2bin_task_list, index));
}

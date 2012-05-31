/*
 *  Multi2Sim
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <repos.h>

#include <southern-islands-emu.h>
#include <mem-system.h>
#include <x86-emu.h>


/* Some globals */

struct si_ndrange_t *si_isa_ndrange;  /* Current ND-Range */
struct si_work_group_t *si_isa_work_group;  /* Current work-group */
struct si_wavefront_t *si_isa_wavefront;  /* Current wavefront */
struct si_work_item_t *si_isa_work_item;  /* Current work-item */
struct si_inst_t *si_isa_cf_inst;  /* Current CF instruction */
struct si_inst_t *si_isa_inst;  /* Current instruction */
struct si_alu_group_t *si_isa_alu_group;  /* Current ALU group */

/* Repository of deferred tasks */
struct repos_t *si_isa_write_task_repos;

/* Instruction execution table */
si_isa_inst_func_t *si_isa_inst_func;

/* Debug */
int si_isa_debug_category;




/*
 * Initialization, finalization
 */


/* Initialization */
void si_isa_init()
{
	/* Allocate instruction execution table */
	si_isa_inst_func = calloc(SI_INST_COUNT, sizeof(si_isa_inst_func_t));
	if (!si_isa_inst_func)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size) \
	extern void si_isa_##_name##_impl(); \
	si_isa_inst_func[SI_INST_##_name] = si_isa_##_name##_impl;
#include <southern-islands-asm.dat>
#undef DEFINST

	/* Repository of deferred tasks */
	si_isa_write_task_repos = repos_create(sizeof(struct si_isa_write_task_t),
		"gpu_isa_write_task_repos");
}

void si_isa_done()
{
	/* Instruction execution table */
	free(si_isa_inst_func);

	/* Repository of deferred tasks */
	repos_free(si_isa_write_task_repos);
}


/* Helper functions */

/* Read SGPR */
unsigned int si_read_sgpr(unsigned int sreg)
{
	/* FIXME */
	/* assert(sreg in range) */

	return SI_SGPR_ELEM(sreg);
}

/* Initialize a buffer resource descriptor */
void si_isa_init_buf_res(struct si_buffer_resource_t *buf_desc, unsigned int sreg)
{
	assert(buf_desc);

	unsigned int values[4];
	int i;

	for(i = 0; i < 4; i++)
	{
		values[i] = si_read_sgpr(sreg+i);
	}

	memcpy(buf_desc, values, sizeof(unsigned int)*4);
}

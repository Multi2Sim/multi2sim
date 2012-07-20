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

#include <arm-emu.h>



/*
 * Global variables
 */

/* Configuration parameters */
long long arm_emu_max_inst = 0;
long long arm_emu_max_cycles = 0;
long long arm_emu_max_time = 0;
char * arm_emu_last_inst_bytes = 0;
enum arm_emu_kind_t arm_emu_kind = arm_emu_kind_functional;

/* Arm CPU Emulator */
struct arm_emu_t *arm_emu;



/*
 * Functional simulation loop
 */

/* Run one iteration of the x86 emulation loop */
void arm_emu_run(void)
{
	struct arm_ctx_t *ctx;

	/* Stop if all contexts finished */
	/* FIXME - don't finish, just exit - what if other CPU contexts are still running? */
	if (arm_emu->finished_list_count >= arm_emu->context_list_count)
		esim_finish = esim_finish_ctx;

	/* Stop if maximum number of CPU instructions exceeded */
	if (arm_emu_max_inst && arm_emu->inst_count >= arm_emu_max_inst)
		esim_finish = esim_finish_arm_max_inst;

	/* Stop if maximum number of cycles exceeded */
	if (arm_emu_max_cycles && esim_cycle >= arm_emu_max_cycles)
		esim_finish = esim_finish_arm_max_cycles;

	/* Stop if any previous reason met */
	if (esim_finish)
		return;

	/* Run an instruction from every running process */
	for (ctx = arm_emu->running_list_head; ctx; ctx = ctx->running_list_next)
	{
		arm_ctx_execute(ctx);

		/* Stop if instruction matches last instruction bytes */
		if (arm_emu_last_inst_bytes &&
			!strncmp(arm_isa_inst_bytes,
				arm_emu_last_inst_bytes,
				strlen(arm_emu_last_inst_bytes)))
			esim_finish = esim_finish_arm_last_inst;
	}

	/* Free finished contexts */
	while (arm_emu->finished_list_head)
		arm_ctx_free(arm_emu->finished_list_head);

	/* Process list of suspended contexts */
	/*arm_emu_process_events();*/
}

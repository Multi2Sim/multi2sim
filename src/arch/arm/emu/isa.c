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

#include <mem-system.h>
#include <arm-emu.h>

/* Debug categories */
int arm_isa_call_debug_category;
int arm_isa_inst_debug_category;

/* FIXME */
char *arm_isa_inst_bytes;


/* Table including references to functions in machine.c
 * that implement machine instructions. */
/* Instruction execution table */


static arm_isa_inst_func_t arm_isa_inst_func[ARM_INST_COUNT] =
{
	NULL /* for op_none */
#define DEFINST(_name, _fmt_str, _category, _arg1, _arg2) , arm_isa_##_name##_impl
#include <arm-asm.dat>
#undef DEFINST
};




/*
 * Instruction statistics
 */

static long long arm_inst_freq[ARM_INST_COUNT];

/* Trace call debugging */
static void arm_isa_debug_call(struct arm_ctx_t *ctx)
{

	/* FIXME: Implement branch calling method for Arm */
	/*
	struct elf_symbol_t *from;
	struct elf_symbol_t *to;

	struct arm_regs_t *regs = ctx->regs;

	char *action;
	int i;

	 Do nothing on speculative mode
	if (ctx->status & arm_ctx_spec_mode)
		return;

	 Call or return. Otherwise, exit
	if (!strncmp(ctx->inst.info->fmt_str, "call", 4))
		action = "call";
	else if (!strncmp(ctx->inst.info->fmt_str, "ret", 3))
		action = "ret";
	else
		return;

	 Debug it
	for (i = 0; i < ctx->function_level; i++)
		arm_isa_call_debug("| ");
	from = elf_symbol_get_by_address(loader->elf_file, ctx->curr_eip, NULL);
	to = elf_symbol_get_by_address(loader->elf_file, regs->eip, NULL);
	if (from)
		arm_isa_call_debug("%s", from->name);
	else
		arm_isa_call_debug("0x%x", ctx->curr_eip);
	arm_isa_call_debug(" - %s to ", action);
	if (to)
		arm_isa_call_debug("%s", to->name);
	else
		arm_isa_call_debug("0x%x", regs->eip);
	arm_isa_call_debug("\n");

	 Change current level
	if (strncmp(ctx->inst.format, "call", 4))
		ctx->function_level--;
	else
		ctx->function_level++;
	*/
}




/*
 * Public Functions
 */


/* FIXME - merge with ctx_execute */
void arm_isa_execute_inst(struct arm_ctx_t *ctx)
{
	struct arm_regs_t *regs = ctx->regs;


	/* Set last, current, and target instruction addresses */
	ctx->last_ip = ctx->curr_ip;
	ctx->curr_ip = regs->ip;
	ctx->target_ip = 0;

	/* Debug */
	if (debug_status(arm_isa_inst_debug_category))
	{
		arm_isa_inst_debug("%d %8lld %x: ", ctx->pid,
			arm_emu->inst_count, ctx->curr_ip);
		arm_inst_debug_dump(&ctx->inst, debug_file(arm_isa_inst_debug_category));
		arm_isa_inst_debug("  (%d bytes)", ctx->inst.info->size);
	}

	/* Call instruction emulation function */
	regs->ip = regs->ip + ctx->inst.info->size;
	if (ctx->inst.info->opcode)
		arm_isa_inst_func[ctx->inst.info->opcode](ctx);

	/* Statistics */
	arm_inst_freq[ctx->inst.info->opcode]++;

	/* Debug */
	arm_isa_inst_debug("\n");
	if (debug_status(arm_isa_call_debug_category))
		arm_isa_debug_call(ctx);
}

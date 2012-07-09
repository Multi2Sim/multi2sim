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

static struct arm_ctx_t *arm_ctx_do_create()
{
	struct arm_ctx_t *ctx;

	/* Create context and set its value */
	ctx = calloc(1, sizeof(struct arm_ctx_t));
	if(!ctx)
		fatal("%s: out of memory", __FUNCTION__);

	/* Structures */
	ctx->regs = arm_regs_create();

	/* Return context */
	return ctx;
}

struct arm_ctx_t *arm_ctx_create()
{
	struct arm_ctx_t *ctx;

	ctx = arm_ctx_do_create();

	/* Memory */
	ctx->mem = mem_create();

	/* Initialize */
	ctx->args = linked_list_create();
	ctx->env = linked_list_create();

	return ctx;
}

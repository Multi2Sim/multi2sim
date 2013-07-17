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

#include <lib/util/debug.h>
#include <mem-system/memory.h>

#include "context.h"
#include "signal.h"
#include "regs.h"
#include "syscall.h"



/* Return from a signal handler */
void arm_signal_handler_return(struct arm_ctx_t *ctx)
{
	/* Change context status */
	if (!arm_ctx_get_status(ctx, arm_ctx_handler))
		fatal("%s: not handling a signal", __FUNCTION__);
	arm_ctx_clear_status(ctx, arm_ctx_handler);

	/* Free signal frame */
	mem_unmap(ctx->mem, ctx->signal_mask_table->pretcode, MEM_PAGE_SIZE);
	arm_sys_debug("  signal handler return code at 0x%x deallocated\n",
		ctx->signal_mask_table->pretcode);

	/* Restore saved register file and free backup */
	arm_regs_copy(ctx->regs, ctx->signal_mask_table->regs);
	arm_regs_free(ctx->signal_mask_table->regs);
}

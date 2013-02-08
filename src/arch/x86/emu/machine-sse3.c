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

#include <arch/common/arch.h>

#include "context.h"
#include "emu.h"
#include "isa.h"
#include "machine.h"
#include "regs.h"
#include "uinst.h"


/* Macros defined to prevent accidental use of 'mem_<xx>' instructions */
#define mem_access __COMPILATION_ERROR__
#define mem_read __COMPILATION_ERROR__
#define mem_write __COMPILATION_ERROR__
#define mem_zero __COMPILATION_ERROR__
#define mem_read_string __COMPILATION_ERROR__
#define mem_write_string __COMPILATION_ERROR__
#define mem_get_buffer __COMPILATION_ERROR__
#define fatal __COMPILATION_ERROR__
#define panic __COMPILATION_ERROR__
#define warning __COMPILATION_ERROR__
#ifdef assert
#undef assert
#endif
#define assert __COMPILATION_ERROR__


void x86_isa_movntdq_m128_xmm_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_palignr_xmm_xmmm128_imm8_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pcmpistri_xmm_xmmm128_imm8_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pinsrb_xmm_r32m8_imm8_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pinsrd_xmm_rm32_imm8_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_pshufb_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}


void x86_isa_ptest_xmm_xmmm128_impl(struct x86_ctx_t *ctx)
{
	x86_isa_error(ctx, "%s: not implemented", __FUNCTION__);
}



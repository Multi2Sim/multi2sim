/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (ubal@gap.upv.es)
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

#include <cpukernel.h>


void op_movaps_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_movaps_xmmm128_xmm_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_movd_xmm_rm32_impl()
{
	uint8_t xmm[16];
	uint32_t value = isa_load_rm32();

	memset(xmm, 0, 16);
	* (uint32_t *) xmm = value;

	isa_store_xmm(xmm);
}


void op_movd_rm32_xmm_impl()
{
	uint8_t xmm[16];
	uint32_t value;

	isa_load_xmm(xmm);
	value = * (uint32_t *) xmm;

	isa_store_rm32(value);
}


void op_movdqa_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_movdqa_xmmm128_xmm_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_movdqu_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_movdqu_xmmm128_xmm_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_movntdq_m128_xmm_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_movq_xmm_xmmm64_impl()
{
	uint8_t value[16];

	/* If 'movq' happens from memory to register, the 64-bit value is 0-extended to 128 bits.
	 * If 'movq' is from register to register, only the lower 64-bit of the destination register
	 * should be affected. */
	if (isa_inst.modrm == 3)
		isa_load_xmm(value);
	else
		memset(value, 0, 16);

	/* Copy lower 64-bit */
	isa_load_xmmm64(value);
	isa_store_xmm(value);
}


void op_movq_xmmm64_xmm_impl()
{
	uint8_t value[16];
	isa_load_xmm(value);
	isa_store_xmmm64(value);
}


void op_palignr_xmm_xmmm128_imm8_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_pshufd_xmm_xmmm128_imm8_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


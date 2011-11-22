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


void op_cvttsd2si_r32_xmmm64_impl()
{
	uint8_t xmm[16];
	uint32_t r32;

	isa_load_xmmm64(xmm);
	asm volatile (
		"cvttsd2si %1, %%eax\n\t"
		"mov %%eax, %0"
		: "=m" (r32)
		: "m" (*xmm)
		: "eax"
	);
	isa_store_r32(r32);
}


void op_cvttss2si_r32_xmmm32_impl()
{
	uint8_t xmm[16];
	uint32_t r32;

	isa_load_xmmm32(xmm);
	asm volatile (
		"cvttss2si %1, %%eax\n\t"
		"mov %%eax, %0"
		: "=m" (r32)
		: "m" (*xmm)
		: "eax"
	);
	isa_store_r32(r32);
}


void op_ldmxcsr_m32_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


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


void op_movhpd_xmm_m64_impl()
{
	uint64_t m64;
	uint8_t xmm[16];

	isa_load_xmm(xmm);
	m64 = isa_load_m64();
	* (uint64_t *) &xmm[8] = m64;
	isa_store_xmm(xmm);
}


void op_movhpd_m64_xmm_impl()
{
	uint64_t m64;
	uint8_t xmm[16];

	isa_load_xmm(xmm);
	m64 = * (uint64_t *) &xmm[8];
	isa_store_m64(m64);
}


void op_movlpd_xmm_m64_impl()
{
	uint64_t m64;
	uint8_t xmm[16];

	isa_load_xmm(xmm);
	m64 = isa_load_m64();
	* (uint64_t *) xmm = m64;
	isa_store_xmm(xmm);
}


void op_movlpd_m64_xmm_impl()
{
	uint64_t m64;
	uint8_t xmm[16];

	isa_load_xmm(xmm);
	m64 = * (uint64_t *) xmm;
	isa_store_m64(m64);
}


void op_pmovmskb_r32_xmmm128_impl()
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

	memset(value, 0, 16);
	isa_load_xmmm64(value);
	isa_store_xmm(value);
}


void op_movq_xmmm64_xmm_impl()
{
	uint8_t value[16];

	memset(value, 0, 16);
	if (isa_inst.modrm_mod == 3)
		memcpy(&isa_regs->xmm[isa_inst.modrm_rm], value, 16);
	isa_load_xmm(value);
	isa_store_xmmm64(value);
}


void op_movss_xmm_xmmm32_impl()
{
	uint8_t value[16];

	/* xmm <= m32: bits 127-32 of xmm set to 0.
	 * xmm <= xmm: bits 127-32 unmodified */
	if (isa_inst.modrm_mod == 3)
		isa_load_xmm(value);
	else
		memset(value, 0, 16);
	isa_load_xmmm32(value);
	isa_store_xmm(value);
}


void op_movss_xmmm32_xmm_impl()
{
	uint8_t value[16];

	/* xmm <= xmm: bits 127-32 of xmm set to 0.
	 * m32 <= xmm: copy 32 bits to memory */
	isa_load_xmm(value);
	memset(value + 4, 0, 12);
	if (isa_inst.modrm_mod == 3)
		memcpy(&isa_regs->xmm[isa_inst.modrm_rm], value, 16);
	else
		isa_store_xmmm32(value);
}


void op_palignr_xmm_xmmm128_imm8_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_pand_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_pcmpeqb_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_pcmpeqw_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_pcmpeqd_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_pcmpistri_xmm_xmmm128_imm8_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_pinsrb_xmm_r32m8_imm8_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_pinsrd_xmm_rm32_imm8_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_punpcklbw_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_punpcklwd_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_punpckldq_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_punpcklqdq_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_pshufb_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_pshufd_xmm_xmmm128_imm8_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_pslldq_xmmm128_imm8_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_psrldq_xmmm128_imm8_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_stmxcsr_m32_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_psubb_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_psubw_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_psubd_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_ptest_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


void op_pxor_xmm_xmmm128_impl()
{
	fatal("%s: not implemented", __FUNCTION__);
}


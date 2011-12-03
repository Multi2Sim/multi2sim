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

#include <cpukernel.h>


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


void op_cvttsd2si_r32_xmmm64_impl()
{
	uint8_t xmm[16];
	uint32_t r32;

	isa_load_xmmm64(xmm);

	__ISA_ASM_START__
	asm volatile (
		"cvttsd2si %1, %%eax\n\t"
		"mov %%eax, %0"
		: "=m" (r32)
		: "m" (*xmm)
		: "eax"
	);
	__ISA_ASM_END__

	isa_store_r32(r32);

	x86_uinst_new(x86_uinst_xmm_conv, x86_dep_xmmm64, 0, 0, x86_dep_r32, 0, 0, 0);
}


void op_cvttss2si_r32_xmmm32_impl()
{
	uint8_t xmm[16];
	uint32_t r32;

	isa_load_xmmm32(xmm);

	__ISA_ASM_START__
	asm volatile (
		"cvttss2si %1, %%eax\n\t"
		"mov %%eax, %0"
		: "=m" (r32)
		: "m" (*xmm)
		: "eax"
	);
	__ISA_ASM_END__

	isa_store_r32(r32);

	x86_uinst_new(x86_uinst_xmm_conv, x86_dep_xmmm32, 0, 0, x86_dep_r32, 0, 0, 0);
}


void op_ldmxcsr_m32_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_movaps_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_movaps_xmmm128_xmm_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_movd_xmm_rm32_impl()
{
	uint8_t xmm[16];
	uint32_t value = isa_load_rm32();

	memset(xmm, 0, 16);
	* (uint32_t *) xmm = value;

	isa_store_xmm(xmm);

	x86_uinst_new(x86_uinst_xmm_move, x86_dep_rm32, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void op_movd_rm32_xmm_impl()
{
	uint8_t xmm[16];
	uint32_t value;

	isa_load_xmm(xmm);
	value = * (uint32_t *) xmm;

	isa_store_rm32(value);

	x86_uinst_new(x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_rm32, 0, 0, 0);
}


void op_movdqa_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_movdqa_xmmm128_xmm_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_movdqu_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_movdqu_xmmm128_xmm_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_movhpd_xmm_m64_impl()
{
	uint64_t m64;
	uint8_t xmm[16];

	isa_load_xmm(xmm);
	m64 = isa_load_m64();
	* (uint64_t *) &xmm[8] = m64;
	isa_store_xmm(xmm);

	x86_uinst_new(x86_uinst_xmm_move, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void op_movhpd_m64_xmm_impl()
{
	uint64_t m64;
	uint8_t xmm[16];

	isa_load_xmm(xmm);
	m64 = * (uint64_t *) &xmm[8];
	isa_store_m64(m64);

	x86_uinst_new(x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm64, 0, 0, 0);
}


void op_movlpd_xmm_m64_impl()
{
	uint64_t m64;
	uint8_t xmm[16];

	isa_load_xmm(xmm);
	m64 = isa_load_m64();
	* (uint64_t *) xmm = m64;
	isa_store_xmm(xmm);

	x86_uinst_new(x86_uinst_xmm_move, x86_dep_xmmm64, x86_dep_xmm, 0, x86_dep_xmm, 0, 0, 0);
}


void op_movlpd_m64_xmm_impl()
{
	uint64_t m64;
	uint8_t xmm[16];

	isa_load_xmm(xmm);
	m64 = * (uint64_t *) xmm;
	isa_store_m64(m64);

	x86_uinst_new(x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm64, 0, 0, 0);
}


void op_pmovmskb_r32_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_movntdq_m128_xmm_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_movq_xmm_xmmm64_impl()
{
	uint8_t value[16];

	memset(value, 0, 16);
	isa_load_xmmm64(value);
	isa_store_xmm(value);

	x86_uinst_new(x86_uinst_xmm_move, x86_dep_xmmm64, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void op_movq_xmmm64_xmm_impl()
{
	uint8_t value[16];

	memset(value, 0, 16);
	if (isa_inst.modrm_mod == 3)
		memcpy(&isa_regs->xmm[isa_inst.modrm_rm], value, 16);
	isa_load_xmm(value);
	isa_store_xmmm64(value);

	x86_uinst_new(x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm64, 0, 0, 0);
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

	x86_uinst_new(x86_uinst_xmm_move, x86_dep_xmmm32, 0, 0, x86_dep_xmm, 0, 0, 0);
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

	x86_uinst_new(x86_uinst_xmm_move, x86_dep_xmm, 0, 0, x86_dep_xmmm32, 0, 0, 0);
}


void op_palignr_xmm_xmmm128_imm8_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_pand_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_pcmpeqb_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_pcmpeqw_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_pcmpeqd_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_pcmpistri_xmm_xmmm128_imm8_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_pinsrb_xmm_r32m8_imm8_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_pinsrd_xmm_rm32_imm8_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_punpcklbw_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_punpcklwd_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_punpckldq_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_punpcklqdq_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_pshufb_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_pshufd_xmm_xmmm128_imm8_impl()
{
	uint32_t src[4], dst[4];
	uint8_t imm = isa_inst.imm.b;

	isa_load_xmmm128((uint8_t *) src);
	dst[0] = src[imm & 3];
	dst[1] = src[(imm >> 2) & 3];
	dst[2] = src[(imm >> 4) & 3];
	dst[3] = src[(imm >> 6) & 3];
	isa_store_xmm((uint8_t *) dst);

	x86_uinst_new(x86_uinst_xmm_shuf, x86_dep_xmmm128, 0, 0, x86_dep_xmm, 0, 0, 0);
}


void op_pslldq_xmmm128_imm8_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_psrldq_xmmm128_imm8_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_stmxcsr_m32_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_psubb_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_psubw_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_psubd_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_ptest_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


void op_pxor_xmm_xmmm128_impl()
{
	isa_error("%s: not implemented", __FUNCTION__);
}


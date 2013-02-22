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
#include <limits.h>
#include <math.h>

#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <mem-system/memory.h>

#include "emu.h"
#include "isa.h"
#include "machine.h"
#include "wavefront.h"
#include "work-group.h"
#include "work-item.h"


char *err_si_isa_note =
	"\tThe AMD Southern Islands instruction set is partially supported by\n"
	"\tMulti2Sim. If your program is using an unimplemented instruction,\n"
	"\tplease email development@multi2sim.org' to request support for it.\n";

#define NOT_IMPL() fatal("GPU instruction '%s' not implemented\n%s", \
	inst->info->name, err_si_isa_note)


#define INST SI_INST_SMRD
void si_isa_S_BUFFER_LOAD_DWORD_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t value;

	unsigned int m_offset;
	unsigned int m_base;

	/* unsigned int m_size; */
	struct si_buffer_resource_t buf_desc;
	int sbase;
	struct si_wavefront_t *wavefront;

	wavefront = work_item->wavefront;

	/* Record access */
	wavefront->scalar_mem_read = 1;

	sbase = INST.sbase << 1;

	/* sbase holds the first of 4 registers containing the buffer
	 * resource descriptor */
	si_isa_read_buf_res(work_item, &buf_desc, sbase);

	/* sgpr[dst] = read_dword_from_kcache(m_base, m_offset, m_size) */
	m_base = buf_desc.base_addr;
	m_offset =
		(INST.imm) ? (INST.offset * 4) : si_isa_read_sreg(work_item,
		INST.offset);
	/* m_size = (buf_desc.stride == 0) ? 1 : buf_desc.num_records; */

	mem_read(si_emu->global_mem, m_base + m_offset, 4, &value);

	/* Store the data in the destination register */
	si_isa_write_sreg(work_item, INST.sdst, value.as_uint);

	/* FIXME Set value based on type */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%u,%gf)", INST.sdst, value.as_uint,
			value.as_float);
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = m_base + m_offset;
	work_item->global_mem_access_size = 4;
}
#undef INST

#define INST SI_INST_SMRD
void si_isa_S_BUFFER_LOAD_DWORDX2_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t value[2];

	unsigned int m_base;
	unsigned int m_offset;
	unsigned int m_addr;

	struct si_mem_ptr_t mem_ptr;
	struct si_wavefront_t *wavefront;

	int sbase;
	int i;

	wavefront = work_item->wavefront;

	/* Record access */
	wavefront->scalar_mem_read = 1;

	sbase = INST.sbase << 1;

	si_isa_read_mem_ptr(work_item, &mem_ptr, sbase);

	/* assert(uav_table_ptr.addr < UINT32_MAX) */

	m_base = mem_ptr.addr;
	m_offset =
		(INST.imm) ? (INST.offset * 4) : si_isa_read_sreg(work_item,
		INST.offset);
	m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	for (i = 0; i < 2; i++)
	{
		mem_read(si_emu->global_mem, m_base + m_offset + i * 4, 4,
			&value[i]);
		si_isa_write_sreg(work_item, INST.sdst + i, value[i].as_uint);
	}

	/* FIXME Set value based on type */
	if (debug_status(si_isa_debug_category))
	{
		for (i = 0; i < 2; i++)
		{
			si_isa_debug("S%u<=(%u,%gf) ", INST.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = m_base + m_offset;
	work_item->global_mem_access_size = 4 * 2;
}
#undef INST

#define INST SI_INST_SMRD
void si_isa_S_BUFFER_LOAD_DWORDX4_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t value[4];

	unsigned int m_base;
	unsigned int m_offset;
	unsigned int m_addr;

	struct si_mem_ptr_t mem_ptr;
	struct si_wavefront_t *wavefront;

	int sbase;
	int i;

	wavefront = work_item->wavefront;

	/* Record access */
	wavefront->scalar_mem_read = 1;

	sbase = INST.sbase << 1;

	si_isa_read_mem_ptr(work_item, &mem_ptr, sbase);

	/* assert(uav_table_ptr.addr < UINT32_MAX) */

	m_base = mem_ptr.addr;
	m_offset =
		(INST.imm) ? (INST.offset * 4) : si_isa_read_sreg(work_item,
		INST.offset);
	m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	for (i = 0; i < 4; i++)
	{
		mem_read(si_emu->global_mem, m_base + m_offset + i * 4, 4,
			&value[i]);
		si_isa_write_sreg(work_item, INST.sdst + i, value[i].as_uint);
	}

	/* FIXME Set value based on type */
	if (debug_status(si_isa_debug_category))
	{
		for (i = 0; i < 4; i++)
		{
			si_isa_debug("S%u<=(%u,%gf) ", INST.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = m_base + m_offset;
	work_item->global_mem_access_size = 4 * 4;
}
#undef INST

#define INST SI_INST_SMRD
void si_isa_S_LOAD_DWORDX4_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t value[4];
	unsigned int m_base;
	unsigned int m_offset;
	unsigned int m_addr;
	struct si_mem_ptr_t mem_ptr;
	int sbase;
	int i;

	assert(INST.imm);

	sbase = INST.sbase << 1;

	si_isa_read_mem_ptr(work_item, &mem_ptr, sbase);

	/* assert(uav_table_ptr.addr < UINT32_MAX) */

	m_base = mem_ptr.addr;
	m_offset = INST.offset * 4;
	m_addr = m_base + m_offset;

	assert(!(m_addr & 0x3));

	for (i = 0; i < 4; i++)
	{
		mem_read(si_emu->global_mem, m_base + m_offset + i * 4, 4,
			&value[i]);
		si_isa_write_sreg(work_item, INST.sdst + i, value[i].as_uint);
	}

	/* FIXME Set value based on type */
	if (debug_status(si_isa_debug_category))
	{
		for (i = 0; i < 4; i++)
		{
			si_isa_debug("S%u<=(%u,%gf) ", INST.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = m_base + m_offset;
	work_item->global_mem_access_size = 4 * 4;
}
#undef INST

#define iNST SI_INST_SMRD
void si_isa_S_LOAD_DWORDX8_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST


/* D.u = S0.u + S1.u. SCC = carry out. */
#define INST SI_INST_SOP2
void si_isa_S_ADD_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t sum;
	union si_reg_t carry;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Calculate the sum and carry out. */
	sum.as_uint = s0 + s1;
	carry.as_uint =
		((unsigned long long) s0 + (unsigned long long) s1) >> 32;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, sum.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, carry.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%u) ", INST.sdst, sum.as_uint);
		si_isa_debug("scc<=(%u) ", carry.as_uint);
	}
}
#undef INST

/* D.u = S0.i + S1.i. scc = overflow. */
#define INST SI_INST_SOP2
void si_isa_S_ADD_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t sum;
	union si_reg_t ovf;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Calculate the sum and overflow. */
	sum.as_uint = s0 + s1;
	ovf.as_uint = s0 >> 31 != s1 >> 31 ? 0 : (s0 > 0 && sum.as_int < 0) ||
		(s0 < 0 && sum.as_int > 0);

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, sum.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, ovf.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%u) ", INST.sdst, sum.as_uint);
		si_isa_debug("scc<=(%u) ", ovf.as_uint);
	}
}
#undef INST

/* D.u = S0.i - S1.i. scc = overflow. */
#define INST SI_INST_SOP2
void si_isa_S_SUB_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t diff;
	union si_reg_t ovf;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Calculate the sum and overflow. */
	diff.as_uint = s0 - s1;
	ovf.as_uint = s0 >> 31 != s1 >> 31 ? (s0 > 0 && diff.as_int < 0) ||
		(s0 < 0 && diff.as_int > 0) : 0;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, diff.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, ovf.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d) ", INST.sdst, diff.as_int);
		si_isa_debug("scc<=(%u) ", ovf.as_uint);
	}
}
#undef INST

/* D.u = (S0.u < S1.u) ? S0.u : S1.u, scc = 1 if S0 is min. */
#define INST SI_INST_SOP2
void si_isa_S_MIN_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t min;
	union si_reg_t s0_min;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Calculate the minimum operand. */
	if (s0 < s1)
	{
		min.as_uint = s0;
		s0_min.as_uint = 1;
	}
	else
	{
		min.as_uint = s1;
		s0_min.as_uint = 0;
	}

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, min.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, s0_min.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%u) ", INST.sdst, min.as_uint);
		si_isa_debug("scc<=(%d) ", s0_min.as_uint);
	}
}
#undef INST

/* D.i = (S0.i > S1.i) ? S0.i : S1.i, scc = 1 if S0 is max. */
#define INST SI_INST_SOP2
void si_isa_S_MAX_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t max;
	union si_reg_t s0_max;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Calculate the minimum operand. */
	if (s0 > s1)
	{
		max.as_int = s0;
		s0_max.as_uint = 1;
	}
	else
	{
		max.as_int = s1;
		s0_max.as_uint = 0;
	}

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, max.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, s0_max.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d) ", INST.sdst, max.as_int);
		si_isa_debug("scc<=(%u) ", s0_max.as_uint);
	}
}
#undef INST

/* D.u = (S0.u > S1.u) ? S0.u : S1.u, scc = 1 if S0 is max. */
#define INST SI_INST_SOP2
void si_isa_S_MAX_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t max;
	union si_reg_t s0_max;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Calculate the minimum operand. */
	if (s0 > s1)
	{
		max.as_uint = s0;
		s0_max.as_uint = 1;
	}
	else
	{
		max.as_uint = s1;
		s0_max.as_uint = 0;
	}

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, max.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, s0_max.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%u) ", INST.sdst, max.as_uint);
		si_isa_debug("scc<=(%u) ", s0_max.as_uint);
	}
}
#undef INST

/* D.u = SCC ? S0.u : S1.u */
#define INST SI_INST_SOP2
void si_isa_S_CSELECT_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;
	unsigned int scc = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);
	scc = si_isa_read_sreg(work_item, SI_SCC);

	/* Calculate the result */
	result.as_uint = scc ? s0 : s1;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, result.as_uint);
	}
}
#undef INST

/* D.u = S0.u & S1.u. scc = 1 if result is non-zero. */
#define INST SI_INST_SOP2
void si_isa_S_AND_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;
	union si_reg_t nonzero;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result.as_uint = s0 & s1;
	nonzero.as_uint = ! !result.as_uint;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, result.as_uint);
		si_isa_debug("scc<=(%u) ", nonzero.as_uint);
	}
}
#undef INST

/* D.u = S0.u & S1.u. scc = 1 if result is non-zero. */
#define INST SI_INST_SOP2
void si_isa_S_AND_B64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* Assert no literal constants for a 64 bit instruction. */
	assert(!(INST.ssrc0 == 0xFF || INST.ssrc1 == 0xFF));

	unsigned int s0_lo;
	unsigned int s0_hi;
	unsigned int s1_lo;
	unsigned int s1_hi;

	union si_reg_t result_lo;
	union si_reg_t result_hi;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	s0_lo = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi = si_isa_read_sreg(work_item, INST.ssrc0 + 1);
	s1_lo = si_isa_read_sreg(work_item, INST.ssrc1);
	s1_hi = si_isa_read_sreg(work_item, INST.ssrc1 + 1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = s0_lo & s1_lo;
	result_hi.as_uint = s0_hi & s1_hi;
	nonzero.as_uint = result_lo.as_uint && result_hi.as_uint;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result_lo.as_uint);
	si_isa_write_sreg(work_item, INST.sdst + 1, result_hi.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, result_lo.as_uint);
		si_isa_debug("S%u<=(0x%x) ", INST.sdst + 1, result_hi.as_uint);
		si_isa_debug("scc<=(%u) ", nonzero.as_uint);
	}
}
#undef INST

/* D.u = S0.u | S1.u. scc = 1 if result is non-zero. */
#define INST SI_INST_SOP2
void si_isa_S_OR_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;
	union si_reg_t nonzero;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result.as_uint = s0 | s1;
	nonzero.as_uint = ! !result.as_uint;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, result.as_uint);
		si_isa_debug("scc<=(%u) ", nonzero.as_uint);
	}
}
#undef INST

/* D.u = S0.u | S1.u. scc = 1 if result is non-zero. */
#define INST SI_INST_SOP2
void si_isa_S_OR_B64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* Assert no literal constants for a 64 bit instruction. */
	assert(!(INST.ssrc0 == 0xFF || INST.ssrc1 == 0xFF));

	unsigned int s0_lo;
	unsigned int s0_hi;
	unsigned int s1_lo;
	unsigned int s1_hi;

	union si_reg_t result_lo;
	union si_reg_t result_hi;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	s0_lo = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi = si_isa_read_sreg(work_item, INST.ssrc0 + 1);
	s1_lo = si_isa_read_sreg(work_item, INST.ssrc1);
	s1_hi = si_isa_read_sreg(work_item, INST.ssrc1 + 1);

	/* Bitwise OR the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = s0_lo | s1_lo;
	result_hi.as_uint = s0_hi | s1_hi;
	nonzero.as_uint = result_lo.as_uint && result_hi.as_uint;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result_lo.as_uint);
	si_isa_write_sreg(work_item, INST.sdst + 1, result_hi.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, result_lo.as_uint);
		si_isa_debug("S%u<=(0x%x) ", INST.sdst + 1, result_hi.as_uint);
		si_isa_debug("scc<=(%u) ", nonzero.as_uint);
	}
}
#undef INST

/* D.u = S0.u ^ S1.u. scc = 1 if result is non-zero. */
#define INST SI_INST_SOP2
void si_isa_S_XOR_B64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* Assert no literal constants for a 64 bit instruction. */
	assert(!(INST.ssrc0 == 0xFF || INST.ssrc1 == 0xFF));

	unsigned int s0_lo;
	unsigned int s0_hi;
	unsigned int s1_lo;
	unsigned int s1_hi;

	union si_reg_t result_lo;
	union si_reg_t result_hi;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	s0_lo = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi = si_isa_read_sreg(work_item, INST.ssrc0 + 1);
	s1_lo = si_isa_read_sreg(work_item, INST.ssrc1);
	s1_hi = si_isa_read_sreg(work_item, INST.ssrc1 + 1);

	/* Bitwise OR the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = s0_lo ^ s1_lo;
	result_hi.as_uint = s0_hi ^ s1_hi;
	nonzero.as_uint = result_lo.as_uint && result_hi.as_uint;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result_lo.as_uint);
	si_isa_write_sreg(work_item, INST.sdst + 1, result_hi.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, result_lo.as_uint);
		si_isa_debug("S%u<=(0x%x) ", INST.sdst + 1, result_hi.as_uint);
		si_isa_debug("scc<=(%u) ", nonzero.as_uint);
	}
}
#undef INST

/* D.u = S0.u & ~S1.u. scc = 1 if result is non-zero. */
#define INST SI_INST_SOP2
void si_isa_S_ANDN2_B64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* Assert no literal constants for a 64 bit instruction. */
	assert(!(INST.ssrc0 == 0xFF || INST.ssrc1 == 0xFF));

	unsigned int s0_lo;
	unsigned int s0_hi;
	unsigned int s1_lo;
	unsigned int s1_hi;

	union si_reg_t result_lo;
	union si_reg_t result_hi;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	s0_lo = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi = si_isa_read_sreg(work_item, INST.ssrc0 + 1);
	s1_lo = si_isa_read_sreg(work_item, INST.ssrc1);
	s1_hi = si_isa_read_sreg(work_item, INST.ssrc1 + 1);

	/* Bitwise AND the first operand with the negation of the second and
	 * determine if the result is non-zero. */
	result_lo.as_uint = s0_lo & ~s1_lo;
	result_hi.as_uint = s0_hi & ~s1_hi;
	nonzero.as_uint = result_lo.as_uint && result_hi.as_uint;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result_lo.as_uint);
	si_isa_write_sreg(work_item, INST.sdst + 1, result_hi.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, result_lo.as_uint);
		si_isa_debug("S%u<=(0x%x) ", INST.sdst + 1, result_hi.as_uint);
		si_isa_debug("scc<=(%u)", nonzero.as_uint);
	}
}
#undef INST

/* D.u = ~(S0.u & S1.u). scc = 1 if result is non-zero. */
#define INST SI_INST_SOP2
void si_isa_S_NAND_B64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* Assert no literal constants for a 64 bit instruction. */
	assert(!(INST.ssrc0 == 0xFF || INST.ssrc1 == 0xFF));

	unsigned int s0_lo;
	unsigned int s0_hi;
	unsigned int s1_lo;
	unsigned int s1_hi;

	union si_reg_t result_lo;
	union si_reg_t result_hi;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	s0_lo = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi = si_isa_read_sreg(work_item, INST.ssrc0 + 1);
	s1_lo = si_isa_read_sreg(work_item, INST.ssrc1);
	s1_hi = si_isa_read_sreg(work_item, INST.ssrc1 + 1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = ~(s0_lo & s1_lo);
	result_hi.as_uint = ~(s0_hi & s1_hi);
	nonzero.as_uint = result_lo.as_uint && result_hi.as_uint;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result_lo.as_uint);
	si_isa_write_sreg(work_item, INST.sdst + 1, result_hi.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, result_lo.as_uint);
		si_isa_debug("S%u<=(0x%x) ", INST.sdst + 1, result_hi.as_uint);
		si_isa_debug("scc<=(%u) ", nonzero.as_uint);
	}
}
#undef INST

/* D.u = S0.u << S1.u[4:0]. scc = 1 if result is non-zero. */
#define INST SI_INST_SOP2
void si_isa_S_LSHL_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;
	union si_reg_t nonzero;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst & 0x1F;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1) & 0x1F;

	/* Left shift the first operand by the second and determine if the
	 * result is non-zero. */
	result.as_uint = s0 << s1;
	nonzero.as_uint = result.as_uint != 0;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, result.as_uint);
		si_isa_debug("scc<=(%u)", nonzero.as_uint);
	}
}
#undef INST

/* D.u = S0.u >> S1.u[4:0]. scc = 1 if result is non-zero. */
#define INST SI_INST_SOP2
void si_isa_S_LSHR_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;
	union si_reg_t nonzero;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst & 0x1F;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1) & 0x1F;

	/* Right shift the first operand by the second and determine if the
	 * result is non-zero. */
	result.as_uint = s0 >> s1;
	nonzero.as_uint = result.as_uint != 0;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, result.as_uint);
		si_isa_debug("scc<=(%u)", nonzero.as_uint);
	}
}
#undef INST

/* D.i = signext(S0.i) >> S1.i[4:0]. scc = 1 if result is non-zero. */
#define INST SI_INST_SOP2
void si_isa_S_ASHR_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t result;
	union si_reg_t nonzero;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst & 0x1F;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1) & 0x1F;
	long long se_s0 = s0;

	/* Right shift the first operand sign extended by the second and
	 * determine if the result is non-zero. */
	result.as_int = se_s0 >> s1;
	nonzero.as_uint = result.as_uint != 0;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d) ", INST.sdst, result.as_int);
		si_isa_debug("scc<=(%u)", nonzero.as_uint);
	}
}
#undef INST

/* D.i = S0.i * S1.i. */
#define INST SI_INST_SOP2
void si_isa_S_MUL_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Multiply the two operands. */
	result.as_int = s0 * s1;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)", INST.sdst, result.as_int);
	}
}
#undef INST

/* D.i = signext(simm16). */
#define INST SI_INST_SOPK
void si_isa_S_MOVK_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	short simm16;
	union si_reg_t result;

	/* Load constant operand from instruction. */
	simm16 = INST.simm16;

	/* Sign extend the short constant to an integer. */
	result.as_int = (int) simm16;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)", INST.sdst, result.as_int);
	}
}
#undef INST

/* D.i = D.i + signext(SIMM16). scc = overflow. */
#define INST SI_INST_SOPK
void si_isa_S_ADDK_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	short simm16;

	int se_simm16;
	int dest;

	union si_reg_t sum;
	union si_reg_t ovf;

	/* Load short constant operand from instruction and sign extend to an 
	 * integer. */
	simm16 = INST.simm16;
	se_simm16 = simm16;

	/* Load operand from destination register. */
	dest = si_isa_read_sreg(work_item, INST.sdst);

	/* Add the two operands and determine overflow. */
	sum.as_int = dest + se_simm16;
	ovf.as_uint = dest >> 31 != se_simm16 >> 31 ? 0 :
		(dest > 0 && sum.as_int < 0) || (dest < 0 && sum.as_int > 0);

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, sum.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, ovf.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)", INST.sdst, sum.as_int);
		si_isa_debug("scc<=(%u)", ovf.as_uint);
	}
}
#undef INST

/* D.i = D.i * signext(SIMM16). scc = overflow. */
#define INST SI_INST_SOPK
void si_isa_S_MULK_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	short simm16;

	int se_simm16;
	int dest;

	union si_reg_t product;
	union si_reg_t ovf;

	/* Load short constant operand from instruction and sign extend to an 
	 * integer. */
	simm16 = INST.simm16;
	se_simm16 = simm16;

	/* Load operand from destination register. */
	dest = si_isa_read_sreg(work_item, INST.sdst);

	/* Multiply the two operands and determine overflow. */
	product.as_int = dest * se_simm16;
	ovf.as_uint =
		((long) dest * (long) se_simm16) > (long) product.as_uint;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, product.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, ovf.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)", INST.sdst, product.as_int);
		si_isa_debug("scc<=(%u)", ovf.as_uint);
	}
}
#undef INST

/* D.u = S0.u. */
#define INST SI_INST_SOP1
void si_isa_S_MOV_B64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* Assert no literal constant with a 64 bit instruction. */
	assert(!(INST.ssrc0 == 0xFF));

	union si_reg_t s0_lo;
	union si_reg_t s0_hi;

	/* Load operand from registers. */
	s0_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc0 + 1);

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, s0_lo.as_uint);
	si_isa_write_sreg(work_item, INST.sdst + 1, s0_hi.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, s0_lo.as_uint);
		si_isa_debug("S%u<=(0x%x)", INST.sdst + 1, s0_hi.as_uint);
	}
}
#undef INST

/* D.u = S0.u. */
#define INST SI_INST_SOP1
void si_isa_S_MOV_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;

	/* Load operand from registers or as a literal constant. */
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, s0.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, s0.as_uint);
	}
}
#undef INST

/* D.u = ~S0.u SCC = 1 if result non-zero. */
#define INST SI_INST_SOP1
void si_isa_S_NOT_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t nonzero;

	/* Load operand from registers or as a literal constant. */
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = ~INST.lit_cnst;
	else
		s0.as_uint = ~si_isa_read_sreg(work_item, INST.ssrc0);
	nonzero.as_uint = ! !INST.lit_cnst;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, s0.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, s0.as_uint);
	}
}
#undef INST

/* D.u = PC + 4, PC = S0.u */
#define INST SI_INST_SOP1
void si_isa_S_SWAPPC_B64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = EXEC, EXEC = S0.u & EXEC. scc = 1 if the new value of EXEC is
 * non-zero. */
#define INST SI_INST_SOP1
void si_isa_S_AND_SAVEEXEC_B64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* Assert no literal constant with a 64 bit instruction. */
	assert(!(INST.ssrc0 == 0xFF));

	unsigned int exec_lo;
	unsigned int exec_hi;
	unsigned int s0_lo;
	unsigned int s0_hi;

	union si_reg_t exec_new_lo;
	union si_reg_t exec_new_hi;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	exec_lo = si_isa_read_sreg(work_item, SI_EXEC);
	exec_hi = si_isa_read_sreg(work_item, SI_EXEC + 1);
	s0_lo = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi = si_isa_read_sreg(work_item, INST.ssrc0 + 1);

	/* Bitwise AND exec and the first operand and determine if the result 
	 * is non-zero. */
	exec_new_lo.as_uint = s0_lo & exec_lo;
	exec_new_hi.as_uint = s0_hi & exec_hi;
	nonzero.as_uint = exec_new_lo.as_uint && exec_new_hi.as_uint;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, exec_lo);
	si_isa_write_sreg(work_item, INST.sdst + 1, exec_hi);
	si_isa_write_sreg(work_item, SI_EXEC, exec_new_lo.as_uint);
	si_isa_write_sreg(work_item, SI_EXEC + 1, exec_new_hi.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, exec_lo);
		si_isa_debug("S%u<=(0x%x) ", INST.sdst + 1, exec_hi);
		si_isa_debug("exec_lo<=(0x%x) ", exec_new_lo.as_uint);
		si_isa_debug("exec_hi<=(0x%x) ", exec_new_hi.as_uint);
		si_isa_debug("scc<=(%u)", nonzero.as_uint);
	}
}
#undef INST

/* scc = (S0.i == S1.i). */
#define INST SI_INST_SOPC
void si_isa_S_CMP_EQ_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0 == s1);

	/* Write the results. */
	si_isa_write_sreg(work_item, SI_SCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

/* scc = (S0.i > S1.i). */
#define INST SI_INST_SOPC
void si_isa_S_CMP_GT_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0 > s1);

	/* Write the results. */
	si_isa_write_sreg(work_item, SI_SCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

/* scc = (S0.i >= S1.i). */
#define INST SI_INST_SOPC
void si_isa_S_CMP_GE_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0 >= s1);

	/* Write the results. */
	si_isa_write_sreg(work_item, SI_SCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

/* scc = (S0.i < S1.i). */
#define INST SI_INST_SOPC
void si_isa_S_CMP_LT_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0 < s1);

	/* Write the results. */
	si_isa_write_sreg(work_item, SI_SCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

/* scc = (S0.i <= S1.i). */
#define INST SI_INST_SOPC
void si_isa_S_CMP_LE_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0 <= s1);

	/* Write the results. */
	si_isa_write_sreg(work_item, SI_SCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

/* scc = (S0.u > S1.u). */
#define INST SI_INST_SOPC
void si_isa_S_CMP_GT_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0 > s1);

	/* Write the results. */
	si_isa_write_sreg(work_item, SI_SCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

/* scc = (S0.u >= S1.u). */
#define INST SI_INST_SOPC
void si_isa_S_CMP_GE_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0 >= s1);

	/* Write the results. */
	si_isa_write_sreg(work_item, SI_SCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

/* scc = (S0.u <= S1.u). */
#define INST SI_INST_SOPC
void si_isa_S_CMP_LE_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0 <= s1);

	/* Write the results. */
	si_isa_write_sreg(work_item, SI_SCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

/* End the program. */
void si_isa_S_ENDPGM_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	work_item->wavefront->finished = 1;
}

/* PC = PC + signext(SIMM16 * 4) + 4 */
#define INST SI_INST_SOPP
void si_isa_S_BRANCH_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	short simm16;
	int se_simm16;

	/* Load the short constant operand and sign extend into an integer. */
	simm16 = INST.simm16;
	se_simm16 = simm16;

	/* Relative jump */
	work_item->wavefront->pc += se_simm16 * 4 + 4 - inst->info->size;
}
#undef INST

/* if(SCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */
#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_SCC0_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	short simm16;
	int se_simm16;

	if (!si_isa_read_sreg(work_item, SI_SCC))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		/* Determine the program counter to branch to. */
		work_item->wavefront->pc +=
			se_simm16 * 4 + 4 - inst->info->size;
	}
}
#undef INST


/* if(SCC == 1) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */
#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_SCC1_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	short simm16;
	int se_simm16;

	if (si_isa_read_sreg(work_item, SI_SCC))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		/* Determine the program counter to branch to. */
		work_item->wavefront->pc +=
			se_simm16 * 4 + 4 - inst->info->size;
	}
}
#undef INST

/* if(VCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */
#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_VCCZ_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	short simm16;
	int se_simm16;

	if (si_isa_read_sreg(work_item, SI_VCCZ))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		/* Determine the program counter to branch to. */
		work_item->wavefront->pc +=
			se_simm16 * 4 + 4 - inst->info->size;
	}
}
#undef INST

/* if(VCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */
#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_VCCNZ_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	short simm16;
	int se_simm16;

	if (!si_isa_read_sreg(work_item, SI_VCCZ))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		/* Determine the program counter to branch to. */
		work_item->wavefront->pc +=
			se_simm16 * 4 + 4 - inst->info->size;
	}
}
#undef INST


/* if(EXEC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */
#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_EXECZ_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	short simm16;
	int se_simm16;

	if (si_isa_read_sreg(work_item, SI_EXECZ))
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		/* Determine the program counter to branch to. */
		work_item->wavefront->pc +=
			se_simm16 * 4 + 4 - inst->info->size;
	}
}
#undef INST


/* if(EXEC != 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */
#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_EXECNZ_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* Suspend current wavefront at the barrier. If all wavefronts in work-group
 * reached the barrier, wake them up */
void si_isa_S_BARRIER_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	struct si_wavefront_t *wavefront = work_item->wavefront;
	struct si_work_group_t *work_group = work_item->work_group;

	/* Suspend current wavefront at the barrier */
	wavefront->barrier = 1;
	assert(DOUBLE_LINKED_LIST_MEMBER(work_group, running, wavefront));
	DOUBLE_LINKED_LIST_REMOVE(work_group, running, wavefront);
	DOUBLE_LINKED_LIST_INSERT_TAIL(work_group, barrier, wavefront);
	si_isa_debug("%s (gid=%d) reached barrier (%d reached, %d left)\n",
		wavefront->name, work_group->id,
		work_group->barrier_list_count,
		work_group->wavefront_count - work_group->barrier_list_count);

	/* If all wavefronts in work-group reached the barrier, wake them up */
	if (work_group->barrier_list_count == work_group->wavefront_count)
	{
		struct si_wavefront_t *wavefront;

		while (work_group->barrier_list_head)
		{
			wavefront = work_group->barrier_list_head;
			DOUBLE_LINKED_LIST_REMOVE(work_group, barrier,
				wavefront);
			DOUBLE_LINKED_LIST_INSERT_TAIL(work_group, running,
				wavefront);
		}
		assert(work_group->running_list_count ==
			work_group->wavefront_count);
		assert(work_group->barrier_list_count == 0);
		si_isa_debug("%s completed barrier, waking up wavefronts\n",
			work_group->name);
	}
}

void si_isa_S_WAITCNT_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* Nothing to do in emulation */
	work_item->wavefront->mem_wait = 1;
}

/* Do nothing */
#define INST SI_INST_VOP1
void si_isa_V_NOP_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* FIXME: Do nothing */
}
#undef INST

/* D.u = S0.u. */
#define INST SI_INST_VOP1
void si_isa_V_MOV_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t value;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		value.as_uint = INST.lit_cnst;
	else
		value.as_uint = si_isa_read_reg(work_item, INST.src0);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, value.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			value.as_uint);
	}
}
#undef INST

/* Copy one VGPR value to one SGPR. */
#define INST SI_INST_VOP1
void si_isa_V_READFIRSTLANE_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t value;

	/* Load operand from register. */
	assert(INST.src0 >= 256 || INST.src0 == SI_M0);
	value.as_uint = si_isa_read_reg(work_item, INST.src0);

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.vdst, value.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			value.as_uint);
	}
}
#undef INST

/* D.i = (int)S0.d. */
#define INST SI_INST_VOP1
void si_isa_V_CVT_I32_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.f = (float)S0.i. */
#define INST SI_INST_VOP1
void si_isa_V_CVT_F64_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0;
	union
	{
		double as_double;
		unsigned int as_reg[2];

	} value;
	union si_reg_t result_lo;
	union si_reg_t result_hi;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);

	/* Convert and separate value. */
	value.as_double = (double) s0;

	/* Write the results. */
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	si_isa_write_vreg(work_item, INST.vdst, result_lo.as_uint);
	si_isa_write_vreg(work_item, INST.vdst + 1, result_hi.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V[%u:+1]<=(%lgf) ", work_item->id,
			INST.vdst, value.as_double);
	}
}
#undef INST

/* D.f = (float)S0.i. */
#define INST SI_INST_VOP1
void si_isa_V_CVT_F32_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0;
	union si_reg_t value;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	value.as_float = (float) s0;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, value.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			value.as_float);
	}
}
#undef INST

/* D.f = (float)S0.u. */
#define INST SI_INST_VOP1
void si_isa_V_CVT_F32_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	union si_reg_t value;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	value.as_float = (float) s0;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, value.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			value.as_float);
	}
}
#undef INST

/* D.i = (uint)S0.f. */
#define INST SI_INST_VOP1
void si_isa_V_CVT_U32_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	float fvalue;
	union si_reg_t value;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	value.as_uint = s0;
	fvalue = value.as_float;

	/* Handle special number cases and cast to an unsigned int */

	/* -inf, NaN, 0, -0 --> 0 */
	if ((isinf(fvalue) && fvalue < 0.0f) || isnan(fvalue)
		|| fvalue == 0.0f || fvalue == -0.0f)
		value.as_uint = 0;
	/* inf, > max_uint --> max_uint */
	else if (isinf(fvalue) || fvalue >= UINT_MAX)
		value.as_uint = UINT_MAX;
	else
		value.as_uint = (unsigned int) fvalue;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, value.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u) ", work_item->id, INST.vdst,
			value.as_uint);
	}
}
#undef INST

/* D.i = (int)S0.f. */
#define INST SI_INST_VOP1
void si_isa_V_CVT_I32_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	float fvalue;
	union si_reg_t value;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	value.as_uint = s0;
	fvalue = value.as_float;

	/* Handle special number cases and cast to an int */

	/* inf, > max_int --> max_int */
	if ((isinf(fvalue) && fvalue > 0.0f) || fvalue >= INT_MAX)
		value.as_uint = INT_MAX;
	/* -inf, < -max_int --> -max_int */
	else if (isinf(fvalue) || fvalue < INT_MIN)
		value.as_uint = INT_MIN;
	/* NaN, 0, -0 --> 0 */
	else if (isnan(fvalue) || fvalue == 0.0f || fvalue == -0.0f)
		value.as_uint = 0;
	else
		value.as_uint = (int) fvalue;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, value.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst,
			value.as_int);
	}
}
#undef INST

/* D.f = (float)S0.d. */
#define INST SI_INST_VOP1
void si_isa_V_CVT_F32_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union
	{
		double as_double;
		unsigned int as_reg[2];

	} s0;
	union si_reg_t value;

	/* Load operand from registers. */
	s0.as_reg[0] = si_isa_read_reg(work_item, INST.src0);
	s0.as_reg[1] = si_isa_read_reg(work_item, INST.src0 + 1);

	/* Cast to a single precision float */
	value.as_float = (float) s0.as_double;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, value.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			value.as_float);
	}
}
#undef INST

/* D.d = (double)S0.f. */
#define INST SI_INST_VOP1
void si_isa_V_CVT_F64_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union
	{
		double as_double;
		unsigned int as_reg[2];

	} value;
	union si_reg_t value_lo;
	union si_reg_t value_hi;

	/* Load operand from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);

	/* Cast to a single precision float */
	value.as_double = (double) s0.as_float;

	/* Write the results. */
	value_lo.as_uint = value.as_reg[0];
	value_hi.as_uint = value.as_reg[1];
	si_isa_write_vreg(work_item, INST.vdst, value_lo.as_uint);
	si_isa_write_vreg(work_item, INST.vdst + 1, value_hi.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V[%u:+1]<=(%lgf) ", work_item->id,
			INST.vdst, value.as_double);
	}
}
#undef INST

/* D.d = (double)S0.u. */
#define INST SI_INST_VOP1
void si_isa_V_CVT_F64_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.f = trunc(S0.f), return integer part of S0. */
#define INST SI_INST_VOP1
void si_isa_V_TRUNC_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.f = trunc(S0); if ((S0 < 0.0) && (S0 != D)) D += -1.0. */
#define INST SI_INST_VOP1
void si_isa_V_FLOOR_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.f = log2(S0.f). */
#define INST SI_INST_VOP1
void si_isa_V_LOG_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.f = 1.0 / S0.f. */
#define INST SI_INST_VOP1
void si_isa_V_RCP_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t rcp;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		rcp.as_float =
			1.0 / ((union si_reg_t) INST.lit_cnst).as_float;
	else
		rcp.as_float =
			1.0 / ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, rcp.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			rcp.as_float);
	}
}
#undef INST

/* D.d = 1.0 / (S0.d). */
#define INST SI_INST_VOP1
void si_isa_V_RCP_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.f = 1.0 / sqrt(S0.f). */
#define INST SI_INST_VOP1
void si_isa_V_RSQ_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.f = sqrt(S0.f). */
#define INST SI_INST_VOP1
void si_isa_V_SQRT_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t srt;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		srt.as_float =
			sqrt(((union si_reg_t) INST.lit_cnst).as_float);
	else
		srt.as_float =
			sqrt(((union si_reg_t) si_isa_read_reg(work_item,
					INST.src0)).as_float);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, srt.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			srt.as_float);
	}
}
#undef INST

/* D.f = sin(S0.f) */
#define INST SI_INST_VOP1
void si_isa_V_SIN_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t result;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);

	/* Normalize input */
	s0.as_float = s0.as_float / (2 * M_PI);

	if (IN_RANGE(s0.as_float, -256, 256))
	{
		result.as_float = sinf(s0.as_float);
	}
	else
	{
		result.as_float = 0;
	}

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			result.as_float);
	}
}
#undef INST

/* D.f = cos(S0.f) */
#define INST SI_INST_VOP1
void si_isa_V_COS_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t result;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);

	/* Normalize input */
	s0.as_float = s0.as_float / (2 * M_PI);

	if (IN_RANGE(s0.as_float, -256, 256))
	{
		result.as_float = cosf(s0.as_float);
	}
	else
	{
		result.as_float = 1;
	}

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			result.as_float);
	}
}
#undef INST

/* D.f = sqrt(S0.f). */
#define INST SI_INST_VOP1
void si_isa_V_NOT_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t result;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		result.as_uint = INST.lit_cnst;
	else
		result.as_uint = si_isa_read_reg(work_item, INST.src0);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = position of first 1 in S0 from MSB; D=0xFFFFFFFF if S0==0. */
#define INST SI_INST_VOP1
void si_isa_V_FFBH_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}

/* D.d = FRAC64(S0.d); */
#define INST SI_INST_VOP1
void si_isa_V_FRACT_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* VGPR[D.u + M0.u] = VGPR[S0.u]. */
#define INST SI_INST_VOP1
void si_isa_V_MOVRELD_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* VGPR[D.u] = VGPR[S0.u + M0.u]. */
#define INST SI_INST_VOP1
void si_isa_V_MOVRELS_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = VCC[i] ? S1.u : S0.u (i = threadID in wave); VOP3: specify VCC as a
 * scalar GPR in S2. */
#define INST SI_INST_VOP2
void si_isa_V_CNDMASK_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	int vcci;

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);
	vcci = si_isa_read_bitmask_sreg(work_item, SI_VCC);

	/* Calculate the result. */
	result = (vcci) ? s1 : s0;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.f = S0.f + S1.f. */
#define INST SI_INST_VOP2
void si_isa_V_ADD_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t sum;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = ((union si_reg_t) INST.lit_cnst).as_float;
	else
		s0 = ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vsrc1)).as_float;

	/* Calculate the sum. */
	sum.as_float = s0 + s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, sum.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			sum.as_float);
	}
}
#undef INST

/* D.f = S0.f - S1.f. */
#define INST SI_INST_VOP2
void si_isa_V_SUB_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t dif;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = ((union si_reg_t) INST.lit_cnst).as_float;
	else
		s0 = ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vsrc1)).as_float;

	/* Calculate the difference. */
	dif.as_float = s0 - s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, dif.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			dif.as_float);
	}
}
#undef INST

/* D.f = S1.f - S0.f. */
#define INST SI_INST_VOP2
void si_isa_V_SUBREV_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t dif;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = ((union si_reg_t) INST.lit_cnst).as_float;
	else
		s0 = ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vsrc1)).as_float;

	/* Calculate the difference. */
	dif.as_float = s1 - s0;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, dif.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			dif.as_float);
	}
}
#undef INST

/* D.f = S0.F * S1.f + D.f. */
#define INST SI_INST_VOP2
void si_isa_V_MAC_LEGACY_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST


/* D.f = S0.f * S1.f (DX9 rules, 0.0*x = 0.0). */
#define INST SI_INST_VOP2
void si_isa_V_MUL_LEGACY_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t product;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = ((union si_reg_t) INST.lit_cnst).as_float;
	else
		s0 = ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vsrc1)).as_float;

	/* Calculate the product. */
	if (s0 == 0 || s1 == 0)
	{
		product.as_float = 0.0;
	}
	else
	{
		product.as_float = s0 * s1;
	}

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, product.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			product.as_float);
	}
}
#undef INST

/* D.f = S0.f * S1.f. */
#define INST SI_INST_VOP2
void si_isa_V_MUL_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t product;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = ((union si_reg_t) INST.lit_cnst).as_float;
	else
		s0 = ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vsrc1)).as_float;

	/* Calculate the product. */
	product.as_float = s0 * s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, product.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			product.as_float);
	}
}
#undef INST

/* D.i = S0.i[23:0] * S1.i[23:0]. */
#define INST SI_INST_VOP2
void si_isa_V_MUL_I32_I24_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t product;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Truncate operands to 24-bit signed integers */
	s0 = SEXT32(s0, 24);
	s1 = SEXT32(s1, 24);

	/* Calculate the product. */
	product.as_int = s0 * s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, product.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d)", work_item->id, INST.vdst,
			product.as_int);
	}
}
#undef INST

/* D.f = min(S0.f, S1.f). */
#define INST SI_INST_VOP2
void si_isa_V_MIN_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t min;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = ((union si_reg_t) INST.lit_cnst).as_float;
	else
		s0 = ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vsrc1)).as_float;

	/* Calculate the minimum operand. */
	if (s0 < s1)
	{
		min.as_float = s0;
	}
	else
	{
		min.as_float = s1;
	}

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, min.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf)", work_item->id, INST.vdst,
			min.as_float);
	}
}
#undef INST

/* D.f = max(S0.f, S1.f). */
#define INST SI_INST_VOP2
void si_isa_V_MAX_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t max;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = ((union si_reg_t) INST.lit_cnst).as_float;
	else
		s0 = ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vsrc1)).as_float;

	/* Calculate the minimum operand. */
	if (s0 > s1)
	{
		max.as_float = s0;
	}
	else
	{
		max.as_float = s1;
	}

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, max.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf)", work_item->id, INST.vdst,
			max.as_float);
	}
}
#undef INST

/* D.i = max(S0.i, S1.i). */
#define INST SI_INST_VOP2
void si_isa_V_MAX_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t max;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the minimum operand. */
	if (s0 > s1)
	{
		max.as_int = s0;
	}
	else
	{
		max.as_int = s1;
	}

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, max.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d)", work_item->id, INST.vdst,
			max.as_int);
	}
}
#undef INST

/* D.u = min(S0.u, S1.u). */
#define INST SI_INST_VOP2
void si_isa_V_MIN_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t min;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the minimum operand. */
	if (s0 < s1)
	{
		min.as_uint = s0;
	}
	else
	{
		min.as_uint = s1;
	}

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, min.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u)", work_item->id, INST.vdst,
			min.as_uint);
	}
}
#undef INST

/* D.u = max(S0.u, S1.u). */
#define INST SI_INST_VOP2
void si_isa_V_MAX_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t max;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the minimum operand. */
	if (s0 > s1)
	{
		max.as_uint = s0;
	}
	else
	{
		max.as_uint = s1;
	}

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, max.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u)", work_item->id, INST.vdst,
			max.as_uint);
	}
}
#undef INST

/* D.u = S1.u >> S0.u[4:0]. */
#define INST SI_INST_VOP2
void si_isa_V_LSHRREV_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst & 0x1F;
	else
		s0 = si_isa_read_reg(work_item, INST.src0) & 0x1F;
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Right shift s1 by s0. */
	result.as_uint = s1 >> s0;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.i = S1.i >> S0.i[4:0]. */
#define INST SI_INST_VOP2
void si_isa_V_ASHRREV_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst & 0x1F;
	else
		s0 = si_isa_read_reg(work_item, INST.src0) & 0x1F;
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Right shift s1 by s0. */
	result.as_int = s1 >> s0;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst,
			result.as_int);
	}
}
#undef INST

/* D.u = S0.u << S1.u[4:0]. */
#define INST SI_INST_VOP2
void si_isa_V_LSHL_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1) & 0x1F;

	/* Left shift s1 by s0. */
	result.as_uint = s0 << s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = S1.u << S0.u[4:0]. */
#define INST SI_INST_VOP2
void si_isa_V_LSHLREV_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst & 0x1F;
	else
		s0 = si_isa_read_reg(work_item, INST.src0) & 0x1F;
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Left shift s1 by s0. */
	result.as_uint = s1 << s0;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = S0.u & S1.u. */
#define INST SI_INST_VOP2
void si_isa_V_AND_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Bitwise OR the two operands. */
	result.as_uint = s0 & s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = S0.u | S1.u. */
#define INST SI_INST_VOP2
void si_isa_V_OR_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Bitwise OR the two operands. */
	result.as_uint = s0 | s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = S0.u ^ S1.u. */
#define INST SI_INST_VOP2
void si_isa_V_XOR_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Bitwise OR the two operands. */
	result.as_uint = s0 ^ s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.f = S0.f * S1.f + D.f. */
#define INST SI_INST_VOP2
void si_isa_V_MAC_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;
	float d = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = ((union si_reg_t) INST.lit_cnst).as_float;
	else
		s0 = ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vsrc1)).as_float;
	d = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vdst)).as_float;

	/* Calculate the result. */
	result.as_float = s0 * s1 + d;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			result.as_float);
	}
}
#undef INST

/* D.u = S0.u + S1.u, vcc = carry-out. */
#define INST SI_INST_VOP2
void si_isa_V_ADD_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t sum;
	union si_reg_t carry;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the sum and carry. */
	sum.as_uint = s0 + s1;
	carry.as_uint =
		! !(((unsigned long long) s0 +
			(unsigned long long) s1) >> 32);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, sum.as_uint);
	si_isa_bitmask_sreg(work_item, SI_VCC, carry.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst,
			sum.as_int);
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, carry.as_uint);
	}
}
#undef INST

/* D.u = S0.u - S1.u; vcc = carry-out. */
#define INST SI_INST_VOP2
void si_isa_V_SUB_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t dif;
	union si_reg_t carry;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the difference and carry. */
	dif.as_uint = s0 - s1;
	carry.as_uint = (s1 > s0);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, dif.as_uint);
	si_isa_bitmask_sreg(work_item, SI_VCC, carry.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst,
			dif.as_int);
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, carry.as_uint);
	}
}
#undef INST

/* D.u = S1.u - S0.u; vcc = carry-out. */
#define INST SI_INST_VOP2
void si_isa_V_SUBREV_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t dif;
	union si_reg_t carry;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the difference and carry. */
	dif.as_uint = s1 - s0;
	carry.as_uint = (s0 > s1);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, dif.as_uint);
	si_isa_bitmask_sreg(work_item, SI_VCC, carry.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst,
			dif.as_int);
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, carry.as_uint);
	}
}
#undef INST

/* D = {flt32_to_flt16(S1.f),flt32_to_flt16(S0.f)}, with round-toward-zero. */
#define INST SI_INST_VOP2
void si_isa_V_CVT_PKRTZ_F16_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = VCC[i] ? S1.u : S0.u (i = threadID in wave); VOP3: specify VCC as a
 * scalar GPR in S2. */
#define INST SI_INST_VOP3a
void si_isa_V_CNDMASK_B32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	int vcci;

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);
	vcci = si_isa_read_bitmask_sreg(work_item, INST.src2);

	/* Calculate the result. */
	result = (vcci) ? s1 : s0;

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.f = S0.f + S1.f. */
#define INST SI_INST_VOP3a
void si_isa_V_ADD_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t sum;

	/* Load operands from registers or as a literal constant. */
	s0 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src1)).as_float;

	/* Calculate the sum. */
	sum.as_float = s0 + s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, sum.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			sum.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP3a
void si_isa_V_SUBREV_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t diff;

	/* Load operands from registers or as a literal constant. */
	s0 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src1)).as_float;

	/* Calculate the diff. */
	diff.as_float = s1 - s0;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, diff.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			diff.as_float);
	}
}
#undef INST

/* D.f = S0.f * S1.f. */
#define INST SI_INST_VOP3a
void si_isa_V_MUL_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0;
	float s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src1)).as_float;

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Calculate the result. */
	result.as_float = s0 * s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			result.as_float);
	}
}
#undef INST

/* D.f = S0. * S1.. */
#define INST SI_INST_VOP3a
void si_isa_V_MUL_I32_I24_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t product;

	/* Load operands from registers or as a literal constant. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Truncate operands to 24-bit signed integers */
	s0 = SEXT32(s0, 24);
	s1 = SEXT32(s1, 24);

	/* Apply absolute value and negation modifiers */
	if (INST.abs & 1)
		s0 = abs(s0);
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.abs & 2)
		s1 = abs(s1);
	if (INST.neg & 2)
		s1 = -s1;

	/* Calculate the product. */
	product.as_int = s0 * s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, product.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d)", work_item->id, INST.vdst,
			product.as_int);
	}
}
#undef INST

/* D.f = max(S0.f, S1.f). */
#define INST SI_INST_VOP3a
void si_isa_V_MAX_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0;
	float s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src1)).as_float;

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Calculate the result. */
	result.as_float = (s0 >= s1) ? s0 : s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			result.as_float);
	}
}
#undef INST

/* D.f = S0.f * S1.f + S2.f. */
#define INST SI_INST_VOP3a
void si_isa_V_MAD_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0;
	float s1;
	float s2;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src1)).as_float;
	s2 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src2)).as_float;

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;
	if (INST.neg & 4)
		s2 = -s2;

	/* Calculate the result. */
	result.as_float = s0 * s1 + s2;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			result.as_float);
	}
}
#undef INST

/* D.u = S0.u[23:0] * S1.u[23:0] + S2.u[31:0]. */
#define INST SI_INST_VOP3a
void si_isa_V_MAD_U32_U24_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = (S0.u >> S1.u[4:0]) & ((1 << S2.u[4:0]) - 1); bitfield extract,
 * S0=data, S1=field_offset, S2=field_width. */
#define INST SI_INST_VOP3a
void si_isa_V_BFE_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	unsigned int s1;
	unsigned int s2;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1) & 0x1F;
	s2 = si_isa_read_reg(work_item, INST.src2) & 0x1F;

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;
	if (INST.neg & 4)
		s2 = -s2;

	/* Calculate the result. */
	result.as_uint = (s0 >> s1) & ((1 << s2) - 1);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: value:s0:%u, offset:s1:%u, width:s2:%u ",
			work_item->id, s0, s1, s2);
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.i = (S0.i >> S1.i[4:0]) & ((1 << S2.i[4:0]) - 1); bitfield extract,
 * S0=data, S1=field_offset, S2=field_width. */
#define INST SI_INST_VOP3a
void si_isa_V_BFE_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0;
	int s1;
	int s2;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1) & 0x1F;
	s2 = si_isa_read_reg(work_item, INST.src2) & 0x1F;

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;
	if (INST.neg & 4)
		s2 = -s2;

	/* Calculate the result. */
	result.as_int = (s0 >> s1) & ((1 << s2) - 1);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst,
			result.as_int);
	}
}
#undef INST

/* D.u = (S0.u & S1.u) | (~S0.u & S2.u). */
#define INST SI_INST_VOP3a
void si_isa_V_BFI_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	unsigned int s1;
	unsigned int s2;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);
	s2 = si_isa_read_reg(work_item, INST.src2);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;
	if (INST.neg & 4)
		s2 = -s2;

	/* Calculate the result. */
	result.as_uint = (s0 & s1) | (~s0 & s2);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.f = S0.f * S1.f + S2.f */
#define INST SI_INST_VOP3a
void si_isa_V_FMA_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.d = S0.d * S1.d + S2.d */
#define INST SI_INST_VOP3a
void si_isa_V_FMA_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* 0,S1} >> S2.u[4:0]) & 0xFFFFFFFF. */
#define INST SI_INST_VOP3a
void si_isa_V_ALIGNBIT_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* 
 *D.d = Special case divide fixup and flags(s0.d = Quotient, s1.d = Denominator, s2.d = Numerator).
 */
#define INST SI_INST_VOP3a
void si_isa_V_DIV_FIXUP_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.d = min(S0.d, S1.d). */
#define INST SI_INST_VOP3a
void si_isa_V_MIN_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.d = max(S0.d, S1.d). */
#define INST SI_INST_VOP3a
void si_isa_V_MAX_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = S0.u * S1.u. */
#define INST SI_INST_VOP3a
void si_isa_V_MUL_LO_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	unsigned int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Calculate the product. */
	result.as_uint = s0 * s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* 
 *D.d = Special case divide FMA with scale and flags(s0.d = Quotient, s1.d = Denominator,
 *s2.d = Numerator).
 */
#define INST SI_INST_VOP3a
void si_isa_V_DIV_FMAS_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.d = Look Up 2/PI (S0.d) with segment select S1.u[4:0]. */
#define INST SI_INST_VOP3a
void si_isa_V_TRIG_PREOP_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = (S0.u * S1.u)>>32 */
#define INST SI_INST_VOP3a
void si_isa_V_MUL_HI_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	unsigned int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Calculate the product and shift right. */
	result.as_uint =
		((unsigned long long) s0 * (unsigned long long) s1) >> 32;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u) ", work_item->id, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.i = S0.i * S1.i. */
#define INST SI_INST_VOP3a
void si_isa_V_MUL_LO_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0;
	int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Calculate the product. */
	result.as_int = s0 * s1;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst,
			result.as_int);
	}
}
#undef INST

/* D.f = S0.f - floor(S0.f). */
#define INST SI_INST_VOP3a
void si_isa_V_FRACT_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t result;

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);

	/* Apply negation modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);

	if (INST.neg & 1)
		s0.as_float = -s0.as_float;

	/* Calculate the product. */
	result.as_float = s0.as_float - floorf(s0.as_float);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst,
			result.as_float);
	}
}
#undef INST

/* vcc = (S0.f < S1.f). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_LT_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = ((union si_reg_t) INST.lit_cnst).as_float;
	else
		s0 = ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vsrc1)).as_float;

	/* Compare the operands. */
	result.as_uint = (s0 < s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.f > S1.f). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_GT_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = ((union si_reg_t) INST.lit_cnst).as_float;
	else
		s0 = ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vsrc1)).as_float;

	/* Compare the operands. */
	result.as_uint = (s0 > s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.f >= S1.f). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_GE_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* vcc = !(S0.f > S1.f). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_NGT_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = ((union si_reg_t) INST.lit_cnst).as_float;
	else
		s0 = ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vsrc1)).as_float;

	/* Compare the operands. */
	result.as_uint = !(s0 > s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = !(S0.f == S1.f). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_NEQ_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0 = 0;
	float s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = ((union si_reg_t) INST.lit_cnst).as_float;
	else
		s0 = ((union si_reg_t) si_isa_read_reg(work_item,
				INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.vsrc1)).as_float;

	/* Compare the operands. */
	result.as_uint = !(s0 == s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.d < S1.d). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_LT_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* vcc = (S0.d == S1.d). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_EQ_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* vcc = (S0.d <= S1.d). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_LE_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* vcc = (S0.d > S1.d). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_GT_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* vcc = !(S0.d >= S1.d). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_NGE_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* vcc = !(S0.d == S1.d). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_NEQ_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* vcc = (S0.i < S1.i). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_LT_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0 < s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.i == S1.i). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_EQ_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0 == s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.i > S1.i). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_GT_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0 > s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.i <> S1.i). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_NE_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0 = 0;
	int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0 != s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* D.u = (S0.i >= S1.i). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_GE_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0;
	int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0 >= s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* D = IEEE numeric class function specified in S1.u, performed on S0.d. */
#define INST SI_INST_VOPC
void si_isa_V_CMP_CLASS_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* vcc = (S0.u < S1.u). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_LT_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0 < s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.u == S1.u). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_EQ_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}

/* vcc = (S0.u <= S1.u). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_LE_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0 <= s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.u > S1.u). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_GT_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0 = 0;
	unsigned int s1 = 0;

	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0 > s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* D.u = (S0.f < S1.f). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_NE_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = (S0.f < S1.f). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_LT_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0;
	float s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src1)).as_float;

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = (s0 < s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.f == S1.f). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_EQ_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0;
	float s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src1)).as_float;

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = (s0 == s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* vcc = (S0.f <= S1.f). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_LE_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}

/* D.u = (S0.f > S1.f). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_GT_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0;
	float s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src1)).as_float;

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = (s0 > s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = !(S0.f <= S1.f). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_NLE_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = !(S0.f == S1.f). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_NEQ_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0;
	float s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src1)).as_float;

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = !(s0 == s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = !(S0.f < S1.f). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_NLT_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	float s0;
	float s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src0)).as_float;
	s1 = ((union si_reg_t) si_isa_read_reg(work_item,
			INST.src1)).as_float;

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = !(s0 < s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* Comparison Operations */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_OP16_F64_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = (S0 < S1) */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_LT_F64_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = (S0 == S1) */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_EQ_F64_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = (S0 <= S1) */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_LE_F64_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = (S0 > S1) */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_GT_F64_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = (S0 >= S1) */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_GE_F64_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = !(S0 < S1) */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_NLT_F64_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = (S0.i < S1.i). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_LT_I32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0;
	int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = (s0 < s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.i == S1.i). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_EQ_I32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0;
	int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = (s0 == s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.i <= S1.i). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_LE_I32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0;
	int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = (s0 <= s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.i > S1.i). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_GT_I32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0;
	int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = (s0 > s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.i <> S1.i). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_NE_I32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0;
	int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = !(s0 == s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.i >= S1.i). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_GE_I32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0;
	int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = (s0 >= s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.i >= S1.i). Also write EXEC */
#define INST_SI_INST_VOP3a
void si_isa_V_CMPX_EQ_I32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D = IEEE numeric class function specified in S1.u, performed on S0.d. */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_CLASS_F64_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = (S0.u < S1.u). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_LT_U32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	unsigned int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = (s0 < s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.u <= S1.u). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_LE_U32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	unsigned int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = (s0 <= s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.u > S1.u). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_GT_U32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	unsigned int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = (s0 > s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP3a
void si_isa_V_CMP_LG_U32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	unsigned int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;
	assert(!(INST.neg & 4));

	/* Calculate result. */
	result.as_uint = (s0 != s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.u >= S1.u). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_GE_U32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	unsigned int s1;

	union si_reg_t result;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;

	/* Compare the operands. */
	result.as_uint = (s0 >= s1);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0 < S1) */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_LT_U64_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* Median of three numbers. */
#define INST SI_INST_VOP3a
void si_isa_V_MED3_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	int s0;
	int s1;
	int s2;

	union si_reg_t median;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);
	s2 = si_isa_read_reg(work_item, INST.src2);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;
	if (INST.neg & 4)
		s2 = -s2;

	/* Determine the median. */
	if (isnan(s0) || isnan(s1) || isnan(s2))
		/* min3(s0, s1, s2) */
		median.as_int =
			(s0 < s1) ? ((s2 < s0) ? s2 : s0) : ((s2 <
				s1) ? s2 : s1);
	/* max3(s0, s1, s2) == s0 */
	else if (s0 > s1 && s0 > s2)
		/* max(s1, s2) */
		median.as_int = (s1 > s2) ? s1 : s2;
	/* max3(s0, s1, s2) == s1 */
	else if (s1 > s0 && s1 > s2)
		/* max(s0, s2) */
		median.as_int = (s0 > s2) ? s0 : s2;
	/* max3(s0, s1, s2) == s2 */
	else
		/* max(s0, s1) */
		median.as_int = (s0 > s1) ? s0 : s1;


	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, median.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: V[%u]<=(%d) ",
			work_item->id_in_wavefront, INST.vdst, median.as_int);
	}
}
#undef INST

/* D = S0.u >> S1.u[4:0]. */
#define INST SI_INST_VOP3a
void si_isa_V_LSHR_B64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union
	{
		unsigned long long as_b64;
		unsigned int as_reg[2];

	} s0, value;
	unsigned int s1;

	union si_reg_t result_lo;
	union si_reg_t result_hi;

	/* Load operands from registers. */
	s0.as_reg[0] = si_isa_read_reg(work_item, INST.src0);
	s0.as_reg[1] = si_isa_read_reg(work_item, INST.src0 + 1);
	s1 = si_isa_read_reg(work_item, INST.src1) & 0x1F;

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_b64 = -s0.as_b64;
	if (INST.neg & 2)
		s1 = -s1;

	/* Shift s0. */
	value.as_b64 = s0.as_b64 >> s1;

	/* Write the results. */
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	si_isa_write_vreg(work_item, INST.vdst, result_lo.as_uint);
	si_isa_write_vreg(work_item, INST.vdst + 1, result_hi.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u]<=(0x%x) ",
			work_item->id_in_wavefront, INST.vdst,
			result_lo.as_uint);
		si_isa_debug("wf_id%d: S[%u]<=(0x%x) ",
			work_item->id_in_wavefront, INST.vdst + 1,
			result_hi.as_uint);
	}
}
#undef INST

/* D.d = S0.d + S1.d. */
#define INST SI_INST_VOP3a
void si_isa_V_ADD_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union
	{
		double as_double;
		unsigned int as_reg[2];

	} s0, s1, value;

	union si_reg_t result_lo;
	union si_reg_t result_hi;

	/* Load operands from registers. */
	s0.as_reg[0] = si_isa_read_reg(work_item, INST.src0);
	s0.as_reg[1] = si_isa_read_reg(work_item, INST.src0 + 1);
	s1.as_reg[0] = si_isa_read_reg(work_item, INST.src1);
	s1.as_reg[1] = si_isa_read_reg(work_item, INST.src1 + 1);

	/* Add the operands, take into account special number cases. */

	/* s0 == NaN64 || s1 == NaN64 */
	if (fpclassify(s0.as_double) == FP_NAN ||
		fpclassify(s1.as_double) == FP_NAN)
		/* value <-- NaN64 */
		value.as_double = NAN;
	/* s0,s1 == infinity */
	else if (fpclassify(s0.as_double) == FP_INFINITE &&
		fpclassify(s1.as_double) == FP_INFINITE)
		/* value <-- NaN64 */
		value.as_double = NAN;
	/* s0,!s1 == infinity */
	else if (fpclassify(s0.as_double) == FP_INFINITE)
		/* value <-- s0(+-infinity) */
		value.as_double = s0.as_double;
	/* s1,!s0 == infinity */
	else if (fpclassify(s1.as_double) == FP_INFINITE)
		/* value <-- s1(+-infinity) */
		value.as_double = s1.as_double;
	/* s0 == +-denormal, +-0 */
	else if (fpclassify(s0.as_double) == FP_SUBNORMAL ||
		fpclassify(s0.as_double) == FP_ZERO)
	{
		/* s1 == +-denormal, +-0 */
		if (fpclassify(s1.as_double) == FP_SUBNORMAL ||
			fpclassify(s1.as_double) == FP_ZERO)
			/* s0 && s1 == -denormal, -0 */
			if (! !signbit(s0.as_double)
				&& ! !signbit(s1.as_double))
				/* value <-- -0 */
				value.as_double = -0;
			else
				/* value <-- +0 */
				value.as_double = +0;
		/* s1 == F */
		else
			/* value <-- s1 */
			value.as_double = s1.as_double;
	}
	/* s1 == +-denormal, +-0 */
	else if (fpclassify(s1.as_double) == FP_SUBNORMAL ||
		fpclassify(s1.as_double) == FP_ZERO)
	{
		/* s0 == +-denormal, +-0 */
		if (fpclassify(s0.as_double) == FP_SUBNORMAL ||
			fpclassify(s0.as_double) == FP_ZERO)
			/* s0 && s1 == -denormal, -0 */
			if (! !signbit(s0.as_double)
				&& ! !signbit(s1.as_double))
				/* value <-- -0 */
				value.as_double = -0;
			else
				/* value <-- +0 */
				value.as_double = +0;
		/* s0 == F */
		else
			/* value <-- s1 */
			value.as_double = s0.as_double;
	}
	/* s0 && s1 == F */
	else
		value.as_double = s0.as_double + s1.as_double;

	/* Write the results. */
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	si_isa_write_vreg(work_item, INST.vdst, result_lo.as_uint);
	si_isa_write_vreg(work_item, INST.vdst + 1, result_hi.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%lgf) ",
			work_item->id_in_wavefront, INST.vdst,
			value.as_double);
	}
}
#undef INST

/* D.d = S0.d * S1.d. */
#define INST SI_INST_VOP3a
void si_isa_V_MUL_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union
	{
		double as_double;
		unsigned int as_reg[2];

	} s0, s1, value;

	union si_reg_t result_lo;
	union si_reg_t result_hi;

	/* Load operands from registers. */
	s0.as_reg[0] = si_isa_read_reg(work_item, INST.src0);
	s0.as_reg[1] = si_isa_read_reg(work_item, INST.src0 + 1);
	s1.as_reg[0] = si_isa_read_reg(work_item, INST.src1);
	s1.as_reg[1] = si_isa_read_reg(work_item, INST.src1 + 1);

	/* Multiply the operands, take into account special number cases. */

	/* s0 == NaN64 || s1 == NaN64 */
	if (fpclassify(s0.as_double) == FP_NAN ||
		fpclassify(s1.as_double) == FP_NAN)
		/* value <-- NaN64 */
		value.as_double = NAN;
	/* s0 == +denormal, +0 */
	else if ((fpclassify(s1.as_double) == FP_SUBNORMAL ||
			fpclassify(s1.as_double) == FP_ZERO) &&
		!signbit(s0.as_double))
	{
		/* s1 == +-infinity */
		if (isinf(s1.as_double))
			/* value <-- NaN64 */
			value.as_double = NAN;
		/* s1 > 0 */
		else if (!signbit(s1.as_double))
			/* value <-- +0 */
			value.as_double = +0;
		/* s1 < 0 */
		else if (! !signbit(s1.as_double))
			/* value <-- -0 */
			value.as_double = -0;
	}
	/* s0 == -denormal, -0 */
	else if ((fpclassify(s1.as_double) == FP_SUBNORMAL ||
			fpclassify(s1.as_double) == FP_ZERO) &&
		! !signbit(s0.as_double))
	{
		/* s1 == +-infinity */
		if (isinf(s1.as_double))
			/* value <-- NaN64 */
			value.as_double = NAN;
		/* s1 > 0 */
		else if (!signbit(s1.as_double))
			/* value <-- -0 */
			value.as_double = -0;
		/* s1 < 0 */
		else if (! !signbit(s1.as_double))
			/* value <-- +0 */
			value.as_double = +0;
	}
	/* s0 == +infinity */
	else if (fpclassify(s0.as_double) == FP_INFINITE &&
		!signbit(s0.as_double))
	{
		/* s1 == +-denormal, +-0 */
		if (fpclassify(s1.as_double) == FP_SUBNORMAL ||
			fpclassify(s1.as_double) == FP_ZERO)
			/* value <-- NaN64 */
			value.as_double = NAN;
		/* s1 > 0 */
		else if (!signbit(s1.as_double))
			/* value <-- +infinity */
			value.as_double = +INFINITY;
		/* s1 < 0 */
		else if (! !signbit(s1.as_double))
			/* value <-- -infinity */
			value.as_double = -INFINITY;
	}
	/* s0 == -infinity */
	else if (fpclassify(s0.as_double) == FP_INFINITE &&
		! !signbit(s0.as_double))
	{
		/* s1 == +-denormal, +-0 */
		if (fpclassify(s1.as_double) == FP_SUBNORMAL ||
			fpclassify(s1.as_double) == FP_ZERO)
			/* value <-- NaN64 */
			value.as_double = NAN;
		/* s1 > 0 */
		else if (!signbit(s1.as_double))
			/* value <-- -infinity */
			value.as_double = -INFINITY;
		/* s1 < 0 */
		else if (! !signbit(s1.as_double))
			/* value <-- +infinity */
			value.as_double = +INFINITY;
	}
	else
		value.as_double = s0.as_double * s1.as_double;

	/* Write the results. */
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	si_isa_write_vreg(work_item, INST.vdst, result_lo.as_uint);
	si_isa_write_vreg(work_item, INST.vdst + 1, result_hi.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%u:+1]<=(%lgf) ",
			work_item->id_in_wavefront, INST.vdst,
			value.as_double);
	}
}
#undef INST

/* D.d = Look Up 2/PI (S0.d) with segment select S1.u[4:0]. */
#define INST SI_INST_VOP3a
void si_isa_V_LDEXP_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.u = S0.u + S1.u + VCC; VCC=carry-out (VOP3:sgpr=carry-out,
 * S2.u=carry-in). */
#define INST SI_INST_VOP3b
void si_isa_V_ADDC_U32_VOP3b_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int s0;
	unsigned int s1;
	unsigned int carry_in;

	union si_reg_t sum;
	union si_reg_t carry_out;

	/* Load operands from registers. */
	s0 = si_isa_read_reg(work_item, INST.src0);
	s1 = si_isa_read_reg(work_item, INST.src1);
	carry_in = si_isa_read_bitmask_sreg(work_item, INST.src2);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0 = -s0;
	if (INST.neg & 2)
		s1 = -s1;
	assert(!(INST.neg & 4));

	/* Calculate sum and carry. */
	sum.as_uint = s0 + s1 + carry_in;
	carry_out.as_uint =
		! !(((unsigned long long) s0 + (unsigned long long) s1 +
			(unsigned long long) carry_in) >> 32);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, sum.as_uint);
	si_isa_bitmask_sreg(work_item, INST.sdst, carry_out.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u) ", work_item->id, INST.vdst,
			sum.as_uint);
		si_isa_debug("wf_id%d: vcc<=(%u) ",
			work_item->id_in_wavefront, carry_out.as_uint);
	}
}
#undef INST

/* 
 *D.d = Special case divide preop and flags(s0.d = Quotient, s1.d = Denominator, s2.d = Numerator)
 *s0 must equal s1 or s2.
 */
#define INST SI_INST_VOP3b
void si_isa_V_DIV_SCALE_F64_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D = P10 * S + P0 */
#define INST SI_INST_VINTRP
void si_isa_V_INTERP_P1_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D = P20 * S + D */
#define INST SI_INST_VINTRP
void si_isa_V_INTERP_P2_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D = {P10,P20,P0}[S] */
#define INST SI_INST_VINTRP
void si_isa_V_INTERP_MOV_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* DS[A] = (DS[A] >= D0 ? 0 : DS[A] + 1); uint increment. */
#define INST SI_INST_DS
void si_isa_DS_INC_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* DS[A] = D0; write a Dword. */
#define INST SI_INST_DS
void si_isa_DS_WRITE_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int addr;
	unsigned int data0;

	union si_reg_t tmp;

	/* Load address and data from registers. */
	addr = si_isa_read_vreg(work_item, INST.addr);
	data0 = si_isa_read_vreg(work_item, INST.data0);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Write Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_write(work_item->work_group->lds_module, addr, 4, &data0);
	}

	/* Record last memory access for the detailed simulator. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		work_item->lds_access_count = 1;
		work_item->lds_access_type[0] = 2;
		work_item->lds_access_addr[0] = addr;
		work_item->lds_access_size[0] = 4;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category) && INST.gds)
	{
		tmp.as_uint = data0;
		si_isa_debug("t%d: GDS[%u]<=(%u,%f) ", work_item->id, addr,
			tmp.as_uint, tmp.as_float);
	}
	else
	{
		tmp.as_uint = data0;
		si_isa_debug("t%d: LDS[%u]<=(%u,%f) ", work_item->id, addr,
			tmp.as_uint, tmp.as_float);
	}
}
#undef INST

/* DS[A] = D0[7:0]; byte write.  */
#define INST SI_INST_DS
void si_isa_DS_WRITE_B8_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int addr;
	unsigned char data0;

	/* Load address and data from registers. */
	addr = si_isa_read_vreg(work_item, INST.addr);
	data0 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.data0)).as_ubyte[0];

	/* Global data store not supported */
	assert(!INST.gds);

	/* Write Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_write(work_item->work_group->lds_module, addr, 1, &data0);
	}

	/* Record last memory access for the detailed simulator. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		work_item->lds_access_count = 1;
		work_item->lds_access_type[0] = 2;
		work_item->lds_access_addr[0] = addr;
		work_item->lds_access_size[0] = 1;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category) && INST.gds)
	{
		si_isa_debug("t%d: GDS[%u]<=(0x%x) ", work_item->id, addr,
			data0);
	}
	else
	{
		si_isa_debug("t%d: LDS[%u]<=(0x%x) ", work_item->id, addr,
			data0);
	}
}
#undef INST

/* DS[A] = D0[15:0]; short write.  */
#define INST SI_INST_DS
void si_isa_DS_WRITE_B16_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int addr;
	unsigned short data0;

	/* Load address and data from registers. */
	addr = si_isa_read_vreg(work_item, INST.addr);
	data0 = ((union si_reg_t) si_isa_read_vreg(work_item,
			INST.data0)).as_ushort[0];

	/* Global data store not supported */
	assert(!INST.gds);

	/* Write Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_write(work_item->work_group->lds_module, addr, 2, &data0);
	}

	/* Record last memory access for the detailed simulator. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		work_item->lds_access_count = 1;
		work_item->lds_access_type[0] = 2;
		work_item->lds_access_addr[0] = addr;
		work_item->lds_access_size[0] = 2;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category) && INST.gds)
	{
		si_isa_debug("t%d: GDS[%u]<=(0x%x) ", work_item->id, addr,
			data0);
	}
	else
	{
		si_isa_debug("t%d: LDS[%u]<=(0x%x) ", work_item->id, addr,
			data0);
	}

}
#undef INST

/* R = DS[A]; Dword read. */
#define INST SI_INST_DS
void si_isa_DS_READ_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int addr;
	union si_reg_t data;

	/* Load address from register. */
	addr = si_isa_read_vreg(work_item, INST.addr);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Read Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_read(work_item->work_group->lds_module, addr, 4,
			&data.as_uint);
	}

	/* Write results. */
	si_isa_write_vreg(work_item, INST.vdst, data.as_uint);

	/* Record last memory access for the detailed simulator. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		work_item->lds_access_count = 1;
		work_item->lds_access_type[0] = 1;
		work_item->lds_access_addr[0] = addr;
		work_item->lds_access_size[0] = 4;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) ", work_item->id, INST.vdst,
			data.as_uint);
	}

}
#undef INST

/* R = signext(DS[A][7:0]}; signed byte read. */
#define INST SI_INST_DS
void si_isa_DS_READ_I8_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int addr;
	union si_reg_t data;

	/* Load address from register. */
	addr = si_isa_read_vreg(work_item, INST.addr);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Read Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_read(work_item->work_group->lds_module, addr, 1,
			&data.as_byte[0]);
	}

	/* Extend the sign. */
	data.as_int = (int) data.as_byte[0];

	/* Write results. */
	si_isa_write_vreg(work_item, INST.vdst, data.as_uint);

	/* Record last memory access for the detailed simulator. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		work_item->lds_access_count = 1;
		work_item->lds_access_type[0] = 1;
		work_item->lds_access_addr[0] = addr;
		work_item->lds_access_size[0] = 1;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst,
			data.as_int);
	}
}
#undef INST

/* R = {24’h0,DS[A][7:0]}; unsigned byte read. */
#define INST SI_INST_DS
void si_isa_DS_READ_U8_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int addr;
	union si_reg_t data;

	/* Load address from register. */
	addr = si_isa_read_vreg(work_item, INST.addr);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Read Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_read(work_item->work_group->lds_module, addr, 1,
			&data.as_ubyte[0]);
	}

	/* Make sure to use only bits [7:0]. */
	data.as_uint = (unsigned int) data.as_ubyte[0];

	/* Write results. */
	si_isa_write_vreg(work_item, INST.vdst, data.as_uint);

	/* Record last memory access for the detailed simulator. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		work_item->lds_access_count = 1;
		work_item->lds_access_type[0] = 1;
		work_item->lds_access_addr[0] = addr;
		work_item->lds_access_size[0] = 1;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u) ", work_item->id, INST.vdst,
			data.as_uint);
	}
}
#undef INST

/* R = signext(DS[A][15:0]}; signed short read. */
#define INST SI_INST_DS
void si_isa_DS_READ_I16_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int addr;
	union si_reg_t data;

	/* Load address from register. */
	addr = si_isa_read_vreg(work_item, INST.addr);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Read Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_read(work_item->work_group->lds_module, addr, 2,
			&data.as_short[0]);
	}

	/* Extend the sign. */
	data.as_int = (int) data.as_short[0];

	/* Write results. */
	si_isa_write_vreg(work_item, INST.vdst, data.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst,
			data.as_int);
	}

	/* Record last memory access for the detailed simulator. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		work_item->lds_access_count = 1;
		work_item->lds_access_type[0] = 1;
		work_item->lds_access_addr[0] = addr;
		work_item->lds_access_size[0] = 2;
	}
}
#undef INST

/* R = {16’h0,DS[A][15:0]}; unsigned short read. */
#define INST SI_INST_DS
void si_isa_DS_READ_U16_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	unsigned int addr;
	union si_reg_t data;

	/* Load address from register. */
	addr = si_isa_read_vreg(work_item, INST.addr);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Read Dword. */
	if (INST.gds)
	{
		mem_read(si_emu->global_mem, addr, 2, &data.as_ushort[0]);
	}
	else
	{
		mem_read(work_item->work_group->lds_module, addr, 2,
			&data.as_ushort[0]);
	}

	/* Make sure to use only bits [15:0]. */
	data.as_uint = (unsigned int) data.as_ushort[0];

	/* Write results. */
	si_isa_write_vreg(work_item, INST.vdst, data.as_uint);

	/* Record last memory access for the detailed simulator. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		work_item->lds_access_count = 1;
		work_item->lds_access_type[0] = 1;
		work_item->lds_access_addr[0] = addr;
		work_item->lds_access_size[0] = 2;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u) ", work_item->id, INST.vdst,
			data.as_uint);
	}
}
#undef INST

#define INST SI_INST_MUBUF
void si_isa_BUFFER_LOAD_SBYTE_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

#define INST SI_INST_MUBUF
void si_isa_BUFFER_ATOMIC_ADD_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_TBUFFER_LOAD_FORMAT_X_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);

	unsigned int offset;
	int elem_size;
	int num_elems;
	int bytes_to_read;
	int index;
	struct si_buffer_resource_t buf_desc;
	unsigned int buffer_addr;
	union si_reg_t value;

	if (INST.offen)
	{
		offset = si_isa_read_vreg(work_item, INST.vaddr) + INST.offset;

		if (INST.idxen)
			index = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}
	else
	{
		offset = INST.offset;

		if (INST.idxen)
			index = si_isa_read_vreg(work_item, INST.vaddr);
	}

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);

	/* If num_elems is greater than 1, we need to see how the
	 * destination register is handled */
	assert(num_elems == 1);

	bytes_to_read = elem_size * num_elems;

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	buffer_addr = offset;
	if (INST.idxen)
	{
		buffer_addr += buf_desc.stride * (index +
			work_item->id_in_wavefront);
	}

	mem_read(si_emu->global_mem, buffer_addr, bytes_to_read, &value);

	si_isa_write_vreg(work_item, INST.vdata, value.as_uint);

	/* TODO Print value based on type */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u)(%u,%gf) ", work_item->id,
			INST.vdata, buffer_addr, value.as_uint,
			value.as_float);
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = buffer_addr;
	work_item->global_mem_access_size = elem_size * num_elems;
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_TBUFFER_LOAD_FORMAT_XY_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);

	unsigned int offset;
	int elem_size;
	int num_elems;
	int index;
	struct si_buffer_resource_t buf_desc;
	unsigned int buffer_addr;
	union si_reg_t value;

	if (INST.offen)
	{
		offset = si_isa_read_vreg(work_item,
			INST.vaddr) + INST.offset;

		if (INST.idxen)
			index = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}
	else
	{
		offset = INST.offset;

		if (INST.idxen)
			index = si_isa_read_vreg(work_item, INST.vaddr);
	}

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);

	/* Four Dwords are read. */
	assert(num_elems == 2);
	assert(elem_size == 4);

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	for (unsigned int i = 0; i < 2; i++)
	{
		buffer_addr = offset + 4 * i;
		if (INST.idxen)
		{
			buffer_addr += buf_desc.stride * (index +
				work_item->id_in_wavefront);
		}

		mem_read(si_emu->global_mem, buffer_addr, 4, &value);

		si_isa_write_vreg(work_item, INST.vdata + i, value.as_uint);

		/* TODO Print value based on type */
		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("t%d: V%u<=(%u)(%u,%gf) ", work_item->id,
				INST.vdata + i, buffer_addr, value.as_uint,
				value.as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = buffer_addr;
	work_item->global_mem_access_size = elem_size * num_elems;
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_TBUFFER_LOAD_FORMAT_XYZW_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);

	unsigned int offset;
	int elem_size;
	int num_elems;
	int index;
	struct si_buffer_resource_t buf_desc;
	unsigned int buffer_addr;
	union si_reg_t value;

	if (INST.offen)
	{
		offset = si_isa_read_vreg(work_item,
			INST.vaddr) + INST.offset;

		if (INST.idxen)
			index = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}
	else
	{
		offset = INST.offset;

		if (INST.idxen)
			index = si_isa_read_vreg(work_item, INST.vaddr);
	}

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);

	/* Four Dwords are read. */
	assert(num_elems == 4);
	assert(elem_size == 4);

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	for (unsigned int i = 0; i < 4; i++)
	{
		buffer_addr = offset + 4 * i;
		if (INST.idxen)
		{
			buffer_addr += buf_desc.stride * (index +
				work_item->id_in_wavefront);
		}

		mem_read(si_emu->global_mem, buffer_addr, 4, &value);

		si_isa_write_vreg(work_item, INST.vdata + i, value.as_uint);

		/* TODO Print value based on type */
		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("t%d: V%u<=(%u)(%u,%gf) ", work_item->id,
				INST.vdata + i, buffer_addr, value.as_uint,
				value.as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = buffer_addr;
	work_item->global_mem_access_size = elem_size * num_elems;
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_TBUFFER_STORE_FORMAT_X_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);

	unsigned int offset;
	int elem_size;
	int num_elems;
	int bytes_to_write;
	int index;
	struct si_buffer_resource_t buf_desc;
	unsigned int buffer_addr;
	union si_reg_t value;

	if (INST.offen)
	{
		offset = si_isa_read_vreg(work_item,
			INST.vaddr) + INST.offset;

		if (INST.idxen)
			index = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}
	else
	{
		offset = INST.offset;

		if (INST.idxen)
			index = si_isa_read_vreg(work_item, INST.vaddr);
	}

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);

	/* If num_elems is greater than 1, we need to see how the
	 * destination register is handled */
	assert(num_elems == 1);

	bytes_to_write = elem_size * num_elems;

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	buffer_addr = offset;
	if (INST.idxen)
	{
		buffer_addr += buf_desc.stride * (index +
			work_item->id_in_wavefront);
	}

	value.as_uint = si_isa_read_vreg(work_item, INST.vdata);

	mem_write(si_emu->global_mem, buffer_addr, bytes_to_write, &value);

	/* TODO Print value based on type */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: (%u)<=V%u(%u,%gf) ", work_item->id,
			buffer_addr, INST.vdata, value.as_uint,
			value.as_float);
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = buffer_addr;
	work_item->global_mem_access_size = elem_size * num_elems;
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_TBUFFER_STORE_FORMAT_XY_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_TBUFFER_STORE_FORMAT_XYZW_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);

	unsigned int offset;
	int elem_size;
	int num_elems;
	int index;
	struct si_buffer_resource_t buf_desc;
	unsigned int buffer_addr;
	union si_reg_t value;

	if (INST.offen)
	{
		offset = si_isa_read_vreg(work_item,
			INST.vaddr) + INST.offset;

		if (INST.idxen)
			index = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}
	else
	{
		offset = INST.offset;

		if (INST.idxen)
			index = si_isa_read_vreg(work_item, INST.vaddr);
	}

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);

	/* Four Dwords are stored. */
	assert(num_elems == 4);
	assert(elem_size == 4);

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	for (unsigned int i = 0; i < 4; i++)
	{

		buffer_addr = offset + 4 * i;
		if (INST.idxen)
		{
			buffer_addr += buf_desc.stride * (index +
				work_item->id_in_wavefront);
		}

		value.as_uint = si_isa_read_vreg(work_item, INST.vdata + i);

		mem_write(si_emu->global_mem, buffer_addr, 4, &value);

		/* TODO Print value based on type */
		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("t%d: (%u)<=V%u(%u,%gf) ", work_item->id,
				buffer_addr, INST.vdata + i, value.as_uint,
				value.as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = buffer_addr;
	work_item->global_mem_access_size = elem_size * num_elems;
}
#undef INST

#define INST SI_INST_MIMG
void si_isa_IMAGE_STORE_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

#define INST SI_INST_MIMG
void si_isa_IMAGE_SAMPLE_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

#define INST SI_INST_EXP
void si_isa_EXPORT_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

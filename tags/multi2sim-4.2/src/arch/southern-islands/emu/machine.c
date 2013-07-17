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

#include <limits.h>
#include <math.h>

#include <lib/util/debug.h>
#include <lib/util/misc.h>
#include <mem-system/memory.h>

#include "isa.h"
#include "machine.h"
#include "ndrange.h"
#include "wavefront.h"
#include "work-group.h"
#include "work-item.h"


char *err_si_isa_note =
	"\tThe AMD Southern Islands instruction set is partially supported by\n"
	"\tMulti2Sim. If your program is using an unimplemented instruction,\n"
	"\tplease email development@multi2sim.org' to request support for it.\n";

#define NOT_IMPL() fatal("GPU instruction '%s' not implemented\n%s", \
	inst->info->name, err_si_isa_note)

/*
 * SMRD
 */

#define INST SI_INST_SMRD
void si_isa_S_BUFFER_LOAD_DWORD_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t value;

	unsigned int m_offset;
	unsigned int m_base;
	unsigned int addr;

	/* unsigned int m_size; */
	struct si_buffer_desc_t buf_desc;
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

	addr = m_base + m_offset;

	mem_read(si_emu->global_mem, addr, 4, &value);

	/* Store the data in the destination register */
	si_isa_write_sreg(work_item, INST.sdst, value.as_uint);

	/* FIXME Set value based on type */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf%d: S%u<=(%u)(%u,%gf)", 
			work_item->wavefront->id, INST.sdst, addr, 
			value.as_uint, value.as_float);
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
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
	unsigned int addr;

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
	addr = m_base + m_offset;

	assert(!(addr & 0x3));

	for (i = 0; i < 2; i++)
	{
		mem_read(si_emu->global_mem, addr + i * 4, 4, &value[i]);
		si_isa_write_sreg(work_item, INST.sdst + i, value[i].as_uint);
	}

	/* FIXME Set value based on type */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf%d: ", work_item->wavefront->id);
		for (i = 0; i < 2; i++)
		{
			si_isa_debug("S%u<=(%u)(%u,%gf) ", INST.sdst + i, 
				addr + i * 4, value[i].as_uint, 
				value[i].as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
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
	unsigned int addr;

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
	addr = m_base + m_offset;

	assert(!(addr & 0x3));

	for (i = 0; i < 4; i++)
	{
		mem_read(si_emu->global_mem, addr + i * 4, 4,
			&value[i]);
		si_isa_write_sreg(work_item, INST.sdst + i, value[i].as_uint);
	}

	/* FIXME Set value based on type */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf%d: ", work_item->wavefront->id);
		for (i = 0; i < 4; i++)
		{
			si_isa_debug("S%u<=(%u)(%u,%gf) ", INST.sdst + i, 
				addr + i*4, value[i].as_uint, 
				value[i].as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
	work_item->global_mem_access_size = 4 * 4;
}
#undef INST

#define INST SI_INST_SMRD
void si_isa_S_LOAD_DWORDX2_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* Record access */
	work_item->wavefront->scalar_mem_read = 1;

	NOT_IMPL();
}

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

	/* Record access */
	work_item->wavefront->scalar_mem_read = 1;

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
		mem_read(si_emu->global_mem, m_addr + i * 4, 4, &value[i]);
		si_isa_write_sreg(work_item, INST.sdst + i, value[i].as_uint);
	}

	/* FIXME Set value based on type */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S[%u,%u] <= (addr %u): ", INST.sdst, INST.sdst+3, 
			m_addr);
		for (i = 0; i < 4; i++)
		{
			si_isa_debug("S%u<=(%u,%gf) ", INST.sdst + i,
				value[i].as_uint, value[i].as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = m_addr;
	work_item->global_mem_access_size = 4 * 4;
}
#undef INST

#define iNST SI_INST_SMRD
void si_isa_S_LOAD_DWORDX8_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* Record access */
	work_item->wavefront->scalar_mem_read = 1;

	NOT_IMPL();
}
#undef INST

/*
 * SOP2
 */

/* D.u = S0.u + S1.u. SCC = carry out. */
#define INST SI_INST_SOP2
void si_isa_S_ADD_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t sum;
	union si_reg_t carry;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Calculate the sum and carry out. */
	sum.as_uint = s0.as_uint + s1.as_uint;
	carry.as_uint = ((unsigned long long) s0.as_uint + 
		(unsigned long long) s1.as_uint) >> 32;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t sum;
	union si_reg_t ovf;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Calculate the sum and overflow. */
	sum.as_int = s0.as_int + s1.as_int;
	ovf.as_uint = (s0.as_int >> 31 != s1.as_int >> 31) ? 0 : 
		((s0.as_int > 0 && sum.as_int < 0) || 
		(s0.as_int < 0 && sum.as_int > 0));

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t diff;
	union si_reg_t ovf;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Calculate the sum and overflow. */
	diff.as_int = s0.as_int - s1.as_int;
	ovf.as_uint = (s0.as_int >> 31 != s1.as_int >> 31) ? 
		((s0.as_int > 0 && diff.as_int < 0) ||
		(s0.as_int < 0 && diff.as_int > 0)) : 0;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t min;
	union si_reg_t s0_min;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Calculate the minimum operand. */
	if (s0.as_uint < s1.as_uint)
	{
		min.as_uint = s0.as_uint;
		s0_min.as_uint = 1;
	}
	else
	{
		min.as_uint = s1.as_uint;
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t max;
	union si_reg_t s0_max;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Calculate the maximum operand. */
	/* Is max defined as GT or GTE? */
	if (s0.as_int > s1.as_int)
	{
		max.as_int = s0.as_int;
		s0_max.as_uint = 1;
	}
	else
	{
		max.as_int = s1.as_int;
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t max;
	union si_reg_t s0_max;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Calculate the maximum operand. */
	if (s0.as_uint > s1.as_uint)
	{
		max.as_uint = s0.as_uint;
		s0_max.as_uint = 1;
	}
	else
	{
		max.as_uint = s1.as_uint;
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t scc;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);
	scc.as_uint = si_isa_read_sreg(work_item, SI_SCC);

	/* Calculate the result */
	result.as_uint = scc.as_uint ? s0.as_uint : s1.as_uint;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;
	union si_reg_t nonzero;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result.as_uint = s0.as_uint & s1.as_uint;
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

	union si_reg_t s0_lo;
	union si_reg_t s0_hi;
	union si_reg_t s1_lo;
	union si_reg_t s1_hi;
	union si_reg_t result_lo;
	union si_reg_t result_hi;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	s0_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc0 + 1);
	s1_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);
	s1_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc1 + 1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = s0_lo.as_uint & s1_lo.as_uint;
	result_hi.as_uint = s0_hi.as_uint & s1_hi.as_uint;
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;
	union si_reg_t nonzero;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result.as_uint = s0.as_uint | s1.as_uint;
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

	union si_reg_t s0_lo;
	union si_reg_t s0_hi;
	union si_reg_t s1_lo;
	union si_reg_t s1_hi;
	union si_reg_t result_lo;
	union si_reg_t result_hi;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	s0_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc0 + 1);
	s1_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);
	s1_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc1 + 1);

	/* Bitwise OR the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = s0_lo.as_uint | s1_lo.as_uint;
	result_hi.as_uint = s0_hi.as_uint | s1_hi.as_uint;
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

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

	union si_reg_t s0_lo;
	union si_reg_t s0_hi;
	union si_reg_t s1_lo;
	union si_reg_t s1_hi;
	union si_reg_t result_lo;
	union si_reg_t result_hi;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	s0_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc0 + 1);
	s1_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);
	s1_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc1 + 1);

	/* Bitwise OR the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = s0_lo.as_uint ^ s1_lo.as_uint;
	result_hi.as_uint = s0_hi.as_uint ^ s1_hi.as_uint;
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

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

	union si_reg_t s0_lo;
	union si_reg_t s0_hi;
	union si_reg_t s1_lo;
	union si_reg_t s1_hi;
	union si_reg_t result_lo;
	union si_reg_t result_hi;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	s0_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc0 + 1);
	s1_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);
	s1_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc1 + 1);

	/* Bitwise AND the first operand with the negation of the second and
	 * determine if the result is non-zero. */
	result_lo.as_uint = s0_lo.as_uint & ~s1_lo.as_uint;
	result_hi.as_uint = s0_hi.as_uint & ~s1_hi.as_uint;
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

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

	union si_reg_t s0_lo;
	union si_reg_t s0_hi;
	union si_reg_t s1_lo;
	union si_reg_t s1_hi;
	union si_reg_t result_lo;
	union si_reg_t result_hi;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	s0_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc0 + 1);
	s1_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);
	s1_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc1 + 1);

	/* Bitwise AND the two operands and determine if the result is
	 * non-zero. */
	result_lo.as_uint = ~(s0_lo.as_uint & s1_lo.as_uint);
	result_hi.as_uint = ~(s0_hi.as_uint & s1_hi.as_uint);
	nonzero.as_uint = result_lo.as_uint || result_hi.as_uint;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;
	union si_reg_t nonzero;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
	{
		s0.as_uint = INST.lit_cnst;
	}
	else
	{
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	}
	if (INST.ssrc1 == 0xFF)
	{
		assert(INST.lit_cnst < 32);
		s1.as_uint = INST.lit_cnst;
	}
	else
	{
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1) & 0x1F;
	}

	/* Left shift the first operand by the second and determine if the
	 * result is non-zero. */
	result.as_uint = s0.as_uint << s1.as_uint;
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;
	union si_reg_t nonzero;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
	{
		s0.as_uint = INST.lit_cnst;
	}
	else
	{
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	}
	if (INST.ssrc1 == 0xFF)
	{
		assert(INST.lit_cnst < 32);
		s1.as_uint = INST.lit_cnst;
	}
	else
	{
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1) & 0x1F;
	}

	/* Right shift the first operand by the second and determine if the
	 * result is non-zero. */
	result.as_uint = s0.as_uint >> s1.as_uint;
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;
	union si_reg_t nonzero;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
	{
		s0.as_uint = INST.lit_cnst;
	}
	else
	{
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	}
	if (INST.ssrc1 == 0xFF)
	{
		assert(INST.lit_cnst < 32);
		s1.as_uint = INST.lit_cnst;
	}
	else
	{
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1) & 0x1F;
	}

	/* Right shift the first operand sign extended by the second and
	 * determine if the result is non-zero. */
	result.as_int = s0.as_int >> s1.as_int;
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Multiply the two operands. */
	result.as_int = s0.as_int * s1.as_int;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)", INST.sdst, result.as_int);
	}
}
#undef INST

/* D.i = (S0.i >> S1.u[4:0]) & ((1 << S2.u[4:0]) - 1); bitfield extract,
 * S0=data, S1=field_offset, S2=field_width. */
#define INST SI_INST_SOP2
void si_isa_S_BFE_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t s2;
	union si_reg_t result;
	union si_reg_t full_reg;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	full_reg.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* s1 (offset) should be [4:0] of ssrc1 and s2 (width) should be [22:16] of ssrc1*/
	s1.as_uint = full_reg.as_uint & 0x1F;
	s2.as_uint = (full_reg.as_uint >> 16) & 0x7F;

	/* Calculate the result. */
	if (s2.as_uint == 0)
	{
		result.as_int = 0;
	}
	else if (s2.as_uint + s1.as_uint < 32)
	{
		result.as_int = (s0.as_int << (32 - s1.as_uint - s2.as_uint)) >> 
			(32 - s2.as_uint);
	}
	else
	{
		result.as_int = s0.as_int >> s1.as_uint;
	}

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


/*
 * SOPK
 */

/* D.i = signext(simm16). */
#define INST SI_INST_SOPK
void si_isa_S_MOVK_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t simm16;
	union si_reg_t result;

	/* Load constant operand from instruction. */
	simm16.as_ushort[0] = INST.simm16;

	/* Sign extend the short constant to an integer. */
	result.as_int = (int) simm16.as_short[0];

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)", INST.sdst, result.as_int);
	}
}
#undef INST

/* */
#define INST SI_INST_SOPK
void si_isa_S_CMPK_LE_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.i = D.i + signext(SIMM16). scc = overflow. */
#define INST SI_INST_SOPK
void si_isa_S_ADDK_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t simm16;
	union si_reg_t sum;
	union si_reg_t ovf;
	union si_reg_t dst;

	int se_simm16;

	/* Load short constant operand from instruction and sign extend to an 
	 * integer. */
	simm16.as_ushort[0] = INST.simm16;
	se_simm16 = (int) simm16.as_short[0];

	/* Load operand from destination register. */
	dst.as_uint = si_isa_read_sreg(work_item, INST.sdst);

	/* Add the two operands and determine overflow. */
	sum.as_int = dst.as_int + se_simm16;
	ovf.as_uint = (dst.as_int >> 31 != se_simm16 >> 31) ? 0 :
		((dst.as_int > 0 && sum.as_int < 0) || 
		 (dst.as_int < 0 && sum.as_int > 0));

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
	union si_reg_t simm16;
	union si_reg_t product;
	union si_reg_t ovf;
	union si_reg_t dst;

	int se_simm16;

	/* Load short constant operand from instruction and sign extend to an 
	 * integer. */
	simm16.as_ushort[0] = INST.simm16;
	se_simm16 = (int) simm16.as_short[0];

	/* Load operand from destination register. */
	dst.as_uint = si_isa_read_sreg(work_item, INST.sdst);

	/* Multiply the two operands and determine overflow. */
	product.as_int = dst.as_int * se_simm16;
	ovf.as_uint = ((long long) dst.as_int * (long long) se_simm16) > 
		(long long) product.as_int;

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

/*
 * SOP1
 */

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
	nonzero.as_uint = ! !s0.as_uint;

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
	/* FIXME: uncomment when fetch shader is ready */
	// union si_reg_t pc_lo;
	// union si_reg_t s0_lo;
	// union si_reg_t s0_hi;

	// /* Load operands from registers */
	// s0_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	// s0_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc0 + 1);

	// /* PC is implemented as 32-bit*/
	// assert(s0_hi.as_uint == 0);

	// /* Load the PC */
	// pc_lo.as_uint = work_item->wavefront->pc + 4;  // XXX Next instruction?

	// /* Write the results */
	// si_isa_write_sreg(work_item, INST.sdst, pc_lo.as_uint);
	// si_isa_write_sreg(work_item, INST.sdst + 1, 0);

	// /* Set the new PC */
	// work_item->wavefront->pc = s0_lo.as_uint;

	// /* Print isa debug information. */
	// if (debug_status(si_isa_debug_category))
	// {
	// 	si_isa_debug("S%u<=(0x%x) ", INST.sdst, pc_lo.as_uint);
	// 	si_isa_debug("S%u<=(0x%x) ", INST.sdst + 1, s0_hi.as_uint);
	// 	si_isa_debug("PC<=(0x%x)", s0_lo.as_uint);
	// }
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

	union si_reg_t exec_lo;
	union si_reg_t exec_hi;
	union si_reg_t s0_lo;
	union si_reg_t s0_hi;
	union si_reg_t exec_new_lo;
	union si_reg_t exec_new_hi;
	union si_reg_t nonzero;

	/* Load operands from registers. */
	exec_lo.as_uint = si_isa_read_sreg(work_item, SI_EXEC);
	exec_hi.as_uint = si_isa_read_sreg(work_item, SI_EXEC + 1);
	s0_lo.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	s0_hi.as_uint = si_isa_read_sreg(work_item, INST.ssrc0 + 1);

	/* Bitwise AND exec and the first operand and determine if the result 
	 * is non-zero. */
	exec_new_lo.as_uint = s0_lo.as_uint & exec_lo.as_uint;
	exec_new_hi.as_uint = s0_hi.as_uint & exec_hi.as_uint;
	nonzero.as_uint = exec_new_lo.as_uint || exec_new_hi.as_uint;

	/* Write the results. */
	si_isa_write_sreg(work_item, INST.sdst, exec_lo.as_uint);
	si_isa_write_sreg(work_item, INST.sdst + 1, exec_hi.as_uint);
	si_isa_write_sreg(work_item, SI_EXEC, exec_new_lo.as_uint);
	si_isa_write_sreg(work_item, SI_EXEC + 1, exec_new_hi.as_uint);
	si_isa_write_sreg(work_item, SI_SCC, nonzero.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(0x%x) ", INST.sdst, exec_lo.as_uint);
		si_isa_debug("S%u<=(0x%x) ", INST.sdst + 1, exec_hi.as_uint);
		si_isa_debug("exec_lo<=(0x%x) ", exec_new_lo.as_uint);
		si_isa_debug("exec_hi<=(0x%x) ", exec_new_hi.as_uint);
		si_isa_debug("scc<=(%u)", nonzero.as_uint);
	}
}
#undef INST

/*
 * SOPC
 */

/* scc = (S0.i == S1.i). */
#define INST SI_INST_SOPC
void si_isa_S_CMP_EQ_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_int == s1.as_int);

	/* Write the results. */
	si_isa_write_sreg(work_item, SI_SCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf%d: scc<=(%u) (%u ==? %u)", 
			work_item->wavefront->id, result.as_uint, s0.as_int,
			s1.as_int);
	}
}
#undef INST

/* scc = (S0.i > S1.i). */
#define INST SI_INST_SOPC
void si_isa_S_CMP_GT_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_int > s1.as_int);

	/* Write the results. */
	si_isa_write_sreg(work_item, SI_SCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("scc<=(%u) (%u >? %u) ", result.as_uint,
			s0.as_uint, s1.as_uint);
	}
}
#undef INST

/* scc = (S0.i >= S1.i). */
#define INST SI_INST_SOPC
void si_isa_S_CMP_GE_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_int >= s1.as_int);

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_int < s1.as_int);

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_int <= s1.as_int);

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_uint > s1.as_uint);

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_uint >= s1.as_uint);

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (INST.ssrc0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_sreg(work_item, INST.ssrc0);
	if (INST.ssrc1 == 0xFF)
		s1.as_uint = INST.lit_cnst;
	else
		s1.as_uint = si_isa_read_sreg(work_item, INST.ssrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_uint <= s1.as_uint);

	/* Write the results. */
	si_isa_write_sreg(work_item, SI_SCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("scc<=(%u) ", result.as_uint);
	}
}
#undef INST

/*
 * SOPP
 */

/* End the program. */
void si_isa_S_ENDPGM_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	work_item->wavefront->finished = 1;
	work_item->work_group->wavefronts_completed_emu++;
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

	union si_reg_t scc;

	scc.as_uint = si_isa_read_sreg(work_item, SI_SCC);

	if (scc.as_uint)
	{
		assert(si_isa_read_sreg(work_item, SI_SCC) == 1);

		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		/* Determine the program counter to branch to. */
		work_item->wavefront->pc +=
			se_simm16 * 4 + 4 - inst->info->size;

		/* Print isa debug information. */
		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("wf%d: SCC=%u (taken)", 
				work_item->wavefront->id, scc.as_uint);
		}
	}
	else
	{
		/* Print isa debug information. */
		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("wf%d: SCC=%u (not taken)", 
				work_item->wavefront->id, scc.as_uint);
		}
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

	union si_reg_t exec;
	union si_reg_t execz;

	exec.as_uint = si_isa_read_sreg(work_item, SI_EXEC);
	execz.as_uint = si_isa_read_sreg(work_item, SI_EXECZ);

	if (execz.as_uint)
	{
		/* Load the short constant operand and sign extend into an
		 * integer. */
		simm16 = INST.simm16;
		se_simm16 = simm16;

		/* Determine the program counter to branch to. */
		work_item->wavefront->pc +=
			se_simm16 * 4 + 4 - inst->info->size;

		/* Print isa debug information. */
		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("wf%d: EXEC=0x%x, EXECZ=%u (taken)", 
				work_item->wavefront->id, exec.as_uint, 
				execz.as_uint);
		}
	}
	else
	{
		/* Print isa debug information. */
		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("wf%d: EXEC=0x%x, EXECZ=%u " 
				"(not taken)", work_item->wavefront->id, 
				exec.as_uint, execz.as_uint);
		}
	}
}
#undef INST


/* if(EXEC != 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */
#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_EXECNZ_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	short simm16;
	int se_simm16;

	if (!si_isa_read_sreg(work_item, SI_EXECZ))
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

/* Suspend current wavefront at the barrier. If all wavefronts in work-group
 * reached the barrier, wake them up */
void si_isa_S_BARRIER_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	struct si_wavefront_t *wavefront = work_item->wavefront;
	struct si_work_group_t *work_group = work_item->work_group;

	int wavefront_id;

	/* Suspend current wavefront at the barrier */
	wavefront->barrier_inst = 1;
	wavefront->at_barrier = 1;
	work_group->wavefronts_at_barrier++;

	si_isa_debug("Group %d wavefront %d reached barrier "
		"(%d reached, %d left)\n",
		work_group->id, wavefront->id, 
		work_group->wavefronts_at_barrier,
		work_group->wavefront_count - 
		work_group->wavefronts_at_barrier);

	/* If all wavefronts in work-group reached the barrier, wake them up */
	if (work_group->wavefronts_at_barrier== work_group->wavefront_count)
	{
		SI_FOREACH_WAVEFRONT_IN_WORK_GROUP(work_group, wavefront_id)
		{
			work_group->wavefronts[wavefront_id]->at_barrier = 0;
		}
		work_group->wavefronts_at_barrier = 0;

		si_isa_debug("Group %d completed barrier\n", work_group->id);
	}
}

void si_isa_S_WAITCNT_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* Nothing to do in emulation */
	work_item->wavefront->mem_wait = 1;
}

/*
 * VOP1
 */

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

/* D.f = (double)S0.i. */
#define INST SI_INST_VOP1
void si_isa_V_CVT_F64_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union
	{
		double as_double;
		unsigned int as_reg[2];

	} value;
	union si_reg_t s0;
	union si_reg_t result_lo;
	union si_reg_t result_hi;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);

	/* Convert and separate value. */
	value.as_double = (double) s0.as_int;

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
	union si_reg_t s0;
	union si_reg_t value;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	value.as_float = (float) s0.as_int;

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
	union si_reg_t s0;
	union si_reg_t value;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	value.as_float = (float) s0.as_uint;

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
	union si_reg_t s0;
	union si_reg_t value;

	float fvalue;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	fvalue = s0.as_float;

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
	union si_reg_t s0;
	union si_reg_t value;

	float fvalue;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	fvalue = s0.as_float;

	/* Handle special number cases and cast to an int */

	/* inf, > max_int --> max_int */
	if ((isinf(fvalue) && fvalue > 0.0f) || fvalue >= INT_MAX)
		value.as_int = INT_MAX;
	/* -inf, < -max_int --> -max_int */
	else if (isinf(fvalue) || fvalue < INT_MIN)
		value.as_int = INT_MIN;
	/* NaN, 0, -0 --> 0 */
	else if (isnan(fvalue) || fvalue == 0.0f || fvalue == -0.0f)
		value.as_int = 0;
	else
		value.as_int = (int) fvalue;

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

	assert(INST.src0 != 0xFF);

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

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
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
	union si_reg_t s0;
	union si_reg_t value;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);

	/* Truncate decimal portion */
	value.as_float = (float)((int)s0.as_float);

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
	union si_reg_t s0;
	union si_reg_t rcp;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);

	rcp.as_float = 1.0f / s0.as_float;

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
	union si_reg_t s0;
	union si_reg_t srt;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);

	srt.as_float = sqrtf(s0.as_float);

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
	/* XXX Should it be module instead of dividing? */
	s0.as_float = s0.as_float * (2 * M_PI);

	if (IN_RANGE(s0.as_float, -256, 256))
	{
		result.as_float = sinf(s0.as_float);
	}
	else
	{
		assert(0); /* Haven't debugged this yet */
		result.as_float = 0;
	}

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) (sin %gf) ", work_item->id, 
			INST.vdst, result.as_float, s0.as_float);
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
	/* XXX Should it be module instead of dividing? */
	s0.as_float = s0.as_float * (2 * M_PI);

	if (IN_RANGE(s0.as_float, -256, 256))
	{
		result.as_float = cosf(s0.as_float);
	}
	else
	{
		assert(0); /* Haven't debugged this yet */
		result.as_float = 1;
	}

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) (cos %gf) ", work_item->id, 
			INST.vdst, result.as_float, s0.as_float);
	}
}
#undef INST

/* D.u = ~S0.u. */
#define INST SI_INST_VOP1
void si_isa_V_NOT_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t result;

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);

	/* Bitwise not */
	result.as_uint = ~s0.as_uint;

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
	union si_reg_t s0;
	union si_reg_t m0;
	
	assert(INST.src0 != 0xFF);

	/* Load operand from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	m0.as_uint = si_isa_read_reg(work_item, SI_M0);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst+m0.as_uint, s0.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V[%u+%u]<=(0x%x) ", work_item->id, 
			INST.vdst, m0.as_uint, s0.as_uint);
	}
}
#undef INST


/* VGPR[D.u] = VGPR[S0.u + M0.u]. */
#define INST SI_INST_VOP1
void si_isa_V_MOVRELS_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t m0;
	
	assert(INST.src0 != 0xFF);

	/* Load operand from register or as a literal constant. */
	m0.as_uint = si_isa_read_reg(work_item, SI_M0);
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0 + m0.as_uint);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, s0.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V[%u]<=(0x%x) ", work_item->id, 
			INST.vdst, s0.as_uint);
	}
}
#undef INST

/*
 * VOP2
 */

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

	/* Load operands from register or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);
	vcci = si_isa_read_bitmask_sreg(work_item, SI_VCC);

	/* Calculate the result. */
	result.as_uint = (vcci) ? s1.as_uint : s0.as_uint;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t sum;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the sum. */
	sum.as_float = s0.as_float + s1.as_float;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t dif;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the difference. */
	dif.as_float = s0.as_float - s1.as_float;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t dif;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the difference. */
	dif.as_float = s1.as_float - s0.as_float;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t product;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the product. */
	if (s0.as_float == 0.0f || s1.as_float == 0.0f)
	{
		product.as_float = 0.0f;
	}
	else
	{
		product.as_float = s0.as_float * s1.as_float;
	}

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, product.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) (%gf * %gf) ", work_item->id, 
			INST.vdst, product.as_float, s0.as_float, s1.as_float);
	}
}
#undef INST

/* D.f = S0.f * S1.f. */
#define INST SI_INST_VOP2
void si_isa_V_MUL_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t product;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the product. */
	product.as_float = s0.as_float * s1.as_float;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, product.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) (%gf*%gf)", work_item->id, 
			INST.vdst, product.as_float, s0.as_float, s1.as_float);
	}
}
#undef INST

/* D.i = S0.i[23:0] * S1.i[23:0]. */
#define INST SI_INST_VOP2
void si_isa_V_MUL_I32_I24_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t product;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Truncate operands to 24-bit signed integers */
	s0.as_uint = SEXT32(s0.as_uint, 24);
	s1.as_uint = SEXT32(s1.as_uint, 24);

	/* Calculate the product. */
	product.as_int = s0.as_int * s1.as_int;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t min;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the minimum operand. */
	if (s0.as_float < s1.as_float)
	{
		min.as_float = s0.as_float;
	}
	else
	{
		min.as_float = s1.as_float;
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t max;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the minimum operand. */
	if (s0.as_float > s1.as_float)
	{
		max.as_float = s0.as_float;
	}
	else
	{
		max.as_float = s1.as_float;
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t max;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the minimum operand. */
	if (s0.as_int > s1.as_int)
	{
		max.as_int = s0.as_int;
	}
	else
	{
		max.as_int = s1.as_int;
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t min;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the minimum operand. */
	if (s0.as_uint < s1.as_uint)
	{
		min.as_uint = s0.as_uint;
	}
	else
	{
		min.as_uint = s1.as_uint;
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t max;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the minimum operand. */
	if (s0.as_uint > s1.as_uint)
	{
		max.as_uint = s0.as_uint;
	}
	else
	{
		max.as_uint = s1.as_uint;
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
	{
		assert(INST.lit_cnst < 32);
		s0.as_uint = INST.lit_cnst;
	}
	else
	{
		s0.as_uint = si_isa_read_reg(work_item, INST.src0) & 0x1F;
	}
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Right shift s1 by s0. */
	result.as_uint = s1.as_uint >> s0.as_uint;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u) (%u >> %u) ", work_item->id,
			INST.vdst, result.as_uint, s1.as_uint, s0.as_uint);
	}
}
#undef INST

/* D.i = S1.i >> S0.i[4:0]. */
#define INST SI_INST_VOP2
void si_isa_V_ASHRREV_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
	{
		assert(INST.lit_cnst < 32);
		s0.as_uint = INST.lit_cnst & 0x1F;
	}
	else
	{
		s0.as_uint = si_isa_read_reg(work_item, INST.src0) & 0x1F;
	}
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Right shift s1 by s0. */
	result.as_int = s1.as_int >> s0.as_int;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d, %u) ", work_item->id, INST.vdst,
			result.as_int, result.as_uint);
	}
}
#undef INST

/* D.u = S0.u << S1.u[4:0]. */
#define INST SI_INST_VOP2
void si_isa_V_LSHL_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1) & 0x1F;

	/* Left shift s1 by s0. */
	result.as_uint = s0.as_uint << s1.as_uint;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
	{
		assert(INST.lit_cnst < 32);
		s0.as_uint = INST.lit_cnst;
	}
	else
	{
		s0.as_uint = si_isa_read_reg(work_item, INST.src0) & 0x1F;
	}
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Left shift s1 by s0. */
	result.as_uint = s1.as_uint << s0.as_uint;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) (%u << %u) ", work_item->id,
			INST.vdst, result.as_uint, s1.as_uint, s0.as_uint);
	}
}
#undef INST

/* D.u = S0.u & S1.u. */
#define INST SI_INST_VOP2
void si_isa_V_AND_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
	{
		s0.as_uint = INST.lit_cnst;
	}
	else
	{
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	}
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Bitwise OR the two operands. */
	result.as_uint = s0.as_uint & s1.as_uint;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) (%u & %u) ", work_item->id, 
			INST.vdst, result.as_uint, s0.as_uint, s1.as_uint);
	}
}
#undef INST

/* D.u = S0.u | S1.u. */
#define INST SI_INST_VOP2
void si_isa_V_OR_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Bitwise OR the two operands. */
	result.as_uint = s0.as_uint | s1.as_uint;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x) (%u | %u) ", 
			work_item->id, INST.vdst, result.as_uint, s0.as_uint,
			s1.as_uint);
	}
}
#undef INST

/* D.u = S0.u ^ S1.u. */
#define INST SI_INST_VOP2
void si_isa_V_XOR_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Bitwise OR the two operands. */
	result.as_uint = s0.as_uint ^ s1.as_uint;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u)(%u ^ %u) ", work_item->id, 
			INST.vdst, result.as_uint, s0.as_uint, s1.as_uint);
	}
}
#undef INST

/*D.u = ((1<<S0.u[4:0])-1) << S1.u[4:0]; S0=bitfield_width, S1=bitfield_offset. */
#define INST SI_INST_VOP2
void si_isa_V_BFM_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* D.f = S0.f * S1.f + D.f. */
#define INST SI_INST_VOP2
void si_isa_V_MAC_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t dst;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);
	dst.as_uint = si_isa_read_vreg(work_item, INST.vdst);

	/* Calculate the result. */
	result.as_float = s0.as_float * s1.as_float + dst.as_float;

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

/* D.f = S0.f * K + S1.f; K is a 32-bit inline constant */
#define INST SI_INST_VOP2
void si_isa_V_MADMK_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t K;
	union si_reg_t dst;

	/* Load operands from registers or as a literal constant. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);
	K.as_uint = INST.lit_cnst;
	
	/* Calculate the result */
	dst.as_float = s0.as_float * K.as_float + s1.as_float;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, dst.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%f) (%f * %f + %f)", work_item->id, 
			INST.vdst, dst.as_float, s0.as_float, K.as_float, 
			s1.as_float);
	}
}
#undef INST

/* D.u = S0.u + S1.u, vcc = carry-out. */
#define INST SI_INST_VOP2
void si_isa_V_ADD_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t sum;
	union si_reg_t carry;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the sum and carry. */
	sum.as_int = s0.as_int + s1.as_int;
	carry.as_uint = 
		! !(((long long) s0.as_int + (long long) s1.as_int) >> 32);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, sum.as_uint);
	si_isa_bitmask_sreg(work_item, SI_VCC, carry.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) (%d + %d) ", work_item->id, 
			INST.vdst, sum.as_int, s0.as_int, s1.as_int);
		si_isa_debug("vcc<=(%u) ", carry.as_uint);
	}
}
#undef INST

/* D.u = S0.u - S1.u; vcc = carry-out. */
#define INST SI_INST_VOP2
void si_isa_V_SUB_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t dif;
	union si_reg_t carry;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the difference and carry. */
	dif.as_uint = s0.as_int - s1.as_int;
	carry.as_uint = (s1.as_int > s0.as_int);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, dif.as_uint);
	si_isa_bitmask_sreg(work_item, SI_VCC, carry.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst,
			dif.as_int);
		si_isa_debug("vcc<=(%u) ", carry.as_uint);
	}
}
#undef INST

/* D.u = S1.u - S0.u; vcc = carry-out. */
#define INST SI_INST_VOP2
void si_isa_V_SUBREV_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t dif;
	union si_reg_t carry;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Calculate the difference and carry. */
	dif.as_int = s1.as_int - s0.as_int;
	carry.as_uint = (s0.as_int > s1.as_int);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, dif.as_uint);
	si_isa_bitmask_sreg(work_item, SI_VCC, carry.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst,
			dif.as_int);
		si_isa_debug("vcc<=(%u) ", carry.as_uint);
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

/*
 * VOPC
 */

/* vcc = (S0.f < S1.f). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_LT_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_float < s1.as_float);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.f > S1.f). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_GT_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_float > s1.as_float);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = !(s0.as_float > s1.as_float);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = !(S0.f == S1.f). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_NEQ_F32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item, INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = !(s0.as_float == s1.as_float);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item,INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_int < s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.i == S1.i). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_EQ_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item,INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_int == s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.i <= S1.i). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_LE_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item,INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_int <= s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.i > S1.i). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_GT_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 255)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item,INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_int > s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.i <> S1.i). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_NE_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item,INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_int != s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* D.u = (S0.i >= S1.i). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_GE_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item,INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_int >= s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item,INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_uint < s1.as_uint);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item,INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_uint <= s1.as_uint);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
			work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

/* vcc = (S0.u > S1.u). */
#define INST SI_INST_VOPC
void si_isa_V_CMP_GT_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	/* Load operands from registers or as a literal constant. */
	if (INST.src0 == 0xFF)
		s0.as_uint = INST.lit_cnst;
	else
		s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_vreg(work_item,INST.vsrc1);

	/* Compare the operands. */
	result.as_uint = (s0.as_uint > s1.as_uint);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, SI_VCC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: vcc<=(%u) ",
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


#define INST SI_INST_VOPC
void si_isa_V_CMP_GE_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST



/*
 * VOP3a
 */

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

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);
	vcci = si_isa_read_bitmask_sreg(work_item, INST.src2);

	/* Perform "floating-point negation" */
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	/* Calculate the result. */
	result.as_uint = (vcci) ? s1.as_uint : s0.as_uint;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u, %gf) (s1=%u, s0=%u)", 
			work_item->id, INST.vdst, result.as_uint, 
			result.as_float, s1.as_uint, s0.as_uint);
	}
}
#undef INST

/* D.f = S0.f + S1.f. */
#define INST SI_INST_VOP3a
void si_isa_V_ADD_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t sum;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers or as a literal constant. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	/* Calculate the sum. */
	sum.as_float = s0.as_float + s1.as_float;

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

/* D.f = S1.f - S0.f */
#define INST SI_INST_VOP3a
void si_isa_V_SUBREV_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t diff;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers or as a literal constant. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	/* Calculate the diff. */
	diff.as_float = s1.as_float - s0.as_float;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers or as a literal constant. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	/* Calculate the result. */
	result.as_float = s0.as_float * s1.as_float;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) (%gf*%gf)", work_item->id, 
			INST.vdst, result.as_float, s0.as_float, s1.as_float);
	}
}
#undef INST

/* D.f = S0. * S1.. */
#define INST SI_INST_VOP3a
void si_isa_V_MUL_I32_I24_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t product;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers or as a literal constant. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Truncate operands to 24-bit signed integers */
	s0.as_uint = SEXT32(s0.as_uint, 24);
	s1.as_uint = SEXT32(s1.as_uint, 24);

	/* Calculate the product. */
	product.as_int = s0.as_int * s1.as_int;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	/* Calculate the result. */
	result.as_float = (s0.as_float > s1.as_float) ? 
		s0.as_float : s1.as_float;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t s2;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);
	s2.as_uint = si_isa_read_reg(work_item, INST.src2);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	if (INST.abs & 4)
		s2.as_float = fabsf(s2.as_float);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	if (INST.neg & 4)
		s2.as_float = -s2.as_float;

	/* Calculate the result. */
	result.as_float = s0.as_float * s1.as_float + s2.as_float;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t s2;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);
	s2.as_uint = si_isa_read_reg(work_item, INST.src2);

	s0.as_uint = s0.as_uint & 0x00FFFFFF;
	s1.as_uint = s1.as_uint & 0x00FFFFFF;

	/* Calculate the result. */
	result.as_uint = s0.as_uint * s1.as_uint + s2.as_uint;

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

/* D.u = (S0.u >> S1.u[4:0]) & ((1 << S2.u[4:0]) - 1); bitfield extract,
 * S0=data, S1=field_offset, S2=field_width. */
#define INST SI_INST_VOP3a
void si_isa_V_BFE_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t s2;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);
	s2.as_uint = si_isa_read_reg(work_item, INST.src2);

	s1.as_uint = s1.as_uint & 0x1F;
	s2.as_uint = s2.as_uint & 0x1F;

	/* Calculate the result. */
	result.as_uint = (s0.as_uint >> s1.as_uint) & ((1 << s2.as_uint) - 1);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: value:s0:%u, offset:s1:%u, width:s2:%u ",
			work_item->id, s0.as_uint, s1.as_uint, s2.as_uint);
		si_isa_debug("V%u<=(0x%x) ", INST.vdst, result.as_uint);
	}
}
#undef INST

/* D.i = (S0.i >> S1.u[4:0]) & ((1 << S2.u[4:0]) - 1); bitfield extract,
 * S0=data, S1=field_offset, S2=field_width. */
#define INST SI_INST_VOP3a
void si_isa_V_BFE_I32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t s2;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);
	s2.as_uint = si_isa_read_reg(work_item, INST.src2);

	s1.as_uint = s1.as_uint & 0x1F;
	s2.as_uint = s2.as_uint & 0x1F;

	/* Calculate the result. */
	if (s2.as_uint == 0)
	{
		result.as_int = 0;
	}
	else if (s2.as_uint + s1.as_uint < 32)
	{
		result.as_int = (s0.as_int << (32 - s1.as_uint - s2.as_uint)) >> 
			(32 - s2.as_uint);
	}
	else
	{
		result.as_int = s0.as_int >> s1.as_uint;
	}

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t s2;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);
	s2.as_uint = si_isa_read_reg(work_item, INST.src2);

	/* Calculate the result. */
	result.as_uint = (s0.as_uint & s1.as_uint) | 
		(~s0.as_uint & s2.as_uint);

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t s2;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);
	s2.as_uint = si_isa_read_reg(work_item, INST.src2);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	if (INST.abs & 4)
		s2.as_float = fabsf(s2.as_float);

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	if (INST.neg & 4)
		s2.as_float = -s2.as_float;

	/* FMA */
	result.as_float = (s0.as_float * s1.as_float) + s2.as_float;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) (%gf*%gf + %gf) ", 
			work_item->id, INST.vdst, result.as_float, 
			s0.as_float, s1.as_float, s2.as_float);
	}
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

/* D.u = ({S0,S1} >> S2.u[4:0]) & 0xFFFFFFFF. */
#define INST SI_INST_VOP3a
void si_isa_V_ALIGNBIT_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t src2;
	union si_reg_t result;

	union
	{
		unsigned long long as_b64;
		unsigned int as_reg[2];

	} src;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	src.as_reg[0] = si_isa_read_reg(work_item, INST.src1);
	src.as_reg[1] = si_isa_read_reg(work_item, INST.src0);
	src2.as_uint = si_isa_read_reg(work_item, INST.src2);
	src2.as_uint = src2.as_uint & 0x1F;

	/* ({S0,S1} >> S2.u[4:0]) & 0xFFFFFFFF. */
	result.as_uint = (src.as_b64 >> src2.as_uint) & 0xFFFFFFFF;

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x, %u) ({0x%x,0x%x} >> %u) ",
			work_item->id, INST.vdst, result.as_uint, 
			result.as_uint, src.as_reg[1], src.as_reg[0], 
			src2.as_uint);
	}
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Calculate the product. */
	result.as_uint = s0.as_uint * s1.as_uint;

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Calculate the product and shift right. */
	result.as_uint = (unsigned int) 
		(((unsigned long long)s0.as_uint * 
		(unsigned long long)s1.as_uint) >> 32);

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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Calculate the product. */
	result.as_int = s0.as_int * s1.as_int;


	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d)(%d*%d) ", work_item->id, 
			INST.vdst, result.as_int, s0.as_int, s1.as_int);
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

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);

	/* Apply negation modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	assert (!(INST.abs & 2));
	assert (!(INST.abs & 4));

	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	assert (!(INST.neg & 2));
	assert (!(INST.neg & 4));

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

/* D.u = (S0.f < S1.f). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_LT_F32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	/* Compare the operands. */
	result.as_uint = (s0.as_float < s1.as_float);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	/* Compare the operands. */
	result.as_uint = (s0.as_float == s1.as_float);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	/* Compare the operands. */
	result.as_uint = (s0.as_float > s1.as_float);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	/* Compare the operands. */
	result.as_uint = !(s0.as_float == s1.as_float);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_float = fabsf(s0.as_float);
	if (INST.abs & 2)
		s1.as_float = fabsf(s1.as_float);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_float = -s0.as_float;
	if (INST.neg & 2)
		s1.as_float = -s1.as_float;
	assert(!(INST.neg & 4));

	/* Compare the operands. */
	result.as_uint = !(s0.as_float < s1.as_float);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	/* Compare the operands. */
	result.as_uint = (s0.as_int < s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	/* Compare the operands. */
	result.as_uint = (s0.as_int == s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	/* Compare the operands. */
	result.as_uint = (s0.as_int <= s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	/* Compare the operands. */
	result.as_uint = (s0.as_int > s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:%u]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst, INST.vdst + 1, 
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.i <> S1.i). */
#define INST SI_INST_VOP3a
void si_isa_V_CMP_NE_I32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	/* Compare the operands. */
	result.as_uint = !(s0.as_int == s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	/* Compare the operands. */
	result.as_uint = (s0.as_int >= s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

/* D.u = (S0.i == S1.i). Also write EXEC */
#define INST SI_INST_VOP3a
void si_isa_V_CMPX_EQ_I32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Apply absolute value modifiers. */
	if (INST.abs & 1)
		s0.as_int = abs(s0.as_int);
	if (INST.abs & 2)
		s1.as_int = abs(s1.as_int);
	assert(!(INST.abs & 4));

	/* Apply negation modifiers. */
	if (INST.neg & 1)
		s0.as_int = -s0.as_int;
	if (INST.neg & 2)
		s1.as_int = -s1.as_int;
	assert(!(INST.neg & 4));

	/* Compare the operands. */
	result.as_uint = (s0.as_int == s1.as_int);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Write EXEC */
	si_isa_bitmask_sreg(work_item, SI_EXEC, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:%u],EXEC<=(%u) ",
			work_item->id_in_wavefront, INST.vdst, INST.vdst+1,
			result.as_uint);
	}
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Compare the operands. */
	result.as_uint = (s0.as_uint < s1.as_uint);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Compare the operands. */
	result.as_uint = (s0.as_uint <= s1.as_uint);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Compare the operands. */
	result.as_uint = (s0.as_uint > s1.as_uint);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
			work_item->id_in_wavefront, INST.vdst,
			result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP3a
void si_isa_V_CMP_LG_U32_VOP3a_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Calculate result. */
	result.as_uint = ((s0.as_uint < s1.as_uint) || 
		(s0.as_uint > s1.as_uint));

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t result;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);

	/* Compare the operands. */
	result.as_uint = (s0.as_uint >= s1.as_uint);

	/* Write the results. */
	si_isa_bitmask_sreg(work_item, INST.vdst, result.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%u) ",
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
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t s2;
	union si_reg_t median;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);
	s2.as_uint = si_isa_read_reg(work_item, INST.src2);

	/* Determine the median. */
	/* max3(s0, s1, s2) == s0 */
	if (s0.as_int >= s1.as_int && s0.as_int >= s2.as_int)
	{
		/* max(s1, s2) */
		median.as_int = (s1.as_int >= s2.as_int) ? 
			s1.as_int : s2.as_int;
	}
	/* max3(s0, s1, s2) == s1 */
	else if (s1.as_int >= s0.as_int && s1.as_int >= s2.as_int)
	{
		/* max(s0, s2) */
		median.as_int = (s0.as_int >= s2.as_int) ? 
			s0.as_int : s2.as_int;
	}
	/* max3(s0, s1, s2) == s2 */
	else if (s2.as_int >= s0.as_int && s2.as_int >= s1.as_int)
	{
		/* max(s0, s1) */
		median.as_int = (s0.as_int >= s1.as_int) ? 
			s0.as_int : s1.as_int;
	}
	else
	{
		fatal("%s: Median algorithm failed\n", __FUNCTION__);
	}

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, median.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V[%u]<=(%d) ",
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

	union si_reg_t s1;
	union si_reg_t result_lo;
	union si_reg_t result_hi;

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_reg[0] = si_isa_read_reg(work_item, INST.src0);
	s0.as_reg[1] = si_isa_read_reg(work_item, INST.src0 + 1);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);
	s1.as_uint = s1.as_uint & 0x1F;

	/* Shift s0. */
	value.as_b64 = s0.as_b64 >> s1.as_uint;

	/* Write the results. */
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	si_isa_write_vreg(work_item, INST.vdst, result_lo.as_uint);
	si_isa_write_vreg(work_item, INST.vdst + 1, result_hi.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u]<=(0x%x) ",
			work_item->id_in_wavefront, INST.vdst,
			result_lo.as_uint);
		si_isa_debug("S[%u]<=(0x%x) ", INST.vdst + 1,
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

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_reg[0] = si_isa_read_reg(work_item, INST.src0);
	s0.as_reg[1] = si_isa_read_reg(work_item, INST.src0 + 1);
	s1.as_reg[0] = si_isa_read_reg(work_item, INST.src1);
	s1.as_reg[1] = si_isa_read_reg(work_item, INST.src1 + 1);

	/* Add the operands, take into account special number cases. */

	/* s0 == NaN64 || s1 == NaN64 */
	if (fpclassify(s0.as_double) == FP_NAN ||
		fpclassify(s1.as_double) == FP_NAN)
	{
		/* value <-- NaN64 */
		value.as_double = NAN;
	}
	/* s0,s1 == infinity */
	else if (fpclassify(s0.as_double) == FP_INFINITE &&
		fpclassify(s1.as_double) == FP_INFINITE)
	{
		/* value <-- NaN64 */
		value.as_double = NAN;
	}
	/* s0,!s1 == infinity */
	else if (fpclassify(s0.as_double) == FP_INFINITE)
	{
		/* value <-- s0(+-infinity) */
		value.as_double = s0.as_double;
	}
	/* s1,!s0 == infinity */
	else if (fpclassify(s1.as_double) == FP_INFINITE)
	{
		/* value <-- s1(+-infinity) */
		value.as_double = s1.as_double;
	}
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
	{
		value.as_double = s0.as_double + s1.as_double;
	}

	/* Write the results. */
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	si_isa_write_vreg(work_item, INST.vdst, result_lo.as_uint);
	si_isa_write_vreg(work_item, INST.vdst + 1, result_hi.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%lgf) ",
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

	assert(!INST.clamp);
	assert(!INST.omod);
	assert(!INST.neg);
	assert(!INST.abs);

	/* Load operands from registers. */
	s0.as_reg[0] = si_isa_read_reg(work_item, INST.src0);
	s0.as_reg[1] = si_isa_read_reg(work_item, INST.src0 + 1);
	s1.as_reg[0] = si_isa_read_reg(work_item, INST.src1);
	s1.as_reg[1] = si_isa_read_reg(work_item, INST.src1 + 1);

	/* Multiply the operands, take into account special number cases. */

	/* s0 == NaN64 || s1 == NaN64 */
	if (fpclassify(s0.as_double) == FP_NAN ||
		fpclassify(s1.as_double) == FP_NAN)
	{
		/* value <-- NaN64 */
		value.as_double = NAN;
	}
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
	{
		value.as_double = s0.as_double * s1.as_double;
	}

	/* Write the results. */
	result_lo.as_uint = value.as_reg[0];
	result_hi.as_uint = value.as_reg[1];
	si_isa_write_vreg(work_item, INST.vdst, result_lo.as_uint);
	si_isa_write_vreg(work_item, INST.vdst + 1, result_hi.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: S[%u:+1]<=(%lgf) ",
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

/*
 * VOP3b
 */

/* D.u = S0.u + S1.u + VCC; VCC=carry-out (VOP3:sgpr=carry-out,
 * S2.u=carry-in). */
#define INST SI_INST_VOP3b
void si_isa_V_ADDC_U32_VOP3b_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t s0;
	union si_reg_t s1;
	union si_reg_t sum;
	union si_reg_t carry_in;
	union si_reg_t carry_out;

	assert(!INST.omod);
	assert(!INST.neg); 

	/* Load operands from registers. */
	s0.as_uint = si_isa_read_reg(work_item, INST.src0);
	s1.as_uint = si_isa_read_reg(work_item, INST.src1);
	carry_in.as_uint = si_isa_read_bitmask_sreg(work_item, INST.src2);

	/* Calculate sum and carry. */
	sum.as_uint = s0.as_uint + s1.as_uint + carry_in.as_uint;
	carry_out.as_uint =
		! !(((unsigned long long) s0.as_uint + 
			(unsigned long long) s1.as_uint +
			(unsigned long long) carry_in.as_uint) >> 32);

	/* Write the results. */
	si_isa_write_vreg(work_item, INST.vdst, sum.as_uint);
	si_isa_bitmask_sreg(work_item, INST.sdst, carry_out.as_uint);

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u) ", work_item->id, INST.vdst,
			sum.as_uint);
		si_isa_debug("vcc<=(%u) ", carry_out.as_uint);
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

/* 
 * VINTRP
 */

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

/* 
 * DS
 */

/* DS[A] = (DS[A] >= D0 ? 0 : DS[A] + 1); uint increment. */
#define INST SI_INST_DS
void si_isa_DS_INC_U32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/* DS[ADDR+offset0*4] = D0; DS[ADDR+offset1*4] = D1; Write 2 Dwords */
#define INST SI_INST_DS
void si_isa_DS_WRITE2_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t addr0;
	union si_reg_t addr1;
	union si_reg_t data0;
	union si_reg_t data1;

	assert(!INST.gds);

	/* Load address and data from registers. */
	addr0.as_uint = si_isa_read_vreg(work_item, INST.addr);
	addr0.as_uint += INST.offset0*4;
	addr1.as_uint = si_isa_read_vreg(work_item, INST.addr);
	addr1.as_uint += INST.offset1*4;
	data0.as_uint = si_isa_read_vreg(work_item, INST.data0);
	data1.as_uint = si_isa_read_vreg(work_item, INST.data1);

	if (addr0.as_uint > MIN(work_item->work_group->ndrange->local_mem_top,
		si_isa_read_sreg(work_item, SI_M0)))
	{
		fatal("%s: invalid address\n", __FUNCTION__);
	}
	if (addr1.as_uint > MIN(work_item->work_group->ndrange->local_mem_top,
		si_isa_read_sreg(work_item, SI_M0)))
	{
		fatal("%s: invalid address\n", __FUNCTION__);
	}

	/* Write Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_write(work_item->work_group->lds_module, addr0.as_uint, 4,
			&data0.as_uint);
		mem_write(work_item->work_group->lds_module, addr1.as_uint, 4,
			&data1.as_uint);
	}

	/* Record last memory access for the detailed simulator. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		/* If offset1 != 1, then the following is incorrect */
		assert(INST.offset0 == 0);
		assert(INST.offset1 == 1);
		work_item->lds_access_count = 2;
		work_item->lds_access_type[0] = 2;
		work_item->lds_access_addr[0] = addr0.as_uint;
		work_item->lds_access_size[0] = 4;
		work_item->lds_access_type[1] = 2;
		work_item->lds_access_addr[1] = addr0.as_uint + 4;
		work_item->lds_access_size[1] = 4;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category) && INST.gds)
	{
		si_isa_debug("t%d: GDS[%u]<=(%u,%f) ", work_item->id, 
			addr0.as_uint, data0.as_uint, data0.as_float);
		si_isa_debug("GDS[%u]<=(%u,%f) ", addr1.as_uint, data0.as_uint,
			data0.as_float);
	}
	else
	{
		si_isa_debug("t%d: LDS[%u]<=(%u,%f) ", work_item->id, 
			addr0.as_uint, data0.as_uint, data0.as_float);
		si_isa_debug("LDS[%u]<=(%u,%f) ", addr1.as_uint, data1.as_uint, 
			data1.as_float);
	}
}
#undef INST

/* DS[A] = D0; write a Dword. */
#define INST SI_INST_DS
void si_isa_DS_WRITE_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t addr;
	union si_reg_t data0;

	assert(!INST.offset0);
	//assert(!INST.offset1);
	assert(!INST.gds);

	/* Load address and data from registers. */
	addr.as_uint = si_isa_read_vreg(work_item, INST.addr);
	data0.as_uint = si_isa_read_vreg(work_item, INST.data0);

	if (addr.as_uint > MIN(work_item->work_group->ndrange->local_mem_top, 
		si_isa_read_sreg(work_item, SI_M0)))
	{
		fatal("%s: invalid address\n", __FUNCTION__);
	}

	/* Global data store not supported */
	assert(!INST.gds);

	/* Write Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_write(work_item->work_group->lds_module, addr.as_uint, 4, 
			&data0.as_uint);
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
		work_item->lds_access_addr[0] = addr.as_uint;
		work_item->lds_access_size[0] = 4;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category) && INST.gds)
	{
		si_isa_debug("t%d: GDS[%u]<=(%u,%f) ", work_item->id, 
			addr.as_uint, data0.as_uint, data0.as_float);
	}
	else
	{
		si_isa_debug("t%d: LDS[%u]<=(%u,%f) ", work_item->id, 
			addr.as_uint, data0.as_uint, data0.as_float);
	}
}
#undef INST

/* DS[A] = D0[7:0]; byte write.  */
#define INST SI_INST_DS
void si_isa_DS_WRITE_B8_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t addr;
	union si_reg_t data0;

	assert(!INST.offset0);
	assert(!INST.offset1);
	assert(!INST.gds);

	/* Load address and data from registers. */
	addr.as_uint = si_isa_read_vreg(work_item, INST.addr);
	data0.as_uint = si_isa_read_vreg(work_item, INST.data0);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Write Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_write(work_item->work_group->lds_module, addr.as_uint, 1, 
			&data0.as_ubyte[0]);
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
		work_item->lds_access_addr[0] = addr.as_uint;
		work_item->lds_access_size[0] = 1;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category) && INST.gds)
	{
		si_isa_debug("t%d: GDS[%u]<=(0x%x) ", work_item->id, 
			addr.as_uint, data0.as_ubyte[0]);
	}
	else
	{
		si_isa_debug("t%d: LDS[%u]<=(0x%x) ", work_item->id, 
			addr.as_uint, data0.as_ubyte[0]);
	}
}
#undef INST

/* DS[A] = D0[15:0]; short write.  */
#define INST SI_INST_DS
void si_isa_DS_WRITE_B16_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t addr;
	union si_reg_t data0;

	assert(!INST.offset0);
	assert(!INST.offset1);
	assert(!INST.gds);

	/* Load address and data from registers. */
	addr.as_uint = si_isa_read_vreg(work_item, INST.addr);
	data0.as_uint = si_isa_read_vreg(work_item, INST.data0);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Write Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_write(work_item->work_group->lds_module, addr.as_uint, 2, 
			&data0.as_ushort[0]);
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
		work_item->lds_access_addr[0] = addr.as_uint;
		work_item->lds_access_size[0] = 2;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category) && INST.gds)
	{
		si_isa_debug("t%d: GDS[%u]<=(0x%x) ", work_item->id, 
			addr.as_uint, data0.as_ushort[0]);
	}
	else
	{
		si_isa_debug("t%d: LDS[%u]<=(0x%x) ", work_item->id, 
			addr.as_uint, data0.as_ushort[0]);
	}

}
#undef INST

/* R = DS[A]; Dword read. */
#define INST SI_INST_DS
void si_isa_DS_READ_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t addr;
	union si_reg_t data;

	assert(!INST.offset0);
	//assert(!INST.offset1);
	assert(!INST.gds);

	/* Load address from register. */
	addr.as_uint = si_isa_read_vreg(work_item, INST.addr);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Read Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_read(work_item->work_group->lds_module, addr.as_uint, 4,
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
		work_item->lds_access_addr[0] = addr.as_uint;
		work_item->lds_access_size[0] = 4;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x)(0x%x) ", work_item->id, 
			INST.vdst, addr.as_uint, data.as_uint);
	}

}
#undef INST

/* R = DS[ADDR+offset0*4], R+1 = DS[ADDR+offset1*4]. Read 2 Dwords. */
#define INST SI_INST_DS
void si_isa_DS_READ2_B32_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t addr;
	union si_reg_t data0;
	union si_reg_t data1;

	assert(!INST.gds);

	/* Load address from register. */
	addr.as_uint = si_isa_read_vreg(work_item, INST.addr);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Read Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_read(work_item->work_group->lds_module, 
			addr.as_uint + INST.offset0*4, 4, &data0.as_uint);
		mem_read(work_item->work_group->lds_module, 
			addr.as_uint + INST.offset1*4, 4, &data1.as_uint);
	}

	/* Write results. */
	si_isa_write_vreg(work_item, INST.vdst, data0.as_uint);
	si_isa_write_vreg(work_item, INST.vdst+1, data1.as_uint);

	/* Record last memory access for the detailed simulator. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		/* If offset1 != 1, then the following is incorrect */
		assert(INST.offset0 == 0);
		assert(INST.offset1 == 1);
		work_item->lds_access_count = 2;
		work_item->lds_access_type[0] = 1;
		work_item->lds_access_addr[0] = addr.as_uint;
		work_item->lds_access_size[0] = 4;
		work_item->lds_access_type[1] = 1;
		work_item->lds_access_addr[1] = addr.as_uint + 4;
		work_item->lds_access_size[1] = 4;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x)(0x%x) ", work_item->id, 
			INST.vdst, addr.as_uint+INST.offset0*4, 
			data0.as_uint);
		si_isa_debug("V%u<=(0x%x)(0x%x) ", INST.vdst+1, 
			addr.as_uint+INST.offset1*4, data1.as_uint);
	}
}
#undef INST

/* R = signext(DS[A][7:0]}; signed byte read. */
#define INST SI_INST_DS
void si_isa_DS_READ_I8_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t addr;
	union si_reg_t data;

	assert(!INST.offset0);
	assert(!INST.offset1);
	assert(!INST.gds);

	/* Load address from register. */
	addr.as_uint = si_isa_read_vreg(work_item, INST.addr);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Read Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_read(work_item->work_group->lds_module, addr.as_uint, 1,
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
		work_item->lds_access_addr[0] = addr.as_uint;
		work_item->lds_access_size[0] = 1;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x)(%d) ", work_item->id, INST.vdst,
			addr.as_uint, data.as_int);
	}
}
#undef INST

/* R = {24’h0,DS[A][7:0]}; unsigned byte read. */
#define INST SI_INST_DS
void si_isa_DS_READ_U8_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t addr;
	union si_reg_t data;

	assert(!INST.offset0);
	assert(!INST.offset1);
	assert(!INST.gds);

	/* Load address from register. */
	addr.as_uint = si_isa_read_vreg(work_item, INST.addr);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Read Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_read(work_item->work_group->lds_module, addr.as_uint, 1,
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
		work_item->lds_access_addr[0] = addr.as_uint;
		work_item->lds_access_size[0] = 1;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x)(%u) ", work_item->id, INST.vdst,
			addr.as_uint, data.as_uint);
	}
}
#undef INST

/* R = signext(DS[A][15:0]}; signed short read. */
#define INST SI_INST_DS
void si_isa_DS_READ_I16_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t addr;
	union si_reg_t data;

	assert(!INST.offset0);
	assert(!INST.offset1);
	assert(!INST.gds);

	/* Load address from register. */
	addr.as_uint = si_isa_read_vreg(work_item, INST.addr);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Read Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_read(work_item->work_group->lds_module, addr.as_uint, 2,
			&data.as_short[0]);
	}

	/* Extend the sign. */
	data.as_int = (int) data.as_short[0];

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
		work_item->lds_access_addr[0] = addr.as_uint;
		work_item->lds_access_size[0] = 2;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x)(%d) ", work_item->id, INST.vdst,
			addr.as_uint, data.as_int);
	}

}
#undef INST

/* R = {16’h0,DS[A][15:0]}; unsigned short read. */
#define INST SI_INST_DS
void si_isa_DS_READ_U16_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	union si_reg_t addr;
	union si_reg_t data;

	assert(!INST.offset0);
	assert(!INST.offset1);
	assert(!INST.gds);

	/* Load address from register. */
	addr.as_uint = si_isa_read_vreg(work_item, INST.addr);

	/* Global data store not supported */
	assert(!INST.gds);

	/* Read Dword. */
	if (INST.gds)
	{
		assert(0);
	}
	else
	{
		mem_read(work_item->work_group->lds_module, addr.as_uint, 2,
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
		work_item->lds_access_addr[0] = addr.as_uint;
		work_item->lds_access_size[0] = 2;
	}

	/* Print isa debug information. */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(0x%x)(%u) ", work_item->id, INST.vdst,
			addr.as_uint, data.as_uint);
	}
}
#undef INST

/*
 * MUBUF
 */

#define INST SI_INST_MUBUF
void si_isa_BUFFER_LOAD_SBYTE_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);
	assert(!INST.glc);
	assert(!INST.slc);
	assert(!INST.tfe);
	assert(!INST.lds);

	struct si_buffer_desc_t buf_desc;
	union si_reg_t value;

	unsigned int addr;
	unsigned int base;
	unsigned int mem_offset = 0;
	unsigned int inst_offset = 0;
	unsigned int off_vgpr = 0;
	unsigned int stride = 0;
	unsigned int idx_vgpr = 0;

	int bytes_to_read = 1;

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	/* Figure 8.1 from SI ISA defines address calculation */
	base = buf_desc.base_addr;
	mem_offset = si_isa_read_sreg(work_item, INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	/* Table 8.3 from SI ISA */
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + work_item->id_in_wavefront);

	mem_read(si_emu->global_mem, addr, bytes_to_read, &value);
	
	/* Sign extend */
	value.as_int = (int) value.as_byte[0];

	si_isa_write_vreg(work_item, INST.vdata, value.as_uint);

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
	work_item->global_mem_access_size = bytes_to_read;

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u)(%d) ", work_item->id,
			INST.vdata, addr, value.as_int);
	}
}
#undef INST

#define INST SI_INST_MUBUF
void si_isa_BUFFER_LOAD_DWORD_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);
	assert(!INST.glc);
	assert(!INST.slc);
	assert(!INST.tfe);
	assert(!INST.lds);

	struct si_buffer_desc_t buf_desc;
	union si_reg_t value;

	unsigned int addr;
	unsigned int base;
	unsigned int mem_offset = 0;
	unsigned int inst_offset = 0;
	unsigned int off_vgpr = 0;
	unsigned int stride = 0;
	unsigned int idx_vgpr = 0;

	int bytes_to_read = 4;

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	/* Figure 8.1 from SI ISA defines address calculation */
	base = buf_desc.base_addr;
	mem_offset = si_isa_read_sreg(work_item, INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	/* Table 8.3 from SI ISA */
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + work_item->id_in_wavefront);

	mem_read(si_emu->global_mem, addr, bytes_to_read, &value);
	
	/* Sign extend */
	value.as_int = (int) value.as_byte[0];

	si_isa_write_vreg(work_item, INST.vdata, value.as_uint);

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
	work_item->global_mem_access_size = bytes_to_read;

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u)(%d) ", work_item->id,
			INST.vdata, addr, value.as_int);
	}
}
#undef INST

#define INST SI_INST_MUBUF
void si_isa_BUFFER_STORE_BYTE_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);
	assert(!INST.slc);
	assert(!INST.tfe);
	assert(!INST.lds);

	struct si_buffer_desc_t buf_desc;
	union si_reg_t value;

	unsigned int addr;
	unsigned int base;
	unsigned int mem_offset = 0;
	unsigned int inst_offset = 0;
	unsigned int off_vgpr = 0;
	unsigned int stride = 0;
	unsigned int idx_vgpr = 0;

	int bytes_to_write = 1;

	if (INST.glc)
	{
		work_item->wavefront->vector_mem_glc = 1; // FIXME redundant
	}

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	/* Figure 8.1 from SI ISA defines address calculation */
	base = buf_desc.base_addr;
	mem_offset = si_isa_read_sreg(work_item, INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	/* Table 8.3 from SI ISA */
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + work_item->id_in_wavefront);

	value.as_int = si_isa_read_vreg(work_item, INST.vdata);

	mem_write(si_emu->global_mem, addr, bytes_to_write, &value);
	
	/* Sign extend */
	//value.as_int = (int) value.as_byte[0];

	si_isa_write_vreg(work_item, INST.vdata, value.as_uint);

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
	work_item->global_mem_access_size = bytes_to_write;

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u)(%d) ", work_item->id,
			INST.vdata, addr, value.as_int);
	}
}
#undef INST

#define INST SI_INST_MUBUF
void si_isa_BUFFER_STORE_DWORD_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);
	assert(!INST.slc);
	assert(!INST.tfe);
	assert(!INST.lds);

	struct si_buffer_desc_t buf_desc;
	union si_reg_t value;

	unsigned int addr;
	unsigned int base;
	unsigned int mem_offset = 0;
	unsigned int inst_offset = 0;
	unsigned int off_vgpr = 0;
	unsigned int stride = 0;
	unsigned int idx_vgpr = 0;

	int bytes_to_write = 4;

	if (INST.glc)
	{
		work_item->wavefront->vector_mem_glc = 1; // FIXME redundant
	}

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	/* Figure 8.1 from SI ISA defines address calculation */
	base = buf_desc.base_addr;
	mem_offset = si_isa_read_sreg(work_item, INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	/* Table 8.3 from SI ISA */
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + work_item->id_in_wavefront);

	value.as_int = si_isa_read_vreg(work_item, INST.vdata);

	mem_write(si_emu->global_mem, addr, bytes_to_write, &value);
	
	/* Sign extend */
	//value.as_int = (int) value.as_byte[0];

	si_isa_write_vreg(work_item, INST.vdata, value.as_uint);

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
	work_item->global_mem_access_size = bytes_to_write;

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u)(%d) ", work_item->id,
			INST.vdata, addr, value.as_int);
	}
}
#undef INST

#define INST SI_INST_MUBUF
void si_isa_BUFFER_ATOMIC_ADD_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	NOT_IMPL();
}
#undef INST

/*
 * MTBUF 
 */

#define INST SI_INST_MTBUF
void si_isa_TBUFFER_LOAD_FORMAT_X_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);
	assert(!INST.tfe);
	assert(!INST.slc);

	struct si_buffer_desc_t buf_desc;
	union si_reg_t value;

	int elem_size;
	int num_elems;
	int bytes_to_read;

	unsigned int addr;
	unsigned int base;
	unsigned int mem_offset = 0;
	unsigned int inst_offset = 0;
	unsigned int off_vgpr = 0;
	unsigned int stride = 0;
	unsigned int idx_vgpr = 0;

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);
	bytes_to_read = elem_size * num_elems;

	assert(num_elems == 1);
	assert(elem_size == 4);

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	/* Figure 8.1 from SI ISA defines address calculation */
	base = buf_desc.base_addr;
	mem_offset = si_isa_read_sreg(work_item, INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	/* Table 8.3 from SI ISA */
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	/* Calculate the address */
	/* XXX Need to know when to enable id_in_wavefront */
	addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + 0/*work_item->id_in_wavefront*/);

	mem_read(si_emu->global_mem, addr, bytes_to_read, &value);

	si_isa_write_vreg(work_item, INST.vdata, value.as_uint);

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
	work_item->global_mem_access_size = bytes_to_read;

	/* TODO Print value based on type */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u)(%u,%gf) ", work_item->id,
			INST.vdata, addr, value.as_uint, value.as_float);
		if (INST.offen)
			si_isa_debug("offen ");
		if (INST.idxen)
			si_isa_debug("idxen ");
		si_isa_debug("%u,%u,%u,%u,%u,%u ", base, mem_offset, 
			inst_offset, off_vgpr, idx_vgpr, stride);
	}
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_TBUFFER_LOAD_FORMAT_XY_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);

	struct si_buffer_desc_t buf_desc;
	union si_reg_t value;

	int i;
	int elem_size;
	int num_elems;
	int bytes_to_read;

	unsigned int addr;
	unsigned int base;
	unsigned int mem_offset = 0;
	unsigned int inst_offset = 0;
	unsigned int off_vgpr = 0;
	unsigned int stride = 0;
	unsigned int idx_vgpr = 0;

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);
	bytes_to_read = elem_size * num_elems;

	assert(num_elems == 2);
	assert(elem_size == 4);

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	/* Figure 8.1 from SI ISA defines address calculation */
	base = buf_desc.base_addr;
	mem_offset = si_isa_read_sreg(work_item, INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	/* Table 8.3 from SI ISA */
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	/* Calculate the address */
	/* XXX Need to know when to enable id_in_wavefront */
	addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + 0/*work_item->id_in_wavefront*/);

	for (i = 0; i < 2; i++)
	{
		mem_read(si_emu->global_mem, addr+4*i, 4, &value);

		si_isa_write_vreg(work_item, INST.vdata + i, value.as_uint);

		/* TODO Print value based on type */
		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("t%d: V%u<=(%u)(%u,%gf) ", work_item->id,
				INST.vdata + i, addr+4*i, value.as_uint,
				value.as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
	work_item->global_mem_access_size = bytes_to_read;
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_TBUFFER_LOAD_FORMAT_XYZW_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);

	struct si_buffer_desc_t buf_desc;
	union si_reg_t value;

	int i;
	int elem_size;
	int num_elems;
	int bytes_to_read;

	unsigned int addr;
	unsigned int base;
	unsigned int mem_offset = 0;
	unsigned int inst_offset = 0;
	unsigned int off_vgpr = 0;
	unsigned int stride = 0;
	unsigned int idx_vgpr = 0;

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);
	bytes_to_read = elem_size * num_elems;

	assert(num_elems == 4);
	assert(elem_size == 4);

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	/* Figure 8.1 from SI ISA defines address calculation */
	base = buf_desc.base_addr;
	mem_offset = si_isa_read_sreg(work_item, INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	/* Table 8.3 from SI ISA */
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	/* Calculate the address */
	/* XXX Need to know when to enable id_in_wavefront */
	addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + 0/*work_item->id_in_wavefront*/);

	for (i = 0; i < 4; i++)
	{
		mem_read(si_emu->global_mem, addr+4*i, 4, &value);

		si_isa_write_vreg(work_item, INST.vdata + i, value.as_uint);

		/* TODO Print value based on type */
		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("t%d: V%u<=(%u)(%u,%gf) ", work_item->id,
				INST.vdata + i, addr+4*i, value.as_uint,
				value.as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
	work_item->global_mem_access_size = bytes_to_read;
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_TBUFFER_STORE_FORMAT_X_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);

	struct si_buffer_desc_t buf_desc;
	union si_reg_t value;

	int elem_size;
	int num_elems;
	int bytes_to_write;

	unsigned int addr;
	unsigned int base;
	unsigned int mem_offset = 0;
	unsigned int inst_offset = 0;
	unsigned int off_vgpr = 0;
	unsigned int stride = 0;
	unsigned int idx_vgpr = 0;

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);
	bytes_to_write = elem_size * num_elems;

	assert(num_elems == 1);
	assert(elem_size == 4);

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	/* Figure 8.1 from SI ISA defines address calculation */
	base = buf_desc.base_addr;
	mem_offset = si_isa_read_sreg(work_item, INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	/* Table 8.3 from SI ISA */
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + work_item->id_in_wavefront);

	value.as_uint = si_isa_read_vreg(work_item, INST.vdata);

	mem_write(si_emu->global_mem, addr, bytes_to_write, &value);

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
	work_item->global_mem_access_size = bytes_to_write;

	/* TODO Print value based on type */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: (%u)<=V%u(%u,%gf) ", work_item->id,
			addr, INST.vdata, value.as_uint,
			value.as_float);
	}
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_TBUFFER_STORE_FORMAT_XY_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);

	struct si_buffer_desc_t buf_desc;
	union si_reg_t value;

	int elem_size;
	int num_elems;
	int bytes_to_write;

	unsigned int addr;
	unsigned int base;
	unsigned int mem_offset = 0;
	unsigned int inst_offset = 0;
	unsigned int off_vgpr = 0;
	unsigned int stride = 0;
	unsigned int idx_vgpr = 0;

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);
	bytes_to_write = elem_size * num_elems;

	assert(num_elems == 2);
	assert(elem_size == 4);

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	/* Figure 8.1 from SI ISA defines address calculation */
	base = buf_desc.base_addr;
	mem_offset = si_isa_read_sreg(work_item, INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	/* Table 8.3 from SI ISA */
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + work_item->id_in_wavefront);

	for (unsigned int i = 0; i < 2; i++)
	{
		value.as_uint = si_isa_read_vreg(work_item, INST.vdata + i);

		mem_write(si_emu->global_mem, addr+4*i, 4, &value);

		/* TODO Print value based on type */
		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("t%d: (%u)<=V%u(%u,%gf) ", work_item->id,
				addr, INST.vdata+i, value.as_uint,
				value.as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
	work_item->global_mem_access_size = bytes_to_write;
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_TBUFFER_STORE_FORMAT_XYZW_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	assert(!INST.addr64);

	struct si_buffer_desc_t buf_desc;
	union si_reg_t value;

	int elem_size;
	int num_elems;
	int bytes_to_write;

	unsigned int addr;
	unsigned int base;
	unsigned int mem_offset = 0;
	unsigned int inst_offset = 0;
	unsigned int off_vgpr = 0;
	unsigned int stride = 0;
	unsigned int idx_vgpr = 0;

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);
	bytes_to_write = elem_size * num_elems;

	assert(num_elems == 4);
	assert(elem_size == 4);

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc * 4);

	/* Figure 8.1 from SI ISA defines address calculation */
	base = buf_desc.base_addr;
	mem_offset = si_isa_read_sreg(work_item, INST.soffset);
	inst_offset = INST.offset;
	stride = buf_desc.stride;

	/* Table 8.3 from SI ISA */
	if (!INST.idxen && INST.offen)
	{
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && !INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
	}
	else if (INST.idxen && INST.offen)
	{
		idx_vgpr = si_isa_read_vreg(work_item, INST.vaddr);
		off_vgpr = si_isa_read_vreg(work_item, INST.vaddr + 1);
	}

	/* It wouldn't make sense to have a value for idxen without
	 * having a stride */
	if (idx_vgpr && !stride)
	{
		fatal("%s: the buffer descriptor is probably not correct",
			__FUNCTION__);
	}

	/* Calculate the address */
	addr = base + mem_offset + inst_offset + off_vgpr + 
		stride * (idx_vgpr + work_item->id_in_wavefront);

	for (unsigned int i = 0; i < 4; i++)
	{
		value.as_uint = si_isa_read_vreg(work_item, INST.vdata + i);

		mem_write(si_emu->global_mem, addr+4*i, 4, &value);

		/* TODO Print value based on type */
		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("t%d: (%u)<=V%u(%u,%gf) ", work_item->id,
				addr, INST.vdata+i, value.as_uint,
				value.as_float);
		}
	}

	/* Record last memory access for the detailed simulator. */
	work_item->global_mem_access_addr = addr;
	work_item->global_mem_access_size = bytes_to_write;
}
#undef INST

/*
 * MIMG
 */

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

/*
 * EXPORT
 */

#define INST SI_INST_EXP
void si_isa_EXPORT_impl(struct si_work_item_t *work_item,
	struct si_inst_t *inst)
{
	/* FIXME : not implemented */
}
#undef INST

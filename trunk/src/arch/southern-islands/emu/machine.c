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

#include <math.h>

#include <southern-islands-emu.h>
#include <mem-system.h>
#include <x86-emu.h>


char *err_si_isa_note =
	"\tThe AMD Southern Islands instruction set is partially supported by\n" 
	"\tMulti2Sim. If your program is using an unimplemented instruction,\n"
        "\tplease email development@multi2sim.org' to request support for it.\n";

#define NOT_IMPL() fatal("GPU instruction '%s' not implemented\n%s", \
	si_isa_inst->info->name, err_si_isa_note)

#define INST SI_INST_SMRD
void si_isa_S_BUFFER_LOAD_DWORD_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	union si_reg_t value;
	uint32_t m_offset;
	uint32_t m_base;
	//uint32_t m_size;
	struct si_buffer_resource_t buf_desc;
	int sbase;

	assert(INST.imm);

	sbase = INST.sbase << 1;

	/* sbase holds the first of 4 registers containing the buffer resource descriptor */
	si_isa_read_buf_res(work_item, &buf_desc, sbase);

	/* sgpr[dst] = read_dword_from_kcache(m_base, m_offset, m_size) */
	m_base = buf_desc.base_addr;
	m_offset = INST.offset * 4;
	//m_size = (buf_desc.stride == 0) ? 1 : buf_desc.num_records;
	
	mem_read(si_emu->global_mem, m_base+m_offset, 4, &value);

	/* Store the data in the destination register */
	si_isa_write_sreg(work_item, INST.sdst, value);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d,%gf)", INST.sdst, value.as_uint, value.as_float);
	}
}
#undef INST

#define INST SI_INST_SMRD
void si_isa_S_LOAD_DWORDX4_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	union si_reg_t value[4];
	uint32_t m_base;
	uint32_t m_offset;
	uint32_t m_addr;
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
		mem_read(si_emu->global_mem, m_base+m_offset+i*4, 4, &value[i]);
		si_isa_write_sreg(work_item, INST.sdst+i, value[i]);
	}	

	if (debug_status(si_isa_debug_category))
	{	
		for (i = 0; i < 4; i++) 
		{
			si_isa_debug("S%u<=(%d,%gf) ", INST.sdst+i, value[i].as_uint, 
				value[i].as_float);
		}
	}
}
#undef INST

#define INST SI_INST_SMRD
void si_isa_S_BUFFER_LOAD_DWORDX2_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	union si_reg_t value[2];
	uint32_t m_base;
	uint32_t m_offset;
	uint32_t m_addr;
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

	for (i = 0; i < 2; i++) 
	{
		mem_read(si_emu->global_mem, m_base+m_offset+i*4, 4, &value[i]);
		si_isa_write_sreg(work_item, INST.sdst+i, value[i]);
	}	

	if (debug_status(si_isa_debug_category))
	{
		for (i = 0; i < 2; i++) 
		{
			si_isa_debug("S%u<=(%d,%gf) ", INST.sdst+i, value[i].as_uint, 
				value[i].as_float);
		}
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_ADD_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.i + S1.i. scc = overflow. */

	int s0 = 0;
	int s1 = 0;
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0).as_int;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1).as_int;

	union si_reg_t sum;
	union si_reg_t ovf;

	sum.as_uint = s0 + s1;
	ovf.as_uint = ((long)(s0) + (long)(s1)) > (long)sum.as_uint;

	si_isa_write_sreg(work_item, INST.sdst, sum);
	si_isa_write_sreg(work_item, SI_SCC, ovf);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d) ", INST.sdst, sum.as_uint);
		si_isa_debug("scc<=(%d) ", ovf.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_MIN_U32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = (S0.u < S1.u) ? S0.u : S1.u, scc = 1 if S0 is min. */

	unsigned int s0 = 0;
	unsigned int s1 = 0;
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0).as_uint;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1).as_uint;

	union si_reg_t min;
	union si_reg_t s0_min;

	if (INST.ssrc0 < INST.ssrc1)
	{
		min.as_uint = s0;
		s0_min.as_uint = 1;
	}
	else
	{
		min.as_uint = s1;
		s0_min.as_uint = 0;
	}

	si_isa_write_sreg(work_item, INST.sdst, min);
	si_isa_write_sreg(work_item, SI_SCC, s0_min);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d) ", INST.sdst, min.as_uint);
		si_isa_debug("scc<=(%d) ", s0_min.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_AND_B32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.u & S1.u. scc = 1 if result is non-zero. */
	
	unsigned int s0 = 0;
	unsigned int s1 = 0;
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0).as_uint;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1).as_uint;

	union si_reg_t result;
	result.as_uint = s0 & s1;
	union si_reg_t nonzero;
	nonzero.as_uint = result.as_uint != 0;

	si_isa_write_sreg(work_item, INST.sdst, result);
	si_isa_write_sreg(work_item, SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d) ", INST.sdst, result.as_uint);
		si_isa_debug("scc<=(%d) ", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_AND_B64_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.u & S1.u. scc = 1 if result is non-zero. */

	assert(!(INST.ssrc0 == 0xFF || INST.ssrc1 == 0xFF));
	unsigned int s0_lo = si_isa_read_sreg(work_item, INST.ssrc0).as_uint;
	unsigned int s0_hi = si_isa_read_sreg(work_item, INST.ssrc0 + 1).as_uint;
	unsigned int s1_lo = si_isa_read_sreg(work_item, INST.ssrc1).as_uint;
	unsigned int s1_hi = si_isa_read_sreg(work_item, INST.ssrc1 + 1).as_uint;

	union si_reg_t result_lo;
	result_lo.as_uint = s0_lo & s1_lo;
	union si_reg_t result_hi;
	result_hi.as_uint = s0_hi & s1_hi;
	union si_reg_t nonzero;
	nonzero.as_uint = result_lo.as_uint && result_hi.as_uint;

	si_isa_write_sreg(work_item, INST.sdst, result_lo);
	si_isa_write_sreg(work_item, INST.sdst + 1, result_hi);
	si_isa_write_sreg(work_item, SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d) ", INST.sdst, result_lo.as_uint);
		si_isa_debug("S%u<=(%d) ", INST.sdst + 1, result_hi.as_uint);
		si_isa_debug("scc<=(%d) ", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_ANDN2_B64_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.u & ~S1.u. scc = 1 if result is non-zero. */

	assert(!(INST.ssrc0 == 0xFF || INST.ssrc1 == 0xFF));
	unsigned int s0_lo = si_isa_read_sreg(work_item, INST.ssrc0).as_uint;
	unsigned int s0_hi = si_isa_read_sreg(work_item, INST.ssrc0 + 1).as_uint;
	unsigned int s1_lo = si_isa_read_sreg(work_item, INST.ssrc1).as_uint;
	unsigned int s1_hi = si_isa_read_sreg(work_item, INST.ssrc1 + 1).as_uint;

	union si_reg_t result_lo;
	result_lo.as_uint = s0_lo & ~s1_lo;
	union si_reg_t result_hi;
	result_hi.as_uint = s0_hi & ~s1_hi;
	union si_reg_t nonzero;
	nonzero.as_uint = result_lo.as_uint && result_hi.as_uint;

	si_isa_write_sreg(work_item, INST.sdst, result_lo);
	si_isa_write_sreg(work_item, INST.sdst + 1, result_hi);
	si_isa_write_sreg(work_item, SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)= ", INST.sdst, result_lo.as_uint);
		si_isa_debug("S%u<=(%d)= ", INST.sdst + 1, result_hi.as_uint);
		si_isa_debug("scc<=(%d)", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_LSHL_B32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.u << S1.u[4:0]. scc = 1 if result is non-zero. */

	unsigned int s0 = 0;
	unsigned int s1 = 0;
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0).as_uint;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst & 0x1F;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1).as_uint & 0x1F;

	union si_reg_t result;
	result.as_uint = s0 << s1;
	union si_reg_t nonzero;
	nonzero.as_uint = result.as_uint != 0;

	si_isa_write_sreg(work_item, INST.sdst, result);
	si_isa_write_sreg(work_item, SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)= ", INST.sdst, result.as_uint);
		si_isa_debug("scc<=(%d)", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_LSHR_B32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.u >> S1.u[4:0]. scc = 1 if result is non-zero. */

	unsigned int s0 = 0;
	unsigned int s1 = 0;
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0).as_uint;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst & 0x1F;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1).as_uint & 0x1F;

	union si_reg_t result;
	result.as_uint = s0 >> s1;
	union si_reg_t nonzero;
	nonzero.as_uint = result.as_uint != 0;

	si_isa_write_sreg(work_item, INST.sdst, result);
	si_isa_write_sreg(work_item, SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)= ", INST.sdst, result.as_uint);
		si_isa_debug("scc<=(%d)", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_ASHR_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.i = signtext(S0.i) >> S1.i[4:0]. scc = 1 if result is non-zero. */

	int s0 = 0;
	int s1 = 0;
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0).as_int;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst & 0x1F;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1).as_int & 0x1F;
	long se_s0 = s0;

	union si_reg_t result;
	result.as_int = se_s0 >> s1;
	union si_reg_t nonzero;
	nonzero.as_uint = result.as_uint != 0;

	si_isa_write_sreg(work_item, INST.sdst, result);
	si_isa_write_sreg(work_item, SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)= ", INST.sdst, result.as_int);
		si_isa_debug("scc<=(%d)", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_MUL_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.i = S0.i * S1.i. */

	int s0 = 0;
	int s1 = 0;
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0).as_int;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1).as_int;

	union si_reg_t result;
	result.as_int = s0 * s1;

	si_isa_write_sreg(work_item, INST.sdst, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)= ", INST.sdst, result.as_int);
	}
}
#undef INST

#define INST SI_INST_SOPK
void si_isa_S_MOVK_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.i = signext(simm16). */

	short simm16 = INST.simm16;

	union si_reg_t result;
	result.as_int = simm16;

	si_isa_write_sreg(work_item, INST.sdst, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)", INST.sdst, result.as_int);
	}
}
#undef INST

#define INST SI_INST_SOPK
void si_isa_S_ADDK_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.i = D.i + signext(SIMM16). scc = overflow. */

	short simm16 = INST.simm16;
	int se_simm16 = simm16;
	int dest = si_isa_read_sreg(work_item, INST.sdst).as_int;

	union si_reg_t sum;
	sum.as_int = dest + se_simm16;
	union si_reg_t ovf;
	ovf.as_uint = ((long)dest + (long)se_simm16) > (long)sum.as_uint;

	si_isa_write_sreg(work_item, INST.sdst, sum);
	si_isa_write_sreg(work_item, SI_SCC, ovf);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)", INST.sdst, sum.as_int);
		si_isa_debug("scc<=(%d)", ovf.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOPK
void si_isa_S_MULK_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.i = D.i * signext(SIMM16). scc = overflow. */

	short simm16 = INST.simm16;
	int se_simm16 = simm16;
	int dest = si_isa_read_sreg(work_item, INST.sdst).as_int;

	union si_reg_t product;
	product.as_int = dest * se_simm16;
	union si_reg_t ovf;
	ovf.as_uint = ((long)dest * (long)se_simm16) > (long)product.as_uint;

	si_isa_write_sreg(work_item, INST.sdst, product);
	si_isa_write_sreg(work_item, SI_SCC, ovf);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d)", INST.sdst, product.as_int);
		si_isa_debug("scc<=(%d)", ovf.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP1
void si_isa_S_MOV_B64_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.u. */

	assert(!(INST.ssrc0 == 0xFF));
	union si_reg_t s0_lo;
	s0_lo = si_isa_read_sreg(work_item, INST.ssrc0);
	union si_reg_t s0_hi;
	s0_hi = si_isa_read_sreg(work_item, INST.ssrc0 + 1);

	si_isa_write_sreg(work_item, INST.sdst, s0_lo);
	si_isa_write_sreg(work_item, INST.sdst + 1, s0_hi);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d) ", INST.sdst, s0_lo.as_int);
		si_isa_debug("S%u<=(%d)", INST.sdst + 1, s0_hi.as_int);
	}
}
#undef INST

#define INST SI_INST_SOP1
void si_isa_S_MOV_B32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.u. */

	assert(!(INST.ssrc0 == 0xFF));
	union si_reg_t s0;
	s0 = si_isa_read_sreg(work_item, INST.ssrc0);

	si_isa_write_sreg(work_item, INST.sdst, s0);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d) ", INST.sdst, s0.as_int);
	}
}
#undef INST

#define INST SI_INST_SOP1
void si_isa_S_AND_SAVEEXEC_B64_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = EXEC, EXEC = S0.u & EXEC. scc = 1 if the new value of EXEC is non-zero. */

	assert(!(INST.ssrc0 == 0xFF));
	union si_reg_t exec_lo = si_isa_read_sreg(work_item, SI_EXEC);
	union si_reg_t exec_hi = si_isa_read_sreg(work_item, SI_EXEC + 1);
	unsigned int s0_lo = si_isa_read_sreg(work_item, INST.ssrc0).as_uint;
	unsigned int s0_hi = si_isa_read_sreg(work_item, INST.ssrc0 + 1).as_uint;

	union si_reg_t exec_new_lo;
	exec_new_lo.as_uint = s0_lo & exec_lo.as_uint;
	union si_reg_t exec_new_hi;
	exec_new_hi.as_uint = s0_hi & exec_hi.as_uint;
	union si_reg_t nonzero;
	nonzero.as_uint = exec_new_lo.as_uint && exec_new_hi.as_uint;

	si_isa_write_sreg(work_item, INST.sdst, exec_lo);
	si_isa_write_sreg(work_item, INST.sdst + 1, exec_hi);
	si_isa_write_sreg(work_item, SI_EXEC, exec_new_lo);
	si_isa_write_sreg(work_item, SI_EXEC + 1, exec_new_hi);
	si_isa_write_sreg(work_item, SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("S%u<=(%d) ", INST.sdst, exec_lo.as_uint);
		si_isa_debug("S%u<=(%d) ", INST.sdst + 1, exec_hi.as_uint);
		si_isa_debug("exec_lo<=(%d) ", exec_new_lo.as_uint);
		si_isa_debug("exec_hi<=(%d) ", exec_new_hi.as_uint);
		si_isa_debug("scc<=(%d)", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOPC
void si_isa_S_CMP_EQ_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* scc = (S0.i == S1.i). */

	int s0 = 0;
	int s1 = 0;
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0).as_int;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1).as_int;

	union si_reg_t equal;
	equal.as_uint = s0 == s1;

	si_isa_write_sreg(work_item, SI_SCC, equal);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("scc<=(%d) ", equal.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOPC
void si_isa_S_CMP_LE_U32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* scc = (S0.u <= S1.u). */

	unsigned int s0 = 0;
	unsigned int s1 = 0;
	assert(!(INST.ssrc0 == 0xFF && INST.ssrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_sreg(work_item, INST.ssrc0).as_uint;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_sreg(work_item, INST.ssrc1).as_uint;

	union si_reg_t le;
	le.as_uint = s0 <= s1;

	si_isa_write_sreg(work_item, SI_SCC, le);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("scc<=(%d) ", le.as_uint);
	}
}
#undef INST

void si_isa_S_ENDPGM_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	work_item->wavefront->finished = 1;
}

#define INST SI_INST_SOPP
void si_isa_S_BRANCH_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* PC = PC + signext(SIMM16 * 4) + 4 */

	long pc = work_item->wavefront->inst_buf - work_item->wavefront->inst_buf_start;
	short simm16 = INST.simm16;
	int se_simm16 = simm16;

	pc = pc + (se_simm16 * 4) + 4;

	work_item->wavefront->inst_buf = work_item->wavefront->inst_buf_start + pc;
}
#undef INST

#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_SCC0_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* if(SCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */

	if(!si_isa_read_sreg(work_item, SI_SCC).as_uint)
	{
		long pc = work_item->wavefront->inst_buf - work_item->wavefront->inst_buf_start;
		short simm16 = INST.simm16;
		int se_simm16 = simm16;

		pc = pc + (se_simm16 * 4) + 4;

		work_item->wavefront->inst_buf = work_item->wavefront->inst_buf_start + pc;
	}
}
#undef INST

#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_SCC1_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* if(SCC == 1) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */

	if(si_isa_read_sreg(work_item, SI_SCC).as_uint)
	{
		long pc = work_item->wavefront->inst_buf - work_item->wavefront->inst_buf_start;
		short simm16 = INST.simm16;
		int se_simm16 = simm16;

		pc = pc + (se_simm16 * 4) + 4;

		work_item->wavefront->inst_buf = work_item->wavefront->inst_buf_start + pc;
	}
}
#undef INST

#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_VCCZ_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* if(VCC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */

	if(si_isa_read_sreg(work_item, SI_VCCZ).as_uint)
	{
		long pc = work_item->wavefront->inst_buf - work_item->wavefront->inst_buf_start;
		short simm16 = INST.simm16;
		int se_simm16 = simm16;

		pc = pc + (se_simm16 * 4) + 4;

		work_item->wavefront->inst_buf = work_item->wavefront->inst_buf_start + pc;
	}
}
#undef INST

#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_EXECZ_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* if(EXEC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */

	if(si_isa_read_sreg(work_item, SI_EXECZ).as_uint)
	{
		long pc = work_item->wavefront->inst_buf - work_item->wavefront->inst_buf_start;
		short simm16 = INST.simm16;
		int se_simm16 = simm16;

		pc = pc + (se_simm16 * 4) + 4;

		work_item->wavefront->inst_buf = work_item->wavefront->inst_buf_start + pc;
	}
}
#undef INST

void si_isa_S_BARRIER_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	list_add(work_item->work_group->barrier_list, work_item->wavefront);
	if(work_item->work_group->barrier_list->count == work_item->work_group->wavefront_count)
		list_clear(work_item->work_group->barrier_list);
}

void si_isa_S_WAITCNT_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* Nothing to do in emulation */
}

#define INST SI_INST_VOP1
void si_isa_V_MOV_B32_VOP1_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	union si_reg_t value;

	if (INST.src0 == 0xFF)
		value.as_uint = INST.lit_cnst;
	else
		value = si_isa_read_reg(work_item, INST.src0);

	si_isa_write_vreg(work_item, INST.vdst, value);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, value.as_int);
	}
}
#undef INST

#define INST SI_INST_VOP1
void si_isa_V_CVT_F32_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.f = (float)S0.i. */

	union si_reg_t value;

	if (INST.src0 == 0xFF)
		value.as_float = (float)INST.lit_cnst;
	else
		value.as_float = (float)si_isa_read_reg(work_item, INST.src0).as_int;

	si_isa_write_vreg(work_item, INST.vdst, value);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst, value.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP1
void si_isa_V_CVT_F32_U32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.f = (float)S0.u. */

	union si_reg_t value;

	if (INST.src0 == 0xFF)
		value.as_float = (float)INST.lit_cnst;
	else
		value.as_float = (float)si_isa_read_reg(work_item, INST.src0).as_uint;

	si_isa_write_vreg(work_item, INST.vdst, value);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst, value.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP1
void si_isa_V_CVT_U32_F32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.i = (uint)S0.f. */

	union si_reg_t value;

	if (INST.src0 == 0xFF)
		value.as_uint = (unsigned int)INST.lit_cnst;
	else
		value.as_uint = (unsigned int)si_isa_read_reg(work_item, INST.src0).as_float;

	si_isa_write_vreg(work_item, INST.vdst, value);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, value.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP1
void si_isa_V_CVT_I32_F32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.i = (int)S0.f. */

	union si_reg_t value;

	if (INST.src0 == 0xFF)
		value.as_int = (int)INST.lit_cnst;
	else
		value.as_int = (int)si_isa_read_reg(work_item, INST.src0).as_float;

	si_isa_write_vreg(work_item, INST.vdst, value);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, value.as_int);
	}
}
#undef INST

#define INST SI_INST_VOP1
void si_isa_V_RCP_F32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.f = 1.0 / S0.f. */

	union si_reg_t rcp;

	if (INST.src0 == 0xFF)
		rcp.as_float = 1.0 / (float)INST.lit_cnst;
	else
		rcp.as_float = 1.0 / si_isa_read_reg(work_item, INST.src0).as_float;

	si_isa_write_vreg(work_item, INST.vdst, rcp);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst, rcp.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_ADD_F32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.f = S0.f + S1.f. */

	float s0 = 0;
	float s1 = 0;
	assert(!(INST.src0 == 0xFF && INST.vsrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0).as_float;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_vreg(work_item, INST.vsrc1).as_float;

	union si_reg_t sum;
	sum.as_float = s0 + s1;

	si_isa_write_vreg(work_item, INST.vdst, sum);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst, sum.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_MUL_F32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.f = S0.f * S1.f. */
	
	float s0 = 0;
	float s1 = 0;
	assert(!(INST.src0 == 0xFF && INST.vsrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0).as_float;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_vreg(work_item, INST.vsrc1).as_float;

	union si_reg_t product;
	product.as_float = s0 * s1;

	si_isa_write_vreg(work_item, INST.vdst, product);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst, product.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_MUL_I32_I24_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{	
	/* D.i = S0.i[23:0] * S1.i[23:0]. */
	
	int s0 = 0;
	int s1 = 0;
	assert(!(INST.src0 == 0xFF && INST.vsrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst & 0xFFFFFF;
	else
		s0 = si_isa_read_reg(work_item, INST.src0).as_int & 0xFFFFFF;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst & 0xFFFFFF;
	else
		s1 = si_isa_read_vreg(work_item, INST.vsrc1).as_int & 0xFFFFFF;

	union si_reg_t product;
	product.as_int = s0 * s1;

	si_isa_write_vreg(work_item, INST.vdst, product);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d)", work_item->id, INST.vdst, product.as_int);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_LSHLREV_B32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S1.u << S0.u[4:0]. */

	unsigned int s0 = 0;
	unsigned int s1 = 0;
	assert(!(INST.src0 == 0xFF && INST.vsrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst & 0x1F;
	else
		s0 = si_isa_read_reg(work_item, INST.src0).as_uint & 0x1F;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_vreg(work_item, INST.vsrc1).as_uint;

	union si_reg_t result;
	result.as_uint = s1 << s0;

	si_isa_write_vreg(work_item, INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_OR_B32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.u | S1.u. */

	unsigned int s0 = 0;
	unsigned int s1 = 0;
	assert(!(INST.src0 == 0xFF && INST.vsrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0).as_uint;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_vreg(work_item, INST.vsrc1).as_uint;

	union si_reg_t result;
	result.as_uint = s0 | s1;

	si_isa_write_vreg(work_item, INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_MAC_F32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.f = S0.f * S1.f + D.f. */

	float s0 = 0;
	float s1 = 0;
	assert(!(INST.src0 == 0xFF && INST.vsrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0).as_float;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_vreg(work_item, INST.vsrc1).as_float;
	float d = si_isa_read_vreg(work_item, INST.vdst).as_float;

	union si_reg_t result;
	result.as_float = s0 * s1 + d;

	si_isa_write_vreg(work_item, INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst, result.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_ADD_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.u + S1.u, vcc = carry-out. */

	unsigned int s0 = 0;
	unsigned int s1 = 0;
	assert(!(INST.src0 == 0xFF && INST.vsrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0).as_uint;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_vreg(work_item, INST.vsrc1).as_uint;

	union si_reg_t sum;
	sum.as_uint = s0 + s1;
	union si_reg_t carry;
	carry.as_uint = (((long)s0 + (long)s1) > 0xFFFFFFFF);

	si_isa_write_vreg(work_item, INST.vdst, sum);
	si_isa_bitmask_sreg(work_item, SI_VCC, carry);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, sum.as_uint);
		si_isa_debug("wf_id%d: vcc<=(%d) ", work_item->id_in_wavefront, carry.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_SUB_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.u - S1.u; vcc = carry-out. */

	unsigned int s0 = 0;
	unsigned int s1 = 0;
	assert(!(INST.src0 == 0xFF && INST.vsrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0).as_uint;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_vreg(work_item, INST.vsrc1).as_uint;

	union si_reg_t dif;
	dif.as_uint = s0 - s1;
	union si_reg_t carry;
	carry.as_uint = (((long)s0 - (long)s1) != (long)dif.as_uint);

	si_isa_write_vreg(work_item, INST.vdst, dif);
	si_isa_bitmask_sreg(work_item, SI_VCC, carry);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, dif.as_uint);
		si_isa_debug("wf_id%d: vcc<=(%d) ", work_item->id_in_wavefront, carry.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP3a
void si_isa_V_CNDMASK_B32_VOP3a_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = VCC[i] ? S1.u : S0.u (i = threadID in wave); VOP3: specify VCC as a scalar GPR in S2. */

	union si_reg_t s0 = si_isa_read_reg(work_item, INST.src0);
	union si_reg_t s1 = si_isa_read_reg(work_item, INST.src1);
	int vcci = si_isa_read_bitmask_sreg(work_item, INST.src2);
	union si_reg_t result = (vcci) ? s1 : s0;

	assert(!INST.neg);

	si_isa_write_vreg(work_item, INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP3a
void si_isa_V_MAD_F32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.f = S0.f * S1.f + S2.f. */

	float s0 = si_isa_read_reg(work_item, INST.src0).as_float;
	float s1 = si_isa_read_reg(work_item, INST.src1).as_float;
	float s2 = si_isa_read_reg(work_item, INST.src2).as_float;

	if(INST.neg & 1)
		s0 = -s0;
	if(INST.neg & 2)
		s1 = -s1;
	if(INST.neg & 4)
		s2 = -s2;

	union si_reg_t result;
	result.as_float = s0 * s1 + s2;

	si_isa_write_vreg(work_item, INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%gf) ", work_item->id, INST.vdst, result.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP3a
void si_isa_V_MUL_LO_U32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.u * S1.u. */

	unsigned int s0 = si_isa_read_reg(work_item, INST.src0).as_uint;
	unsigned int s1 = si_isa_read_reg(work_item, INST.src1).as_uint;

	if(INST.neg & 1)
		s0 = -s0;
	if(INST.neg & 2)
		s1 = -s1;

	union si_reg_t result;
	result.as_uint = s0 * s1;

	si_isa_write_vreg(work_item, INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP3a
void si_isa_V_MUL_HI_U32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = (S0.u * S1.u)>>32 */

	unsigned int s0 = si_isa_read_reg(work_item, INST.src0).as_uint;
	unsigned int s1 = si_isa_read_reg(work_item, INST.src1).as_uint;

	if(INST.neg & 1)
		s0 = -s0;
	if(INST.neg & 2)
		s1 = -s1;

	union si_reg_t result;
	//result.as_uint = ((long)s0 * (long)s1) >> 32;

	si_isa_write_vreg(work_item, INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP3a
void si_isa_V_MUL_LO_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.i = S0.i * S1.i. */

	int s0 = si_isa_read_reg(work_item, INST.src0).as_int;
	int s1 = si_isa_read_reg(work_item, INST.src1).as_int;

	if(INST.neg & 1)
		s0 = -s0;
	if(INST.neg & 2)
		s1 = -s1;

	union si_reg_t result;
	result.as_int = s0 * s1;

	si_isa_write_vreg(work_item, INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, result.as_int);
	}
}
#undef INST

#define INST SI_INST_VOPC
void si_isa_V_CMP_LT_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* vcc = (S0.i < S1.i). */

	int s0 = 0;
	int s1 = 0;
	assert(!(INST.src0 == 0xFF && INST.vsrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0).as_int;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_vreg(work_item, INST.vsrc1).as_int;

	union si_reg_t result;
	result.as_uint = (s0 < s1);

	si_isa_bitmask_sreg(work_item, SI_VCC, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%d) ", work_item->id_in_wavefront,
			result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOPC
void si_isa_V_CMP_GT_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* vcc = (S0.i > S1.i). */

	int s0 = 0;
	int s1 = 0;
	assert(!(INST.src0 == 0xFF && INST.vsrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0).as_int;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_vreg(work_item, INST.vsrc1).as_int;

	union si_reg_t result;
	result.as_uint = (s0 > s1);

	si_isa_bitmask_sreg(work_item, SI_VCC, result);
	
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%d) ", work_item->id_in_wavefront,
			result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOPC
void si_isa_V_CMP_NE_I32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* vcc = (S0.i <> S1.i). */

	int s0 = 0;
	int s1 = 0;
	assert(!(INST.src0 == 0xFF && INST.vsrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0).as_int;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_vreg(work_item, INST.vsrc1).as_int;

	union si_reg_t result;
	result.as_uint = (s0 != s1);

	si_isa_bitmask_sreg(work_item, SI_VCC, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%d) ", work_item->id_in_wavefront,
			result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOPC
void si_isa_V_CMP_LE_U32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* vcc = (S0.u <= S1.u). */

	unsigned int s0 = 0;
	unsigned int s1 = 0;
	assert(!(INST.src0 == 0xFF && INST.vsrc1 == 0xFF));
	if (s0 == 0xFF)
		s0 = INST.lit_cnst;
	else
		s0 = si_isa_read_reg(work_item, INST.src0).as_uint;
	if (s1 == 0xFF)
		s1 = INST.lit_cnst;
	else
		s1 = si_isa_read_vreg(work_item, INST.vsrc1).as_uint;

	union si_reg_t result;
	result.as_uint = (s0 <= s1);

	si_isa_bitmask_sreg(work_item, SI_VCC, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: vcc<=(%d) ", work_item->id_in_wavefront,
			result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP3b
void si_isa_V_CMP_GT_I32_VOP3b_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = (S0.i > S1.i). */

	int s0 = si_isa_read_reg(work_item, INST.src0).as_int;
	int s1 = si_isa_read_reg(work_item, INST.src1).as_int;

	if(INST.neg & 1)
		s0 = -s0;
	if(INST.neg & 2)
		s1 = -s1;

	union si_reg_t result;
	result.as_uint = (s0 > s1);

	si_isa_bitmask_sreg(work_item, INST.vdst, result);
	
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%d:+1]<=(%d) ", work_item->id_in_wavefront,
			INST.vdst, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP3b
void si_isa_V_CMP_NE_I32_VOP3b_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = (S0.i <> S1.i). */

	int s0 = si_isa_read_reg(work_item, INST.src0).as_int;
	int s1 = si_isa_read_reg(work_item, INST.src1).as_int;

	if(INST.neg & 1)
		s0 = -s0;
	if(INST.neg & 2)
		s1 = -s1;

	union si_reg_t result;
	result.as_uint = (s0 != s1);

	si_isa_bitmask_sreg(work_item, INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%d:+1]<=(%d) ", work_item->id_in_wavefront,
			INST.vdst, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP3b
void si_isa_V_CMP_GE_U32_VOP3b_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = (S0.u >= S1.u). */

	unsigned int s0 = si_isa_read_reg(work_item, INST.src0).as_uint;
	unsigned int s1 = si_isa_read_reg(work_item, INST.src1).as_uint;

	if(INST.neg & 1)
		s0 = -s0;
	if(INST.neg & 2)
		s1 = -s1;

	union si_reg_t result;
	result.as_uint = (s0 >= s1);

	si_isa_bitmask_sreg(work_item, INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wf_id%d: S[%d:+1]<=(%d) ", work_item->id_in_wavefront,
			INST.vdst, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP3b
void si_isa_V_ADDC_U32_VOP3b_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* D.u = S0.u + S1.u + VCC; VCC=carry-out (VOP3:sgpr=carry-out, S2.u=carry-in). */

	unsigned int s0 = si_isa_read_reg(work_item, INST.src0).as_uint;
	unsigned int s1 = si_isa_read_reg(work_item, INST.src1).as_uint;

	if(INST.neg & 1)
		s0 = -s0;
	if(INST.neg & 2)
		s1 = -s1;
	assert(!(INST.neg & 4));

	unsigned int carry_in = si_isa_read_bitmask_sreg(work_item, INST.src2);

	union si_reg_t sum;
	sum.as_uint = s0 + s1 + carry_in;
	union si_reg_t carry_out;
	carry_out.as_uint = (((long)s0 + (long)s1 + (long)carry_in) > 0xFFFFFFFF);

	si_isa_write_vreg(work_item, INST.vdst, sum);
	si_isa_bitmask_sreg(work_item, INST.sdst, carry_out);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, sum.as_uint);
		si_isa_debug("wf_id%d: vcc<=(%d) ", work_item->id_in_wavefront, carry_out.as_uint);
	}
}
#undef INST

#define INST SI_INST_DS
void si_isa_DS_WRITE_B32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* DS[A] = D0; write a Dword. */

	unsigned int addr = si_isa_read_vreg(work_item, INST.addr).as_uint;
	unsigned int data0 = si_isa_read_vreg(work_item, INST.data0).as_uint;

	if(INST.gds)
	{
		mem_write(si_emu->global_mem, addr, 4, &data0);
	}
	else
	{
		mem_write(work_item->work_group->local_mem, addr, 4, &data0);
	}

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("GDS?:%d DS[%d]<=(%d) ", INST.gds, addr, data0);
	}
}
#undef INST

#define INST SI_INST_DS
void si_isa_DS_READ_B32_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	/* R = DS[A]; Dword read. */

	unsigned int addr = si_isa_read_vreg(work_item, INST.addr).as_uint;
	union si_reg_t data;

	if(INST.gds)
	{
		mem_read(si_emu->global_mem, addr, 4, &data.as_uint);
	}
	else
	{
		mem_read(work_item->work_group->local_mem, addr, 4, &data.as_uint);
	}

	si_isa_write_vreg(work_item, INST.vdst, data);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%d) ", work_item->id, INST.vdst, data.as_uint);
	}
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_T_BUFFER_LOAD_FORMAT_X_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	assert(!INST.addr64);
	assert(!INST.index);

	unsigned int offset; 
	int elem_size;
	int num_elems;
	int bytes_to_read;
	struct si_buffer_resource_t buf_desc;
	uint32_t buffer_addr;
	union si_reg_t value;

	if (INST.offen)
	{
		offset = si_isa_read_vreg(work_item, INST.vaddr).as_uint;
	}
	else 
	{
		offset = INST.offset;
	}

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);

	/* If num_elems is greater than 1, we need to see how 
	 * the destination register is handled */
	assert(num_elems == 1);

	bytes_to_read = elem_size * num_elems;

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc*4);

	buffer_addr = offset;

	mem_read(si_emu->global_mem, buffer_addr, bytes_to_read, &value);

	si_isa_write_vreg(work_item, INST.vdata, value);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: V%u<=(%u)(%d,%gf) ", work_item->id, INST.vdata,
			buffer_addr, value.as_uint, value.as_float);
	}
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_T_BUFFER_LOAD_FORMAT_XYZW_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	assert(!INST.addr64);
	assert(!INST.index);

	unsigned int offset;
	int elem_size;
	int num_elems;
	int bytes_to_read;
	struct si_buffer_resource_t buf_desc;
	uint32_t buffer_addr;
	union si_reg_t value;

	if (INST.offen)
	{
		offset = si_isa_read_vreg(work_item, INST.vaddr).as_uint;
	}
	else
	{
		offset = INST.offset;
	}

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);

	/* If num_elems is greater than 1, we need to see how
	 * the destination register is handled */
	assert(num_elems == 1);

	bytes_to_read = elem_size * num_elems;

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc*4);

	for(int i = 0; i < 4; i++)
	{
		buffer_addr = offset + 4*i;

		mem_read(si_emu->global_mem, buffer_addr, bytes_to_read, &value);

		si_isa_write_vreg(work_item, INST.vdata + i, value);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("t%d: V%u<=(%u)(%d,%gf) ", work_item->id, INST.vdata + i,
				buffer_addr, value.as_uint, value.as_float);
		}
	}
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_T_BUFFER_STORE_FORMAT_X_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	assert(!INST.addr64);
	assert(!INST.index);

	unsigned int offset; 
	int elem_size;
	int num_elems;
	int bytes_to_write;
	struct si_buffer_resource_t buf_desc;
	uint32_t buffer_addr;
	union si_reg_t value;

	if (INST.offen)
	{
		offset = si_isa_read_vreg(work_item, INST.vaddr).as_uint;
	}
	else 
	{
		offset = INST.offset;
	}

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);

	/* If num_elems is greater than 1, we need to see how 
	 * the destination register is handled */
	assert(num_elems == 1);

	bytes_to_write = elem_size * num_elems;

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc*4);

	buffer_addr = offset;

	value = si_isa_read_vreg(work_item, INST.vdata);

	mem_write(si_emu->global_mem, buffer_addr, bytes_to_write, &value);

	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("t%d: (%u)<=V%u(%d,%gf) ", work_item->id, buffer_addr,
			INST.vdata, value.as_uint, value.as_float);
	}
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_T_BUFFER_STORE_FORMAT_XYZW_impl(struct si_work_item_t *work_item, struct si_inst_t *inst)
{
	assert(!INST.addr64);
	assert(!INST.index);

	unsigned int offset;
	int elem_size;
	int num_elems;
	int bytes_to_write;
	struct si_buffer_resource_t buf_desc;
	uint32_t buffer_addr;
	union si_reg_t value;

	if (INST.offen)
	{
		offset = si_isa_read_vreg(work_item, INST.vaddr).as_uint;
	}
	else
	{
		offset = INST.offset;
	}

	elem_size = si_isa_get_elem_size(INST.dfmt);
	num_elems = si_isa_get_num_elems(INST.dfmt);

	/* If num_elems is greater than 1, we need to see how
	 * the destination register is handled */
	assert(num_elems == 1);

	bytes_to_write = elem_size * num_elems;

	/* srsrc is in units of 4 registers */
	si_isa_read_buf_res(work_item, &buf_desc, INST.srsrc*4);

	for(int i = 0; i < 4; i++)
	{

		buffer_addr = offset + 4*i;

		value = si_isa_read_vreg(work_item, INST.vdata + i);

		mem_write(si_emu->global_mem, buffer_addr, bytes_to_write, &value);

		if (debug_status(si_isa_debug_category))
		{
			si_isa_debug("t%d: (%u)<=V%u(%d,%gf) ", work_item->id, buffer_addr,
				INST.vdata + i, value.as_uint, value.as_float);
		}
	}
}
#undef INST

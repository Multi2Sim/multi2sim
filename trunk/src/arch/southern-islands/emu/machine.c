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
void si_isa_S_BUFFER_LOAD_DWORD_impl()
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
	si_isa_read_buf_res(&buf_desc, sbase);

	/* sgpr[dst] = read_dword_from_kcache(m_base, m_offset, m_size) */
	m_base = buf_desc.base_addr;
	m_offset = INST.offset * 4;
	//m_size = (buf_desc.stride == 0) ? 1 : buf_desc.num_records;
	
	mem_read(si_emu->global_mem, m_base+m_offset, 4, &value);

	/* Store the data in the destination register */
	si_isa_write_sreg(INST.sdst, value);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_smrd(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("S%u<=(%d,%gf)\n", INST.sdst, value.as_uint, value.as_float);
	}
}
#undef INST

#define INST SI_INST_SMRD
void si_isa_S_LOAD_DWORDX4_impl()
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

	si_isa_read_mem_ptr(&mem_ptr, sbase);

	/* assert(uav_table_ptr.addr < UINT32_MAX) */

	m_base = mem_ptr.addr;
        m_offset = INST.offset * 4;
	m_addr = m_base + m_offset; 

	assert(!(m_addr & 0x3));

	for (i = 0; i < 4; i++) 
	{
		mem_read(si_emu->global_mem, m_base+m_offset+i*4, 4, &value[i]);
		si_isa_write_sreg(INST.sdst+i, value[i]);
	}	

	if (debug_status(si_isa_debug_category))
	{	
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_smrd(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		for (i = 0; i < 4; i++) 
		{
			si_isa_debug("S%u<=(%d,%gf)\n", INST.sdst+i, value[i].as_uint, value[i].as_float);
		}
	}
}
#undef INST

#define INST SI_INST_SMRD
void si_isa_S_BUFFER_LOAD_DWORDX2_impl()
{
	NOT_IMPL();
	union si_reg_t value[2];
        uint32_t m_base;
        uint32_t m_offset;
        uint32_t m_addr;
	struct si_mem_ptr_t mem_ptr;
	int sbase;
	int i;

        assert(INST.imm);

	sbase = INST.sbase << 1;

	si_isa_read_mem_ptr(&mem_ptr, sbase);

	/* assert(uav_table_ptr.addr < UINT32_MAX) */

	m_base = mem_ptr.addr;
        m_offset = INST.offset * 4;
	m_addr = m_base + m_offset; 

	assert(!(m_addr & 0x3));

	for (i = 0; i < 2; i++) 
	{
		mem_read(si_emu->global_mem, m_base+m_offset+i*4, 4, &value[i]);
		si_isa_write_sreg(INST.sdst+i, value[i]);
	}	

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_smrd(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		for (i = 0; i < 2; i++) 
		{
			si_isa_debug("S%u<=(%d,%gf)\n", INST.sdst+i, value[i].as_uint, value[i].as_float);
		}
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_MIN_U32_impl()
{
	/* D.u = (S0.u < S1.u) ? S0.u : S1.u, scc = 1 if S0 is min. */

	unsigned int s0 = si_isa_read_sreg(INST.ssrc0).as_uint;
	unsigned int s1 = si_isa_read_sreg(INST.ssrc1).as_uint;

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

	si_isa_write_sreg(INST.sdst, min);
	si_isa_write_sreg(SI_SCC, s0_min);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sop2(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("S%u<=(%d)\n", INST.sdst, min.as_uint);
		si_isa_debug("scc<=(%d)\n", s0_min.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_AND_B32_impl()
{
	/* D.u = S0.u & S1.u. scc = 1 if result is non-zero. */
	
	unsigned int s0 = si_isa_read_sreg(INST.ssrc0).as_uint;
	unsigned int s1 = si_isa_read_sreg(INST.ssrc1).as_uint;

	union si_reg_t result;
	result.as_uint = s0 & s1;
	union si_reg_t nonzero;
	nonzero.as_uint = result.as_uint != 0;

	si_isa_write_sreg(INST.sdst, result);
	si_isa_write_sreg(SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sop2(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("S%u<=(%d)\n", INST.sdst, result.as_uint);
		si_isa_debug("scc<=(%d)\n", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_AND_B64_impl()
{
	/* D.u = S0.u & S1.u. scc = 1 if result is non-zero. */

	unsigned int s0_lo = si_isa_read_sreg(INST.ssrc0).as_uint;
	unsigned int s0_hi = si_isa_read_sreg(INST.ssrc0 + 1).as_uint;
	unsigned int s1_lo = si_isa_read_sreg(INST.ssrc1).as_uint;
	unsigned int s1_hi = si_isa_read_sreg(INST.ssrc1 + 1).as_uint;

	union si_reg_t result_lo;
	result_lo.as_uint = s0_lo & s1_lo;
	union si_reg_t result_hi;
	result_hi.as_uint = s0_hi & s1_hi;
	union si_reg_t nonzero;
	nonzero.as_uint = result_lo.as_uint && result_hi.as_uint;

	si_isa_write_sreg(INST.sdst, result_lo);
	si_isa_write_sreg(INST.sdst + 1, result_hi);
	si_isa_write_sreg(SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sop2(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("S%u<=(%d)\n", INST.sdst, result_lo.as_uint);
		si_isa_debug("S%u<=(%d)\n", INST.sdst + 1, result_hi.as_uint);
		si_isa_debug("scc<=(%d)\n", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_ANDN2_B64_impl()
{
	/* D.u = S0.u & ~S1.u. scc = 1 if result is non-zero. */

	unsigned int s0_lo = si_isa_read_sreg(INST.ssrc0).as_uint;
	unsigned int s0_hi = si_isa_read_sreg(INST.ssrc0 + 1).as_uint;
	unsigned int s1_lo = si_isa_read_sreg(INST.ssrc1).as_uint;
	unsigned int s1_hi = si_isa_read_sreg(INST.ssrc1 + 1).as_uint;

	union si_reg_t result_lo;
	result_lo.as_uint = s0_lo & ~s1_lo;
	union si_reg_t result_hi;
	result_hi.as_uint = s0_hi & ~s1_hi;
	union si_reg_t nonzero;
	nonzero.as_uint = result_lo.as_uint && result_hi.as_uint;

	si_isa_write_sreg(INST.sdst, result_lo);
	si_isa_write_sreg(INST.sdst + 1, result_hi);
	si_isa_write_sreg(SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sop2(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("S%u<=(%d)\n", INST.sdst, result_lo.as_uint);
		si_isa_debug("S%u<=(%d)\n", INST.sdst + 1, result_hi.as_uint);
		si_isa_debug("scc<=(%d)\n", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_LSHL_B32_impl()
{
	/* D.u = S0.u << S1.u[4:0]. scc = 1 if result is non-zero. */

	unsigned int s0 = si_isa_read_sreg(INST.ssrc0).as_uint;
	unsigned int s1 = si_isa_read_sreg(INST.ssrc1).as_uint & 0x1F;

	union si_reg_t result;
	result.as_uint = s0 << s1;
	union si_reg_t nonzero;
	nonzero.as_uint = result.as_uint != 0;

	si_isa_write_sreg(INST.sdst, result);
	si_isa_write_sreg(SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sop2(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("S%u<=(%d)\n", INST.sdst, result.as_uint);
		si_isa_debug("scc<=(%d)\n", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOP2
void si_isa_S_LSHR_B32_impl()
{
	/* D.u = S0.u >> S1.u[4:0]. scc = 1 if result is non-zero. */

	unsigned int s0 = si_isa_read_sreg(INST.ssrc0).as_uint;
	unsigned int s1 = si_isa_read_sreg(INST.ssrc1).as_uint & 0x1F;

	union si_reg_t result;
	result.as_uint = s0 >> s1;
	union si_reg_t nonzero;
	nonzero.as_uint = result.as_uint != 0;

	si_isa_write_sreg(INST.sdst, result);
	si_isa_write_sreg(SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sop2(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("S%u<=(%d)\n", INST.sdst, result.as_uint);
		si_isa_debug("scc<=(%d)\n", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOPK
void si_isa_S_MOVK_I32_impl()
{
	/* D.i = signext(simm16). */
	short simm16 = INST.simm16;

	union si_reg_t result;
	result.as_int = simm16;

	si_isa_write_sreg(INST.sdst, result);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sopk(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("S%u<=(%d)\n", INST.sdst, result.as_int);
	}
}
#undef INST

#define INST SI_INST_SOP1
void si_isa_S_MOV_B64_impl()
{
	/* D.u = S0.u. */

	union si_reg_t s0_lo;
	s0_lo = si_isa_read_sreg(INST.ssrc0);
	union si_reg_t s0_hi;
	s0_hi = si_isa_read_sreg(INST.ssrc0 + 1);

	si_isa_write_sreg(INST.sdst, s0_lo);
	si_isa_write_sreg(INST.sdst + 1, s0_hi);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sop1(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: S%u<=(%d)\n", si_isa_work_item->id, INST.sdst, s0_lo.as_int);
		si_isa_debug("t%d: S%u<=(%d)\n", si_isa_work_item->id, INST.sdst + 1, s0_hi.as_int);
	}
}
#undef INST

#define INST SI_INST_SOP1
void si_isa_S_AND_SAVEEXEC_B64_impl()
{
	/* D.u = EXEC, EXEC = S0.u & EXEC. scc = 1 if the new value of EXEC is non-zero. */

	union si_reg_t exec_lo = si_isa_read_sreg(SI_EXEC);
	union si_reg_t exec_hi = si_isa_read_sreg(SI_EXEC + 1);
	unsigned int s0_lo = si_isa_read_sreg(INST.ssrc0).as_uint;
	unsigned int s0_hi = si_isa_read_sreg(INST.ssrc0 + 1).as_uint;

	union si_reg_t exec_new_lo;
	exec_new_lo.as_uint = s0_lo & exec_lo.as_uint;
	union si_reg_t exec_new_hi;
	exec_new_hi.as_uint = s0_hi & exec_hi.as_uint;
	union si_reg_t nonzero;
	nonzero.as_uint = exec_new_lo.as_uint && exec_new_hi.as_uint;

	si_isa_write_sreg(INST.sdst, exec_lo);
	si_isa_write_sreg(INST.sdst + 1, exec_hi);
	si_isa_write_sreg(SI_EXEC, exec_new_lo);
	si_isa_write_sreg(SI_EXEC + 1, exec_new_hi);
	si_isa_write_sreg(SI_SCC, nonzero);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sop1(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("S%u<=(%d)\n", INST.sdst, exec_lo.as_uint);
		si_isa_debug("S%u<=(%d)\n", INST.sdst + 1, exec_hi.as_uint);
		si_isa_debug("exec_lo<=(%d)\n", exec_new_lo.as_uint);
		si_isa_debug("exec_hi<=(%d)\n", exec_new_hi.as_uint);
		si_isa_debug("scc<=(%d)\n", nonzero.as_uint);
	}
}
#undef INST

#define INST SI_INST_SOPC
void si_isa_S_CMP_EQ_I32_impl()
{
	NOT_IMPL();
	/* scc = (S0.i == S1.i). */

	int s0 = si_isa_read_sreg(INST.ssrc0).as_int;
	int s1 = si_isa_read_sreg(INST.ssrc1).as_int;

	union si_reg_t equal;
	equal.as_uint = s0 == s1;

	si_isa_write_sreg(SI_SCC, equal);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sopc(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("scc<=(%d)\n", equal.as_uint);
	}
}
#undef INST

void si_isa_S_ENDPGM_impl()
{
	si_isa_wavefront->finished = 1;

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sopp(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
	}
}

#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_SCC1_impl()
{
	NOT_IMPL();
	/* if(SCC == 1) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */

	if(si_isa_read_sreg(SI_SCC).as_uint)
	{
		long pc = si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start;
		short simm16 = INST.simm16;
		int se_simm16 = simm16;

		pc = pc + (se_simm16 * 4) + 4;

		si_isa_wavefront->inst_buf = si_isa_wavefront->inst_buf_start + pc;
	}

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sopp(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
	}
}
#undef INST

#define INST SI_INST_SOPP
void si_isa_S_CBRANCH_EXECZ_impl()
{
	/* if(EXEC == 0) then PC = PC + signext(SIMM16 * 4) + 4; else nop. */

	if(si_isa_read_sreg(SI_EXECZ).as_uint)
	{
		long pc = si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start;
		short simm16 = INST.simm16;
		int se_simm16 = simm16;

		pc = pc + (se_simm16 * 4) + 4;

		si_isa_wavefront->inst_buf = si_isa_wavefront->inst_buf_start + pc;
	}

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sopp(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
	}
}
#undef INST

void si_isa_S_WAITCNT_impl()
{
	/* Nothing to do in emulation */
	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_sopp(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
	}
}

#define INST SI_INST_VOP1
void si_isa_V_MOV_B32_VOP1_impl()
{
	union si_reg_t value;

	value = si_isa_read_sreg(INST.src0);

	si_isa_write_vreg(INST.vdst, value);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vop1(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: V%u<=(%d)\n", si_isa_work_item->id, INST.vdst, value.as_int);
	}
}
#undef INST

#define INST SI_INST_VOP1
void si_isa_V_CVT_F32_I32_impl()
{
	/* D.f = (float)S0.i. */

	union si_reg_t value;
	value.as_float = (float)si_isa_read_reg(INST.src0).as_int;

	si_isa_write_vreg(INST.vdst, value);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vop1(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: V%u<=(%gf)\n", si_isa_work_item->id, INST.vdst, value.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP1
void si_isa_V_CVT_I32_F32_impl()
{
	/* D.i = (int)S0.f. */

	union si_reg_t value;
	value.as_int = (int)si_isa_read_reg(INST.src0).as_float;

	si_isa_write_vreg(INST.vdst, value);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vop1(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: V%u<=(%d)\n", si_isa_work_item->id, INST.vdst, value.as_int);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_MUL_F32_impl()
{
	/* D.f = S0.f * S1.f. */
	
	float s0 = si_isa_read_reg(INST.src0).as_float;
	float s1 = si_isa_read_vreg(INST.vsrc1).as_float;

	union si_reg_t product;
	product.as_float = s0 * s1;

	si_isa_write_vreg(INST.vdst, product);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vop2(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: V%u<=(%gf)\n", si_isa_work_item->id, INST.vdst, product.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_MUL_I32_I24_impl()
{	
	/* D.i = S0.i[23:0] * S1.i[23:0]. */
	
	int s0 = si_isa_read_reg(INST.src0).as_int & 0xFFFFFF;
	int s1 = si_isa_read_vreg(INST.vsrc1).as_int &0xFFFFFF;

	union si_reg_t product;
	product.as_int = s0 * s1;

	si_isa_write_vreg(INST.vdst, product);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vop2(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: V%u<=(%d)\n", si_isa_work_item->id, INST.vdst, product.as_int);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_LSHLREV_B32_impl()
{
	/* D.u = S1.u << S0.u[4:0]. */

	unsigned int s0 = si_isa_read_reg(INST.src0).as_uint & 0x1F;
	unsigned int s1 = si_isa_read_vreg(INST.vsrc1).as_uint;

	union si_reg_t result;
	result.as_uint = s1 << s0;

	si_isa_write_vreg(INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vop2(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: V%u<=(%d)\n", si_isa_work_item->id, INST.vdst, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_OR_B32_impl()
{
	/* D.u = S0.u | S1.u. */

	unsigned int s0 = si_isa_read_reg(INST.src0).as_uint;
	unsigned int s1 = si_isa_read_vreg(INST.vsrc1).as_uint;

	union si_reg_t result;
	result.as_uint = s0 | s1;

	si_isa_write_vreg(INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vop2(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: V%u<=(%d)\n", si_isa_work_item->id, INST.vdst, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_MAC_F32_impl()
{
	/* D.f = S0.f * S1.f + D.f. */

	float s0 = si_isa_read_reg(INST.src0).as_float;
	float s1 = si_isa_read_reg(INST.vsrc1).as_float;
	float d = si_isa_read_vreg(INST.vdst).as_float;

	union si_reg_t result;
	result.as_float = s0 * s1 + d;

	si_isa_write_vreg(INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vop2(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: V%u<=(%gf)\n", si_isa_work_item->id, INST.vdst, result.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_ADD_I32_impl()
{
	/* D.u = S0.u + S1.u, vcc = carry-out. */

	unsigned int s0 = si_isa_read_reg(INST.src0).as_uint;
	unsigned int s1 = si_isa_read_vreg(INST.vsrc1).as_uint;

	union si_reg_t sum;
	sum.as_uint = s0 + s1;
	union si_reg_t carry;
	carry.as_uint = (((long)s0 + (long)s1) > 0xFFFFFFFF);

	si_isa_write_vreg(INST.vdst, sum);
	si_isa_bitmask_sreg(SI_VCC, carry);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vop2(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: V%u<=(%d)\n", si_isa_work_item->id, INST.vdst, sum.as_uint);
		si_isa_debug("wf_id%d: vcc<=(%d)\n", si_isa_work_item->id_in_wavefront, carry.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP3a
void si_isa_V_MAD_F32_impl()
{
	/* D.f = S0.f * S1.f + S2.f. */

	float s0 = si_isa_read_reg(INST.src0).as_float;
	float s1 = si_isa_read_reg(INST.src1).as_float;
	float s2 = si_isa_read_reg(INST.src2).as_float;

	union si_reg_t result;
	result.as_float = s0 * s1 + s2;

	si_isa_write_vreg(INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vop3(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: V%u<=(%gf)\n", si_isa_work_item->id, INST.vdst, result.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP3a
void si_isa_V_MUL_LO_I32_impl()
{
	/* D.i = S0.i * S1.i. */

	int s0 = si_isa_read_reg(INST.src0).as_int;
	int s1 = si_isa_read_reg(INST.src1).as_int;

	union si_reg_t result;
	result.as_int = s0 * s1;

	si_isa_write_vreg(INST.vdst, result);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vop3(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: V%u<=(%d)\n", si_isa_work_item->id, INST.vdst, result.as_int);
	}
}
#undef INST

#define INST SI_INST_VOPC
void si_isa_V_CMP_LT_I32_impl()
{
	/* vcc = (S0.i < S1.i). */

	int s0 = si_isa_read_reg(INST.src0).as_int;
	int s1 = si_isa_read_vreg(INST.src0).as_int;

	union si_reg_t result;
	result.as_uint = (s0 < s1);

	si_isa_bitmask_sreg(SI_VCC, result);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vopc(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("wf_id%d: vcc<=(%d)\n", si_isa_work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOPC
void si_isa_V_CMP_GT_I32_impl()
{
	/* vcc = (S0.i > S1.i). */

	int s0 = si_isa_read_reg(INST.src0).as_int;
	int s1 = si_isa_read_vreg(INST.vsrc1).as_int;

	union si_reg_t result;
	result.as_uint = (s0 > s1);

	si_isa_bitmask_sreg(SI_VCC, result);
	
	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vopc(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("wf_id%d: vcc<=(%d)\n", si_isa_work_item->id_in_wavefront, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_VOP3b
void si_isa_V_CMP_GT_I32_VOP3b_impl()
{
	/* D.u = (S0 > S1). */

	int s0 = si_isa_read_reg(INST.src0).as_int;
	int s1 = si_isa_read_reg(INST.src1).as_int;

	union si_reg_t result;
	result.as_uint = (s0 > s1);

	si_isa_bitmask_sreg(INST.sdst, result);
	
	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_vop3(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("wf_id%d: S[%d:+1]<=(%d)\n", si_isa_work_item->id_in_wavefront, INST.sdst, result.as_uint);
	}
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_T_BUFFER_LOAD_FORMAT_X_impl()
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
		offset = si_isa_read_vreg(INST.vaddr).as_uint;
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
	si_isa_read_buf_res(&buf_desc, INST.srsrc*4);

	buffer_addr = offset;

	mem_read(si_emu->global_mem, buffer_addr, bytes_to_read, &value);

	si_isa_write_vreg(INST.vdata, value);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_mtbuf(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: V%u<=(%u)(%d,%gf)\n", si_isa_work_item->id, INST.vdata, 
			buffer_addr, value.as_uint, value.as_float);
	}
}
#undef INST

#define INST SI_INST_MTBUF
void si_isa_T_BUFFER_STORE_FORMAT_X_impl()
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
		offset = si_isa_read_vreg(INST.vaddr).as_uint;
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
	si_isa_read_buf_res(&buf_desc, INST.srsrc*4);

	buffer_addr = offset;

	value = si_isa_read_vreg(INST.vdata);

	mem_write(si_emu->global_mem, buffer_addr, bytes_to_write, &value);

	if (debug_status(si_isa_debug_category))
	{
		int line_size = MAX_INST_STR_SIZE;
		char inst_dump[line_size];
		si_inst_dump_mtbuf(si_isa_inst, si_isa_wavefront->inst_buf - si_isa_wavefront->inst_buf_start, si_isa_wavefront->inst_buf, inst_dump, line_size);
		si_isa_debug("%s", inst_dump);
		si_isa_debug("t%d: (%u)<=V%u(%d,%gf)\n", si_isa_work_item->id, buffer_addr, 
			INST.vdata, value.as_uint, value.as_float);
	}
}
#undef INST

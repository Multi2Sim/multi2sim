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
	uint32_t value;
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
	si_isa_write_sgpr(INST.sdst, value);

	if (debug_status(si_isa_debug_category))
	{
		union si_reg_t reg;
		reg.as_uint = value;

		si_isa_debug("S%u<=(%d,%gf)", INST.sdst, reg.as_uint, reg.as_float);
	}
}
#undef INST

#define INST SI_INST_SMRD
void si_isa_S_LOAD_DWORDX4_impl()
{
	uint32_t value[4];
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
		si_isa_write_sgpr(INST.sdst+i, value[i]);
	}	

	if (debug_status(si_isa_debug_category))
	{
		for (i = 0; i < 4; i++) 
		{
			union si_reg_t reg;
			reg.as_uint = value[i];

			si_isa_debug("S%u<=(%d,%gf) ", INST.sdst+i, reg.as_uint, reg.as_float);
		}
	}
}
#undef INST

void si_isa_S_BUFFER_LOAD_DWORDX2_impl()
{
	NOT_IMPL();
}

#define INST SI_INST_SOP2
void si_isa_S_MIN_U32_impl()
{
	/* D.u = (S0.u < S1.u) ? S0.u : S1.u, scc = 1 if S0 is min. */

	unsigned int s0 = si_isa_read_sgpr(INST.ssrc0);
	unsigned int s1 = si_isa_read_sgpr(INST.ssrc1);

	unsigned int dst;
	unsigned int scc;

	if (INST.ssrc0 < INST.ssrc1)
	{
		dst = s0;
		scc = 1;
	}
	else
	{
		dst = s1;
		scc = 0;
	}

	si_isa_write_sgpr(INST.sdst, dst);
	si_wavefront_bitmask_cc(&si_isa_wavefront->scc, si_isa_work_item->id_in_wavefront, scc);

	if (debug_status(si_isa_debug_category))
	{
		union si_reg_t reg;

		reg.as_uint = dst;
		si_isa_debug("S%u<=(%d,%gf) ", INST.sdst, reg.as_uint, reg.as_float);
		
		reg.as_uint = scc;
		si_isa_debug("scc<=(%d,%gf) ", reg.as_uint, reg.as_float);
	}
}
#undef INST

void si_isa_S_AND_B32_impl()
{
	NOT_IMPL();
}

void si_isa_S_AND_B64_impl()
{
	NOT_IMPL();
}

void si_isa_S_ANDN2N2_B64_impl()
{
	NOT_IMPL();
}

void si_isa_S_LSHL_B32_impl()
{
	NOT_IMPL();
}

void si_isa_S_LSHR_B32_impl()
{
	NOT_IMPL();
}

void si_isa_S_MOVK_I32_impl()
{
	NOT_IMPL();
}

void si_isa_S_MOV_B64_impl()
{
	NOT_IMPL();
}

void si_isa_S_AND_SAVEEXEC_B64_impl()
{
	NOT_IMPL();
}

void si_isa_S_CMP_EQ_I32_impl()
{
	NOT_IMPL();
}

void si_isa_S_ENDPGM_impl()
{
	si_isa_wavefront->finished = 1;
}

void si_isa_S_CBRANCH_SCC1_impl()
{
	NOT_IMPL();
}

void si_isa_S_CBRANCH_EXECZ_impl()
{
	NOT_IMPL();
}

void si_isa_S_WAITCNT_impl()
{
	/* Nothing to do in emulation */
	if (debug_status(si_isa_debug_category))
	{
		si_isa_debug("wait");
	}
}

#define INST SI_INST_VOP1
void si_isa_V_MOV_B32_VOP1_impl()
{
	uint32_t value;

	value = si_isa_read_sgpr(INST.src0);

	si_isa_write_vgpr(INST.vdst, value);

	if (debug_status(si_isa_debug_category))
	{
		union si_reg_t reg;
		reg.as_uint = value;

		si_isa_debug("t%d: V%u<=(%d,%gf) ", si_isa_work_item->id, INST.vdst, reg.as_uint,
			reg.as_float);
	}
}
#undef INST

void si_isa_V_CVT_F32_I32_impl()
{
	NOT_IMPL();
}

void si_isa_V_CVT_I32_F32_impl()
{
	NOT_IMPL();
}

void si_isa_V_MUL_F32_impl()
{
	NOT_IMPL();
}

#define INST SI_INST_VOP2
void si_isa_V_MUL_I32_I24_impl()
{	
	/* D.i = S0.i[23:0] * S1.i[23:0]. */
	
	int s0 = si_isa_read_reg(INST.src0) & 0xFFFFFF;
	int s1 = si_isa_read_vgpr(INST.vsrc1) &0xFFFFFF;

	int dst = s0 * s1;

	si_isa_write_vgpr(INST.vdst, dst);

	if (debug_status(si_isa_debug_category))
	{
		union si_reg_t reg;

		reg.as_int = dst;
		si_isa_debug("t%d: V%u<=(%d,%gf) ", si_isa_work_item->id, INST.vdst, reg.as_uint, 
			reg.as_float);
	}
}
#undef INST

#define INST SI_INST_VOP2
void si_isa_V_LSHLREV_B32_impl()
{
	/* D.u = S1.u << S0.u[4:0]. */

	int s0 = si_isa_read_reg(INST.src0) & 0x1F;
	unsigned int s1 = si_isa_read_vgpr(INST.vsrc1);

	unsigned int dst = s1 << s0;

	si_isa_write_vgpr(INST.vdst, dst);

	if (debug_status(si_isa_debug_category))
	{
		union si_reg_t reg;

		reg.as_uint = dst;
		si_isa_debug("t%d: V%u<=(%d,%gf) ", si_isa_work_item->id, INST.vdst, reg.as_uint, 
			reg.as_float);
	}
}
#undef INST

void si_isa_V_OR_B32_impl()
{
	NOT_IMPL();
}

void si_isa_V_MAC_F32_impl()
{
	NOT_IMPL();
}

#define INST SI_INST_VOP2
void si_isa_V_ADD_I32_impl()
{
	/* D.u = S0.u + S1.u, vcc = carry-out. */

	unsigned int s0 = si_isa_read_reg(INST.src0);
	unsigned int s1 = si_isa_read_vgpr(INST.vsrc1);

	unsigned int dst = s0 + s1;
	unsigned int vcc = (((long)s0 + (long)s1) > 0xFFFFFFFF);

	si_isa_write_vgpr(INST.vdst, dst);
	si_wavefront_bitmask_cc(&si_isa_wavefront->vcc, si_isa_work_item->id_in_wavefront, vcc);

	if (debug_status(si_isa_debug_category))
	{
		union si_reg_t reg;

		reg.as_uint = dst;
		si_isa_debug("t%d: V%u<=(%d,%gf) ", si_isa_work_item->id, INST.vdst, reg.as_uint, 
			reg.as_float);
		
		reg.as_uint = vcc;
		si_isa_debug("vcc<=(%d,%gf) ", reg.as_uint, reg.as_float);
	}
}
#undef INST

void si_isa_V_MAD_F32_impl()
{
	NOT_IMPL();
}

void si_isa_V_MUL_LO_I32_impl()
{
	NOT_IMPL();
}

void si_isa_V_CMP_LT_I32_impl()
{
	NOT_IMPL();
}

void si_isa_V_CMP_GT_I32_impl()
{
	NOT_IMPL();
}

#define INST SI_INST_VOP3b
void si_isa_V_CMP_GT_I32_VOP3b_impl()
{
	NOT_IMPL();
	/* D.u = (S0 > S1). */

	/* int s0 = si_isa_read_reg(INST.src0);
	int s1 = si_isa_read_reg(INST.src1);

	unsigned int value = (s0 > s1); */

	/* Bitmask  [sdst:sdst + 1] */
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
	uint32_t value;

	if (INST.offen)
	{
		offset = si_isa_read_vgpr(INST.vaddr);
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

	si_isa_write_vgpr(INST.vdata, value);

	if (debug_status(si_isa_debug_category))
	{
		union si_reg_t reg;
		reg.as_int = value;
		
		si_isa_debug("t%d: V%u<=(%u)(%d,%gf) ", si_isa_work_item->id, INST.vdata, 
			buffer_addr, reg.as_uint, reg.as_float);
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
	uint32_t value;

	if (INST.offen)
	{
		offset = si_isa_read_vgpr(INST.vaddr);
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

	value = si_isa_read_vgpr(INST.vdata);

	mem_write(si_emu->global_mem, buffer_addr, bytes_to_write, &value);

	if (debug_status(si_isa_debug_category))
	{
		union si_reg_t reg;
		reg.as_int = value;
		
		si_isa_debug("t%d: (%u)<=V%u(%d,%gf) ", si_isa_work_item->id, buffer_addr, 
			INST.vdata, reg.as_uint, reg.as_float);
	}
}
#undef INST

void si_isa_S_ANDN2_B64_impl()
{
	NOT_IMPL();
}


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

	sbase = INST.sbase << 1;

	/* sbase holds the first of 4 registers containing the buffer resource descriptor */
	si_isa_init_buf_res(&buf_desc, sbase);

	/* sgpr[dst] = read_dword_from_kcache(m_base, m_offset, m_size) */
	m_base = buf_desc.base_addr;
	m_offset = INST.offset * 4;
	//m_size = (buf_desc.stride == 0) ? 1 : buf_desc.num_records;
	
	/* Addressing CB0 */
	mem_read(si_emu->global_mem, m_base+m_offset, 4, &value);

	/* Store the data in the destination register */
	si_isa_write_sgpr(INST.sdst, value);

	if (debug_status(si_isa_debug_category))
	{
		union si_reg_t reg;
		reg.as_uint = value;

		si_isa_debug("S%u <=(%d,%gf)\n", INST.sdst, reg.as_uint, reg.as_float);
	}
}
#undef INST

void si_isa_S_LOAD_DWORDX4_impl()
{
	NOT_IMPL();
}

void si_isa_S_MIN_U32_impl()
{
	NOT_IMPL();
}

void si_isa_S_ENDPGM_impl()
{
	NOT_IMPL();
}

void si_isa_S_WAITCNT_impl()
{
	NOT_IMPL();
}

void si_isa_S_MOV_B32_impl()
{
	NOT_IMPL();
}

void si_isa_V_MUL_I32_I24_impl()
{
	NOT_IMPL();
}

void si_isa_V_LSHLREV_B32_impl()
{
	NOT_IMPL();
}

void si_isa_V_ADD_I32_impl()
{
	NOT_IMPL();
}

void si_isa_T_BUFFER_LOAD_FORMAT_X_impl()
{
	NOT_IMPL();
}

void si_isa_T_BUFFER_STORE_FORMAT_X_impl()
{
	NOT_IMPL();
}


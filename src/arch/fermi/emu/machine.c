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

#include <fermi-emu.h>
#include <mem-system.h>
#include <x86-emu.h>



char *frm_err_isa_note =
	"\tThe NVIDIA Fermi SASS instruction set is partially supported by Multi2Sim. If\n"
	"\tyour program is using an unimplemented instruction, please email\n"
	"\t'development@multi2sim.org' to request support for it.\n";

#define NOT_IMPL() fatal("GPU instruction '%s' not implemented\n%s", \
	frm_isa_inst->info->name, frm_err_isa_note)



void frm_isa_FFMA_impl()
{
	NOT_IMPL();
}

void frm_isa_FADD_impl()
{
	NOT_IMPL();
}

void frm_isa_FADD32I_impl()
{
	NOT_IMPL();
}

void frm_isa_FCMP_impl()
{
	NOT_IMPL();
}

void frm_isa_FMUL_impl()
{
	NOT_IMPL();
}

void frm_isa_FMUL32I_impl()
{
	NOT_IMPL();
}

void frm_isa_FMNMX_impl()
{
	NOT_IMPL();
}

void frm_isa_FSWZ_impl()
{
	NOT_IMPL();
}

void frm_isa_FSET_impl()
{
	NOT_IMPL();
}

void frm_isa_FSETP_impl()
{
	NOT_IMPL();
}

void frm_isa_RRO_impl()
{
	NOT_IMPL();
}

void frm_isa_MUFU_impl()
{
	NOT_IMPL();
}

void frm_isa_DFMA_impl()
{
	NOT_IMPL();
}

void frm_isa_DADD_impl()
{
	NOT_IMPL();
}

void frm_isa_DMUL_impl()
{
	NOT_IMPL();
}

void frm_isa_DMNMX_impl()
{
	NOT_IMPL();
}

void frm_isa_DSET_impl()
{
	NOT_IMPL();
}

void frm_isa_DSETP_impl()
{
	NOT_IMPL();
}

void frm_isa_IMAD_impl()
{
	NOT_IMPL();
}

void frm_isa_IMUL_impl()
{
	NOT_IMPL();
}

void frm_isa_IADD_impl()
{
	NOT_IMPL();
}

void frm_isa_IADD32I_impl()
{
	NOT_IMPL();
}

void frm_isa_ISCADD_impl()
{
	NOT_IMPL();
}

void frm_isa_ISAD_impl()
{
	NOT_IMPL();
}

void frm_isa_IMNMX_impl()
{
	NOT_IMPL();
}

void frm_isa_BFE_impl()
{
	NOT_IMPL();
}

void frm_isa_BFI_impl()
{
	NOT_IMPL();
}

void frm_isa_SHR_impl()
{
	NOT_IMPL();
}

void frm_isa_SHL_impl()
{
	NOT_IMPL();
}

void frm_isa_LOP_impl()
{
	NOT_IMPL();
}

void frm_isa_LOP32I_impl()
{
	NOT_IMPL();
}

void frm_isa_FLO_impl()
{
	NOT_IMPL();
}

void frm_isa_ISET_impl()
{
	NOT_IMPL();
}

void frm_isa_ISETP_impl()
{
	NOT_IMPL();
}

void frm_isa_ICMP_impl()
{
	NOT_IMPL();
}

void frm_isa_POPC_impl()
{
	NOT_IMPL();
}

void frm_isa_F2F_impl()
{
	NOT_IMPL();
}

void frm_isa_F2I_impl()
{
	NOT_IMPL();
}

void frm_isa_I2F_impl()
{
	NOT_IMPL();
}

void frm_isa_I2I_impl()
{
	NOT_IMPL();
}

void frm_isa_MOV_impl()
{
	NOT_IMPL();
}

void frm_isa_MOV32I_impl()
{
	NOT_IMPL();
}

void frm_isa_SEL_impl()
{
	NOT_IMPL();
}

void frm_isa_PRMT_impl()
{
	NOT_IMPL();
}

void frm_isa_P2R_impl()
{
	NOT_IMPL();
}

void frm_isa_R2P_impl()
{
	NOT_IMPL();
}

void frm_isa_CSET_impl()
{
	NOT_IMPL();
}

void frm_isa_CSETP_impl()
{
	NOT_IMPL();
}

void frm_isa_PSET_impl()
{
	NOT_IMPL();
}

void frm_isa_PSETP_impl()
{
	NOT_IMPL();
}

void frm_isa_TEX_impl()
{
	NOT_IMPL();
}

void frm_isa_TLD_impl()
{
	NOT_IMPL();
}

void frm_isa_TLD4_impl()
{
	NOT_IMPL();
}

void frm_isa_TXQ_impl()
{
	NOT_IMPL();
}

void frm_isa_LDC_impl()
{
	NOT_IMPL();
}

void frm_isa_LD_impl()
{
	NOT_IMPL();
}

void frm_isa_LDU_impl()
{
	NOT_IMPL();
}

void frm_isa_LDL_impl()
{
	NOT_IMPL();
}

void frm_isa_LDS_impl()
{
	NOT_IMPL();
}

void frm_isa_LDLK_impl()
{
	NOT_IMPL();
}

void frm_isa_LDSLK_impl()
{
	NOT_IMPL();
}

void frm_isa_LD_LDU_impl()
{
	NOT_IMPL();
}

void frm_isa_LDS_LDU_impl()
{
	NOT_IMPL();
}

void frm_isa_ST_impl()
{
	NOT_IMPL();
}

void frm_isa_STL_impl()
{
	NOT_IMPL();
}

void frm_isa_STUL_impl()
{
	NOT_IMPL();
}

void frm_isa_STS_impl()
{
	NOT_IMPL();
}

void frm_isa_STSUL_impl()
{
	NOT_IMPL();
}

void frm_isa_ATOM_impl()
{
	NOT_IMPL();
}

void frm_isa_RED_impl()
{
	NOT_IMPL();
}

void frm_isa_CCTL_impl()
{
	NOT_IMPL();
}

void frm_isa_CCTLL_impl()
{
	NOT_IMPL();
}

void frm_isa_MEMBAR_impl()
{
	NOT_IMPL();
}

void frm_isa_SULD_impl()
{
	NOT_IMPL();
}

void frm_isa_SULEA_impl()
{
	NOT_IMPL();
}

void frm_isa_SUST_impl()
{
	NOT_IMPL();
}

void frm_isa_SURED_impl()
{
	NOT_IMPL();
}

void frm_isa_SUQ_impl()
{
	NOT_IMPL();
}

void frm_isa_BRA_impl()
{
	NOT_IMPL();
}

void frm_isa_BRX_impl()
{
	NOT_IMPL();
}

void frm_isa_JMP_impl()
{
	NOT_IMPL();
}

void frm_isa_JMX_impl()
{
	NOT_IMPL();
}

void frm_isa_CAL_impl()
{
	NOT_IMPL();
}

void frm_isa_JCAL_impl()
{
	NOT_IMPL();
}

void frm_isa_RET_impl()
{
	NOT_IMPL();
}

void frm_isa_BRK_impl()
{
	NOT_IMPL();
}

void frm_isa_CONT_impl()
{
	NOT_IMPL();
}

void frm_isa_LONGJMP_impl()
{
	NOT_IMPL();
}

void frm_isa_SSY_impl()
{
	NOT_IMPL();
}

void frm_isa_PBK_impl()
{
	NOT_IMPL();
}

void frm_isa_PCNT_impl()
{
	NOT_IMPL();
}

void frm_isa_PRET_impl()
{
	NOT_IMPL();
}

void frm_isa_PLONGJMP_impl()
{
	NOT_IMPL();
}

void frm_isa_BPT_impl()
{
	NOT_IMPL();
}

void frm_isa_EXIT_impl()
{
	NOT_IMPL();
}

void frm_isa_NOP_impl()
{
	NOT_IMPL();
}

void frm_isa_S2R_impl()
{
	NOT_IMPL();
}

void frm_isa_B2R_impl()
{
	NOT_IMPL();
}

void frm_isa_LEPC_impl()
{
	NOT_IMPL();
}

void frm_isa_BAR_impl()
{
	NOT_IMPL();
}

void frm_isa_VOTE_impl()
{
	NOT_IMPL();
}


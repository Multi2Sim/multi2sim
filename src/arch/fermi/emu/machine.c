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

#include <fermi-emu.h>
#include <mem-system.h>
#include <x86-emu.h>


char *err_frm_sass_note =
	"\tThe NVIDIA Fermi instruction set is partially supported by Multi2Sim. If\n"
	"\tyour program is using an unimplemented instruction, please email\n"
	"\t'development@multi2sim.org' to request support for it.\n";

#define NOT_IMPL() fatal("Fermi instruction '%s' not implemented\n%s", \
	frm_sass_inst->info->name, err_frm_sass_note)



/*
 * Floating Point Instructions
 */

void frm_sass_FFMA_impl()
{
	NOT_IMPL();
}

void frm_sass_FADD_impl()
{
	NOT_IMPL();
}

void frm_sass_FADD32I_impl()
{
	NOT_IMPL();
}

void frm_sass_FCMP_impl()
{
	NOT_IMPL();
}

void frm_sass_FMUL_impl()
{
	NOT_IMPL();
}

void frm_sass_FMUL32I_impl()
{
	NOT_IMPL();
}

void frm_sass_FMNMX_impl()
{
	NOT_IMPL();
}

void frm_sass_FSWZ_impl()
{
	NOT_IMPL();
}

void frm_sass_FSET_impl()
{
	NOT_IMPL();
}

void frm_sass_FSETP_impl()
{
	NOT_IMPL();
}

void frm_sass_RRO_impl()
{
	NOT_IMPL();
}

void frm_sass_MUFU_impl()
{
	NOT_IMPL();
}

void frm_sass_DFMA_impl()
{
	NOT_IMPL();
}

void frm_sass_DADD_impl()
{
	NOT_IMPL();
}

void frm_sass_DMUL_impl()
{
	NOT_IMPL();
}

void frm_sass_DMNMX_impl()
{
	NOT_IMPL();
}

void frm_sass_DSET_impl()
{
	NOT_IMPL();
}

void frm_sass_DSETP_impl()
{
	NOT_IMPL();
}



/*
 * Integer Instructions
 */

void frm_sass_IMAD_impl()
{
	NOT_IMPL();
}

void frm_sass_IMUL_impl()
{
	NOT_IMPL();
}

void frm_sass_IADD_impl()
{
	NOT_IMPL();
}

void frm_sass_IADD32I_impl()
{
	NOT_IMPL();
}

void frm_sass_ISCADD_impl()
{
	NOT_IMPL();
}

void frm_sass_ISAD_impl()
{
	NOT_IMPL();
}

void frm_sass_IMNMX_impl()
{
	NOT_IMPL();
}

void frm_sass_BFE_impl()
{
	NOT_IMPL();
}

void frm_sass_BFI_impl()
{
	NOT_IMPL();
}

void frm_sass_SHR_impl()
{
	NOT_IMPL();
}

void frm_sass_SHL_impl()
{
	NOT_IMPL();
}

void frm_sass_LOP_impl()
{
	NOT_IMPL();
}

void frm_sass_LOP32I_impl()
{
	NOT_IMPL();
}

void frm_sass_FLO_impl()
{
	NOT_IMPL();
}

void frm_sass_ISET_impl()
{
	NOT_IMPL();
}

void frm_sass_ISETP_impl()
{
	NOT_IMPL();
}

void frm_sass_ICMP_impl()
{
	NOT_IMPL();
}

void frm_sass_POPC_impl()
{
	NOT_IMPL();
}



/*
 * Conversion Instructions
 */

void frm_sass_F2F_impl()
{
	NOT_IMPL();
}

void frm_sass_F2I_impl()
{
	NOT_IMPL();
}

void frm_sass_I2F_impl()
{
	NOT_IMPL();
}

void frm_sass_I2I_impl()
{
	NOT_IMPL();
}



/*
 * Movement Instructions
 */

void frm_sass_MOV_impl()
{
	NOT_IMPL();
}

void frm_sass_MOV32I_impl()
{
	NOT_IMPL();
}

void frm_sass_SEL_impl()
{
	NOT_IMPL();
}

void frm_sass_PRMT_impl()
{
	NOT_IMPL();
}



/*
 * Predicate/CC Instructions 
 */

void frm_sass_P2R_impl()
{
	NOT_IMPL();
}

void frm_sass_R2P_impl()
{
	NOT_IMPL();
}

void frm_sass_CSET_impl()
{
	NOT_IMPL();
}

void frm_sass_CSETP_impl()
{
	NOT_IMPL();
}

void frm_sass_PSET_impl()
{
	NOT_IMPL();
}

void frm_sass_PSETP_impl()
{
	NOT_IMPL();
}



/*
 * Texture Instructions
 */

void frm_sass_TEX_impl()
{
	NOT_IMPL();
}

void frm_sass_TLD_impl()
{
	NOT_IMPL();
}

void frm_sass_TLD4_impl()
{
	NOT_IMPL();
}

void frm_sass_TXQ_impl()
{
	NOT_IMPL();
}



/*
 * Compute Load/Store Instructions
 */

void frm_sass_LDC_impl()
{
	NOT_IMPL();
}

void frm_sass_LD_impl()
{
	NOT_IMPL();
}

void frm_sass_LDU_impl()
{
	NOT_IMPL();
}

void frm_sass_LDL_impl()
{
	NOT_IMPL();
}

void frm_sass_LDS_impl()
{
	NOT_IMPL();
}

void frm_sass_LDLK_impl()
{
	NOT_IMPL();
}

void frm_sass_LDSLK_impl()
{
	NOT_IMPL();
}

void frm_sass_LD_LDU_impl()
{
	NOT_IMPL();
}

void frm_sass_LDS_LDU_impl()
{
	NOT_IMPL();
}

void frm_sass_ST_impl()
{
	NOT_IMPL();
}

void frm_sass_STL_impl()
{
	NOT_IMPL();
}

void frm_sass_STUL_impl()
{
	NOT_IMPL();
}

void frm_sass_STS_impl()
{
	NOT_IMPL();
}

void frm_sass_STSUL_impl()
{
	NOT_IMPL();
}

void frm_sass_ATOM_impl()
{
	NOT_IMPL();
}

void frm_sass_RED_impl()
{
	NOT_IMPL();
}

void frm_sass_CCTL_impl()
{
	NOT_IMPL();
}

void frm_sass_CCTLL_impl()
{
	NOT_IMPL();
}

void frm_sass_MEMBAR_impl()
{
	NOT_IMPL();
}



/*
 * Surface Memory Instructions
 */

void frm_sass_SULD_impl()
{
	NOT_IMPL();
}

void frm_sass_SULEA_impl()
{
	NOT_IMPL();
}

void frm_sass_SUST_impl()
{
	NOT_IMPL();
}

void frm_sass_SURED_impl()
{
	NOT_IMPL();
}

void frm_sass_SUQ_impl()
{
	NOT_IMPL();
}



/*
 * Control Instructions
 */

void frm_sass_BRA_impl()
{
	NOT_IMPL();
}

void frm_sass_BRX_impl()
{
	NOT_IMPL();
}

void frm_sass_JMP_impl()
{
	NOT_IMPL();
}

void frm_sass_JMX_impl()
{
	NOT_IMPL();
}

void frm_sass_CAL_impl()
{
	NOT_IMPL();
}

void frm_sass_JCAL_impl()
{
	NOT_IMPL();
}

void frm_sass_RET_impl()
{
	NOT_IMPL();
}

void frm_sass_BRK_impl()
{
	NOT_IMPL();
}

void frm_sass_CONT_impl()
{
	NOT_IMPL();
}

void frm_sass_LONGJMP_impl()
{
	NOT_IMPL();
}

void frm_sass_SSY_impl()
{
	NOT_IMPL();
}

void frm_sass_PBK_impl()
{
	NOT_IMPL();
}

void frm_sass_PCNT_impl()
{
	NOT_IMPL();
}

void frm_sass_PRET_impl()
{
	NOT_IMPL();
}

void frm_sass_PLONGJMP_impl()
{
	NOT_IMPL();
}

void frm_sass_BPT_impl()
{
	NOT_IMPL();
}

void frm_sass_EXIT_impl()
{
	NOT_IMPL();
}



/*
 * Miscellaneous Instructions
 */

void frm_sass_NOP_impl()
{
	NOT_IMPL();
}

void frm_sass_S2R_impl()
{
	NOT_IMPL();
}

void frm_sass_B2R_impl()
{
	NOT_IMPL();
}

void frm_sass_LEPC_impl()
{
	NOT_IMPL();
}

void frm_sass_BAR_impl()
{
	NOT_IMPL();
}

void frm_sass_VOTE_impl()
{
	NOT_IMPL();
}




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


#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>
#include "asm-thumb.h"


/* Hard-coded instructions */

void arm_thumb32_disasm_init()
{
	int i;

	/* Allocate Memory */
	arm_thumb32_asm_table			= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv1_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv2_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv3_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv4_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv5_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv6_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv7_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv8_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv9_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv10_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv11_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv12_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv13_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv14_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_lv15_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_asm_ldst_mul_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_ldst_mul1_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_ldst_mul2_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_ldst_mul3_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_ldst_mul4_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_ldst_mul5_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_ldst_mul6_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_asm_ldst_dual_table		= xcalloc(32, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_ldst1_dual_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_ldst2_dual_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_asm_ldst3_dual_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_dproc_shft_reg_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_shft_reg1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_shft_reg2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_shft_reg3_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_shft_reg4_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_shft_reg5_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_shft_reg6_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_dproc_imm_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_imm1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_imm2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_imm3_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_imm4_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_imm5_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_imm6_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_dproc_reg_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_reg1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_reg2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_reg3_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_reg4_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_reg5_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_reg6_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_reg7_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_dproc_misc_table	= xcalloc(8, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_misc1_table	= xcalloc(8, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_dproc_bin_imm_table		= xcalloc(32, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_bin_imm1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_bin_imm2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_dproc_bin_imm3_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_brnch_ctrl_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_brnch_ctrl1_table 	= xcalloc(8, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_st_single_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_st_single1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_st_single2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_st_single3_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_st_single4_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_st_single5_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_st_single6_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_ld_byte_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_ld_byte1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_ld_byte2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_ld_byte3_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_ld_byte4_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_ld_byte5_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_ld_byte6_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_ld_hfword_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_ld_hfword1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_ld_hfword2_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_ld_word_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_ld_word1_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_mult_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_mult1_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_mult_long_table	= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));

	arm_thumb32_mov_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));
	arm_thumb32_mov1_table		= xcalloc(16, sizeof(struct arm_thumb32_inst_info_t));


	/* Load store Multiple tables */
	arm_thumb32_asm_table[1].next_table 		=  arm_thumb32_asm_lv1_table;
	arm_thumb32_asm_table[1].next_table_high 	= 26;
	arm_thumb32_asm_table[1].next_table_low		= 26;

	arm_thumb32_asm_lv1_table[0].next_table		= arm_thumb32_asm_lv2_table;
	arm_thumb32_asm_lv1_table[0].next_table_high	= 25;
	arm_thumb32_asm_lv1_table[0].next_table_low	= 25;

	arm_thumb32_asm_lv2_table[0].next_table		= arm_thumb32_asm_lv3_table;
	arm_thumb32_asm_lv2_table[0].next_table_high	= 22;
	arm_thumb32_asm_lv2_table[0].next_table_low	= 22;

	arm_thumb32_asm_lv3_table[0].next_table		= arm_thumb32_asm_ldst_mul_table;
	arm_thumb32_asm_lv3_table[0].next_table_high	= 24;
	arm_thumb32_asm_lv3_table[0].next_table_low	= 23;

	arm_thumb32_asm_ldst_mul_table[1].next_table 		= arm_thumb32_asm_ldst_mul1_table;
	arm_thumb32_asm_ldst_mul_table[1].next_table_high 	= 20;
	arm_thumb32_asm_ldst_mul_table[1].next_table_low 	= 20;

	arm_thumb32_asm_ldst_mul1_table[1].next_table 		= arm_thumb32_asm_ldst_mul2_table;
	arm_thumb32_asm_ldst_mul1_table[1].next_table_high 	= 21;
	arm_thumb32_asm_ldst_mul1_table[1].next_table_low 	= 21;

	arm_thumb32_asm_ldst_mul2_table[1].next_table 		= arm_thumb32_asm_ldst_mul3_table;
	arm_thumb32_asm_ldst_mul2_table[1].next_table_high 	= 19;
	arm_thumb32_asm_ldst_mul2_table[1].next_table_low 	= 16;

	arm_thumb32_asm_ldst_mul_table[2].next_table 		= arm_thumb32_asm_ldst_mul4_table;
	arm_thumb32_asm_ldst_mul_table[2].next_table_high 	= 20;
	arm_thumb32_asm_ldst_mul_table[2].next_table_low 	= 20;

	arm_thumb32_asm_ldst_mul4_table[0].next_table 		= arm_thumb32_asm_ldst_mul5_table;
	arm_thumb32_asm_ldst_mul4_table[0].next_table_high 	= 21;
	arm_thumb32_asm_ldst_mul4_table[0].next_table_low 	= 21;

	arm_thumb32_asm_ldst_mul5_table[1].next_table 		= arm_thumb32_asm_ldst_mul6_table;
	arm_thumb32_asm_ldst_mul5_table[1].next_table_high 	= 19;
	arm_thumb32_asm_ldst_mul5_table[1].next_table_low 	= 16;

	/* Load store Dual tables */
	arm_thumb32_asm_lv3_table[1].next_table		= arm_thumb32_asm_ldst_dual_table;
	arm_thumb32_asm_lv3_table[1].next_table_high	= 24;
	arm_thumb32_asm_lv3_table[1].next_table_low	= 20;

	arm_thumb32_asm_ldst_dual_table[(0x07)].next_table	 = arm_thumb32_asm_ldst1_dual_table;
	arm_thumb32_asm_ldst_dual_table[(0x07)].next_table_high	 = 19;
	arm_thumb32_asm_ldst_dual_table[(0x07)].next_table_low	 = 16;

	arm_thumb32_asm_ldst_dual_table[(0x08)].next_table	 = arm_thumb32_asm_ldst2_dual_table;
	arm_thumb32_asm_ldst_dual_table[(0x08)].next_table_high	 = 19;
	arm_thumb32_asm_ldst_dual_table[(0x08)].next_table_low	 = 16;

	arm_thumb32_asm_ldst_dual_table[(0x0d)].next_table	 = arm_thumb32_asm_ldst3_dual_table;
	arm_thumb32_asm_ldst_dual_table[(0x0d)].next_table_high	 = 7;
	arm_thumb32_asm_ldst_dual_table[(0x0d)].next_table_low	 = 4;

	/* Data Processing Shifted Reg Tables */
	arm_thumb32_asm_lv2_table[1].next_table 	= arm_thumb32_dproc_shft_reg_table;
	arm_thumb32_asm_lv2_table[1].next_table_high	= 24;
	arm_thumb32_asm_lv2_table[1].next_table_low 	= 21;

	arm_thumb32_dproc_shft_reg_table[0].next_table  	= arm_thumb32_dproc_shft_reg1_table;
	arm_thumb32_dproc_shft_reg_table[0].next_table_high 	= 11;
	arm_thumb32_dproc_shft_reg_table[0].next_table_low 	= 8;

	arm_thumb32_dproc_shft_reg_table[2].next_table  	= arm_thumb32_dproc_shft_reg2_table;
	arm_thumb32_dproc_shft_reg_table[2].next_table_high 	= 19;
	arm_thumb32_dproc_shft_reg_table[2].next_table_low 	= 16;

	arm_thumb32_dproc_shft_reg_table[3].next_table  	= arm_thumb32_dproc_shft_reg3_table;
	arm_thumb32_dproc_shft_reg_table[3].next_table_high 	= 19;
	arm_thumb32_dproc_shft_reg_table[3].next_table_low 	= 16;

	arm_thumb32_dproc_shft_reg_table[4].next_table  	= arm_thumb32_dproc_shft_reg4_table;
	arm_thumb32_dproc_shft_reg_table[4].next_table_high 	= 11;
	arm_thumb32_dproc_shft_reg_table[4].next_table_low 	= 8;

	arm_thumb32_dproc_shft_reg_table[8].next_table  	= arm_thumb32_dproc_shft_reg5_table;
	arm_thumb32_dproc_shft_reg_table[8].next_table_high 	= 11;
	arm_thumb32_dproc_shft_reg_table[8].next_table_low 	= 8;

	arm_thumb32_dproc_shft_reg_table[(0xd)].next_table  		= arm_thumb32_dproc_shft_reg6_table;
	arm_thumb32_dproc_shft_reg_table[(0xd)].next_table_high 	= 11;
	arm_thumb32_dproc_shft_reg_table[(0xd)].next_table_low 		= 8;

	arm_thumb32_dproc_shft_reg2_table[(0xf)].next_table	  	= arm_thumb32_mov_table;
	arm_thumb32_dproc_shft_reg2_table[(0xf)].next_table_high 	= 5;
	arm_thumb32_dproc_shft_reg2_table[(0xf)].next_table_low 	= 4;


	/* Data Processing Immediate Tables */
	arm_thumb32_asm_table[2].next_table 		= arm_thumb32_asm_lv4_table;
	arm_thumb32_asm_table[2].next_table_high 	= 15;
	arm_thumb32_asm_table[2].next_table_low 	= 15;

	arm_thumb32_asm_lv4_table[0].next_table 	= arm_thumb32_asm_lv5_table;
	arm_thumb32_asm_lv4_table[0].next_table_high 	= 25;
	arm_thumb32_asm_lv4_table[0].next_table_low 	= 25;

	arm_thumb32_asm_lv5_table[0].next_table 	= arm_thumb32_dproc_imm_table;
	arm_thumb32_asm_lv5_table[0].next_table_high 	= 24;
	arm_thumb32_asm_lv5_table[0].next_table_low 	= 21;

	arm_thumb32_dproc_imm_table[0].next_table  	= arm_thumb32_dproc_imm1_table;
	arm_thumb32_dproc_imm_table[0].next_table_high 	= 11;
	arm_thumb32_dproc_imm_table[0].next_table_low 	= 8;

	arm_thumb32_dproc_imm_table[2].next_table  	= arm_thumb32_dproc_imm2_table;
	arm_thumb32_dproc_imm_table[2].next_table_high 	= 19;
	arm_thumb32_dproc_imm_table[2].next_table_low 	= 16;

	arm_thumb32_dproc_imm_table[3].next_table  	= arm_thumb32_dproc_imm3_table;
	arm_thumb32_dproc_imm_table[3].next_table_high 	= 19;
	arm_thumb32_dproc_imm_table[3].next_table_low 	= 16;

	arm_thumb32_dproc_imm_table[4].next_table  	= arm_thumb32_dproc_imm4_table;
	arm_thumb32_dproc_imm_table[4].next_table_high 	= 11;
	arm_thumb32_dproc_imm_table[4].next_table_low 	= 8;

	arm_thumb32_dproc_imm_table[8].next_table  	= arm_thumb32_dproc_imm5_table;
	arm_thumb32_dproc_imm_table[8].next_table_high 	= 11;
	arm_thumb32_dproc_imm_table[8].next_table_low 	= 8;

	arm_thumb32_dproc_imm_table[(0xd)].next_table  		= arm_thumb32_dproc_imm6_table;
	arm_thumb32_dproc_imm_table[(0xd)].next_table_high 	= 11;
	arm_thumb32_dproc_imm_table[(0xd)].next_table_low 	= 8;


	/* Data Processing Plain Binary Immediate Tables */
	arm_thumb32_asm_lv5_table[1].next_table 	= arm_thumb32_dproc_bin_imm_table;
	arm_thumb32_asm_lv5_table[1].next_table_high 	= 24;
	arm_thumb32_asm_lv5_table[1].next_table_low 	= 20;

	arm_thumb32_dproc_bin_imm_table[0].next_table 		= arm_thumb32_dproc_bin_imm1_table;
	arm_thumb32_dproc_bin_imm_table[0].next_table_high 	= 19;
	arm_thumb32_dproc_bin_imm_table[0].next_table_low 	= 16;

	arm_thumb32_dproc_bin_imm_table[(0x0a)].next_table 	= arm_thumb32_dproc_bin_imm2_table;
	arm_thumb32_dproc_bin_imm_table[(0x0a)].next_table_high = 19;
	arm_thumb32_dproc_bin_imm_table[(0x0a)].next_table_low 	= 16;

	arm_thumb32_dproc_bin_imm_table[(0x16)].next_table 	= arm_thumb32_dproc_bin_imm3_table;
	arm_thumb32_dproc_bin_imm_table[(0x16)].next_table_high = 19;
	arm_thumb32_dproc_bin_imm_table[(0x16)].next_table_low 	= 16;

	/* Branch_control table */
	arm_thumb32_asm_lv4_table[1].next_table 	= arm_thumb32_brnch_ctrl_table;
	arm_thumb32_asm_lv4_table[1].next_table_high 	= 14;
	arm_thumb32_asm_lv4_table[1].next_table_low 	= 12;

	arm_thumb32_brnch_ctrl_table[0].next_table 	= arm_thumb32_brnch_ctrl1_table;
	arm_thumb32_brnch_ctrl_table[0].next_table_high = 25;
	arm_thumb32_brnch_ctrl_table[0].next_table_low 	= 25;

	arm_thumb32_brnch_ctrl_table[2].next_table 	= arm_thumb32_brnch_ctrl1_table;
	arm_thumb32_brnch_ctrl_table[2].next_table_high = 25;
	arm_thumb32_brnch_ctrl_table[2].next_table_low 	= 23;

	/* Single Data table */
	arm_thumb32_asm_table[3].next_table 		= arm_thumb32_asm_lv6_table;
	arm_thumb32_asm_table[3].next_table_high 	= 26;
	arm_thumb32_asm_table[3].next_table_low 	= 26;

	arm_thumb32_asm_lv6_table[0].next_table 	= arm_thumb32_asm_lv7_table;
	arm_thumb32_asm_lv6_table[0].next_table_high 	= 25;
	arm_thumb32_asm_lv6_table[0].next_table_low 	= 24;

	arm_thumb32_asm_lv7_table[0].next_table 	= arm_thumb32_asm_lv8_table;
	arm_thumb32_asm_lv7_table[0].next_table_high 	= 22;
	arm_thumb32_asm_lv7_table[0].next_table_low 	= 20;

	for(i = 0; i < 8; i++)
	{
		if(!(i % 2))
		{
			arm_thumb32_asm_lv8_table[i].next_table 	= arm_thumb32_st_single_table;
			arm_thumb32_asm_lv8_table[i].next_table_high 	= 23;
			arm_thumb32_asm_lv8_table[i].next_table_low 	= 21;
		}
	}

	arm_thumb32_st_single_table[0].next_table 	= arm_thumb32_st_single1_table;
	arm_thumb32_st_single_table[0].next_table_high 	= 11;
	arm_thumb32_st_single_table[0].next_table_low 	= 11;

	arm_thumb32_st_single1_table[1].next_table 	= arm_thumb32_st_single2_table;
	arm_thumb32_st_single1_table[1].next_table_high	= 9;
	arm_thumb32_st_single1_table[1].next_table_low 	= 9;

	arm_thumb32_st_single_table[1].next_table 	= arm_thumb32_st_single3_table;
	arm_thumb32_st_single_table[1].next_table_high 	= 11;
	arm_thumb32_st_single_table[1].next_table_low 	= 11;

	arm_thumb32_st_single3_table[1].next_table 	= arm_thumb32_st_single4_table;
	arm_thumb32_st_single3_table[1].next_table_high	= 9;
	arm_thumb32_st_single3_table[1].next_table_low 	= 9;

	arm_thumb32_st_single_table[2].next_table 	= arm_thumb32_st_single5_table;
	arm_thumb32_st_single_table[2].next_table_high 	= 11;
	arm_thumb32_st_single_table[2].next_table_low 	= 11;

	arm_thumb32_st_single5_table[1].next_table 	= arm_thumb32_st_single6_table;
	arm_thumb32_st_single5_table[1].next_table_high	= 9;
	arm_thumb32_st_single5_table[1].next_table_low 	= 9;

	/* Load Byte Table */
	arm_thumb32_asm_lv7_table[1].next_table 	= arm_thumb32_asm_lv9_table;
	arm_thumb32_asm_lv7_table[1].next_table_high 	= 22;
	arm_thumb32_asm_lv7_table[1].next_table_low 	= 20;

	arm_thumb32_asm_lv9_table[1].next_table 	= arm_thumb32_ld_byte_table;
	arm_thumb32_asm_lv9_table[1].next_table_high 	= 24;
	arm_thumb32_asm_lv9_table[1].next_table_low 	= 23;

	arm_thumb32_asm_lv8_table[1].next_table 	= arm_thumb32_ld_byte_table;
	arm_thumb32_asm_lv8_table[1].next_table_high 	= 24;
	arm_thumb32_asm_lv8_table[1].next_table_low 	= 23;

	arm_thumb32_ld_byte_table[0].next_table 	= arm_thumb32_ld_byte1_table;
	arm_thumb32_ld_byte_table[0].next_table_high 	= 19;
	arm_thumb32_ld_byte_table[0].next_table_low 	= 16;

	for(i = 0; i < 15; i++)
	{
		arm_thumb32_ld_byte1_table[i].next_table 	= arm_thumb32_ld_byte2_table;
		arm_thumb32_ld_byte1_table[i].next_table_high 	= 11;
		arm_thumb32_ld_byte1_table[i].next_table_low 	= 11;
	}

	arm_thumb32_ld_byte2_table[1].next_table 	= arm_thumb32_ld_byte3_table;
	arm_thumb32_ld_byte2_table[1].next_table_high 	= 10;
	arm_thumb32_ld_byte2_table[1].next_table_low 	= 8;

	arm_thumb32_ld_byte_table[2].next_table 	= arm_thumb32_ld_byte4_table;
	arm_thumb32_ld_byte_table[2].next_table_high 	= 19;
	arm_thumb32_ld_byte_table[2].next_table_low 	= 16;

	for(i = 0; i < 15; i++)
	{
		arm_thumb32_ld_byte4_table[i].next_table 	= arm_thumb32_ld_byte5_table;
		arm_thumb32_ld_byte4_table[i].next_table_high 	= 11;
		arm_thumb32_ld_byte4_table[i].next_table_low 	= 11;
	}

	arm_thumb32_ld_byte5_table[1].next_table 	= arm_thumb32_ld_byte6_table;
	arm_thumb32_ld_byte5_table[1].next_table_high 	= 10;
	arm_thumb32_ld_byte5_table[1].next_table_low 	= 8;

	/* Load Halfword Table */
	arm_thumb32_asm_lv7_table[1].next_table 	= arm_thumb32_asm_lv9_table;
	arm_thumb32_asm_lv7_table[1].next_table_high 	= 22;
	arm_thumb32_asm_lv7_table[1].next_table_low 	= 20;

	arm_thumb32_asm_lv9_table[3].next_table 	= arm_thumb32_ld_hfword_table;
	arm_thumb32_asm_lv9_table[3].next_table_high 	= 24;
	arm_thumb32_asm_lv9_table[3].next_table_low 	= 23;

	arm_thumb32_asm_lv8_table[3].next_table 	= arm_thumb32_ld_hfword_table;
	arm_thumb32_asm_lv8_table[3].next_table_high 	= 24;
	arm_thumb32_asm_lv8_table[3].next_table_low 	= 23;

	arm_thumb32_ld_hfword_table[0].next_table	= arm_thumb32_ld_hfword1_table;
	arm_thumb32_ld_hfword_table[0].next_table_high 	= 11;
	arm_thumb32_ld_hfword_table[0].next_table_low	= 11;

	arm_thumb32_ld_hfword_table[2].next_table	= arm_thumb32_ld_hfword2_table;
	arm_thumb32_ld_hfword_table[2].next_table_high 	= 11;
	arm_thumb32_ld_hfword_table[2].next_table_low	= 11;

	/* Load Word Table */
	arm_thumb32_asm_lv7_table[1].next_table 	= arm_thumb32_asm_lv9_table;
	arm_thumb32_asm_lv7_table[1].next_table_high 	= 22;
	arm_thumb32_asm_lv7_table[1].next_table_low 	= 20;

	arm_thumb32_asm_lv9_table[5].next_table 	= arm_thumb32_ld_word_table;
	arm_thumb32_asm_lv9_table[5].next_table_high 	= 24;
	arm_thumb32_asm_lv9_table[5].next_table_low 	= 23;

	arm_thumb32_asm_lv8_table[5].next_table 	= arm_thumb32_ld_word_table;
	arm_thumb32_asm_lv8_table[5].next_table_high 	= 24;
	arm_thumb32_asm_lv8_table[5].next_table_low 	= 23;

	arm_thumb32_ld_word_table[0].next_table		= arm_thumb32_ld_word1_table;
	arm_thumb32_ld_word_table[0].next_table_high 	= 11;
	arm_thumb32_ld_word_table[0].next_table_low	= 11;

	/* Data Processing Register Based Table */
	arm_thumb32_asm_lv7_table[2].next_table 	= arm_thumb32_dproc_reg_table;
	arm_thumb32_asm_lv7_table[2].next_table_high 	= 23;
	arm_thumb32_asm_lv7_table[2].next_table_low 	= 20;

	arm_thumb32_dproc_reg_table[0].next_table	= arm_thumb32_dproc_reg1_table;
	arm_thumb32_dproc_reg_table[0].next_table_high	= 7;
	arm_thumb32_dproc_reg_table[0].next_table_low	= 7;

	arm_thumb32_dproc_reg_table[1].next_table	= arm_thumb32_dproc_reg2_table;
	arm_thumb32_dproc_reg_table[1].next_table_high	= 7;
	arm_thumb32_dproc_reg_table[1].next_table_low	= 7;

	arm_thumb32_dproc_reg2_table[1].next_table	= arm_thumb32_dproc_reg3_table;
	arm_thumb32_dproc_reg2_table[1].next_table_high	= 19;
	arm_thumb32_dproc_reg2_table[1].next_table_low	= 16;

	arm_thumb32_dproc_reg_table[2].next_table	= arm_thumb32_dproc_reg4_table;
	arm_thumb32_dproc_reg_table[2].next_table_high	= 7;
	arm_thumb32_dproc_reg_table[2].next_table_low	= 7;

	arm_thumb32_dproc_reg_table[3].next_table	= arm_thumb32_dproc_reg5_table;
	arm_thumb32_dproc_reg_table[3].next_table_high	= 7;
	arm_thumb32_dproc_reg_table[3].next_table_low	= 7;

	arm_thumb32_dproc_reg_table[4].next_table	= arm_thumb32_dproc_reg6_table;
	arm_thumb32_dproc_reg_table[4].next_table_high	= 7;
	arm_thumb32_dproc_reg_table[4].next_table_low	= 7;

	arm_thumb32_dproc_reg_table[5].next_table	= arm_thumb32_dproc_reg7_table;
	arm_thumb32_dproc_reg_table[5].next_table_high	= 7;
	arm_thumb32_dproc_reg_table[5].next_table_low	= 7;

	arm_thumb32_dproc_reg_table[8].next_table	= arm_thumb32_dproc_misc_table;
	arm_thumb32_dproc_reg_table[8].next_table_high	= 7;
	arm_thumb32_dproc_reg_table[8].next_table_low	= 6;

	arm_thumb32_dproc_misc_table[2].next_table	= arm_thumb32_dproc_misc1_table;
	arm_thumb32_dproc_misc_table[2].next_table_high	= 21;
	arm_thumb32_dproc_misc_table[2].next_table_low	= 20;


	/* Multiply Tables */
	arm_thumb32_asm_lv7_table[3].next_table 	= arm_thumb32_asm_lv10_table;
	arm_thumb32_asm_lv7_table[3].next_table_high 	= 23;
	arm_thumb32_asm_lv7_table[3].next_table_low 	= 23;

	arm_thumb32_asm_lv10_table[0].next_table 	= arm_thumb32_mult_table;
	arm_thumb32_asm_lv10_table[0].next_table_high 	= 5;
	arm_thumb32_asm_lv10_table[0].next_table_low 	= 4;

	arm_thumb32_mult_table[0].next_table 		= arm_thumb32_mult1_table;
	arm_thumb32_mult_table[0].next_table_high 	= 15;
	arm_thumb32_mult_table[0].next_table_low 	= 12;

	/* Multiply Long Tables */
	arm_thumb32_asm_lv10_table[1].next_table 	= arm_thumb32_mult_long_table;
	arm_thumb32_asm_lv10_table[1].next_table_high 	= 22;
	arm_thumb32_asm_lv10_table[1].next_table_low 	= 20;

#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6,_op7,_op8) \
	arm_thumb32_setup_table(#_name, _fmt_str, ARM_THUMB32_CAT_##_cat, _op1, _op2,\
	_op3, _op4, _op5, _op6, _op7, _op8, ARM_THUMB32_INST_##_name);
#include "asm-thumb32.dat"
#undef DEFINST
}

void arm_thumb32_setup_table(char* name , char* fmt_str ,
	enum arm_thumb32_cat_enum cat32 , int op1 , int op2 , int op3 ,
	int op4 , int op5 , int op6, int op7, int op8, enum arm_thumb32_inst_enum inst_name)
{
	struct arm_thumb32_inst_info_t *current_table;
	/* We initially start with the first table arm_asm_table, with the opcode field as argument */
	current_table = arm_thumb32_asm_table;
	int op[8];
	int i;

	op[0] = op1;
	op[1] = op2;
	op[2] = op3;
	op[3] = op4;
	op[4] = op5;
	op[5] = op6;
	op[6] = op7;
	op[7] = op8;

	i = 0;
	while(1)
	{
		if(current_table[op[i]].next_table && (op[i] >= 0))
		{
				current_table = current_table[op[i]].next_table;
				i++;
		}
		else
		{
			current_table[op[i]].name = name;
			current_table[op[i]].fmt_str = fmt_str;
			current_table[op[i]].cat32 = cat32;
			current_table[op[i]].size = 4;
			current_table[op[i]].inst_32 = inst_name;

			break;
		}
	}
}

void arm_thumb16_disasm_init()
{
	//int op[6];
	int i = 0;
	arm_thumb16_asm_table 			= xcalloc(8, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_shft_ins_table		= xcalloc(16, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_shft_ins_lv2_table		= xcalloc(16, sizeof(struct arm_thumb16_inst_info_t));

	arm_thumb16_asm_lv1_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv2_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv3_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv4_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv5_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv6_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv7_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv8_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv9_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));


	arm_thumb16_data_proc_table		= xcalloc(32, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_spcl_data_brex_table	= xcalloc(32, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_spcl_data_brex_lv1_table	= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));

	arm_thumb16_ld_st_table			= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_ld_st_lv1_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_ld_st_lv2_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));


	arm_thumb16_misc_table			= xcalloc(128, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_it_table			= xcalloc(256, sizeof(struct arm_thumb16_inst_info_t));


	/* Directing to Shift Instructions */
	arm_thumb16_asm_table[0].next_table 	= arm_thumb16_shft_ins_table;
	arm_thumb16_asm_table[0].next_table_high= 13;
	arm_thumb16_asm_table[0].next_table_low	= 11;

	arm_thumb16_shft_ins_table[3].next_table = arm_thumb16_shft_ins_lv2_table;
	arm_thumb16_shft_ins_table[3].next_table_high = 10;
	arm_thumb16_shft_ins_table[3].next_table_low = 9;

	/* Directing to Data Processing Instructions */
	arm_thumb16_asm_table[1].next_table 	= arm_thumb16_asm_lv1_table;
	arm_thumb16_asm_table[1].next_table_high= 13;
	arm_thumb16_asm_table[1].next_table_low	= 13;

	arm_thumb16_asm_lv1_table[0].next_table 	= arm_thumb16_asm_lv2_table;
	arm_thumb16_asm_lv1_table[0].next_table_high	= 12;
	arm_thumb16_asm_lv1_table[0].next_table_low	= 12;

	arm_thumb16_asm_lv2_table[0].next_table 	= arm_thumb16_asm_lv3_table;
	arm_thumb16_asm_lv2_table[0].next_table_high	= 11;
	arm_thumb16_asm_lv2_table[0].next_table_low	= 10;

	arm_thumb16_asm_lv3_table[0].next_table 	= arm_thumb16_data_proc_table;
	arm_thumb16_asm_lv3_table[0].next_table_high	= 9;
	arm_thumb16_asm_lv3_table[0].next_table_low	= 6;

	/* Directing to LD/ST Instructions */
	arm_thumb16_asm_lv1_table[1].next_table 	= arm_thumb16_ld_st_table;
	arm_thumb16_asm_lv1_table[1].next_table_high	= 15;
	arm_thumb16_asm_lv1_table[1].next_table_low	= 11;

	arm_thumb16_asm_lv2_table[1].next_table 	= arm_thumb16_ld_st_table;
	arm_thumb16_asm_lv2_table[1].next_table_high	= 15;
	arm_thumb16_asm_lv2_table[1].next_table_low	= 11;

	arm_thumb16_asm_lv4_table[0].next_table 	= arm_thumb16_ld_st_table;
	arm_thumb16_asm_lv4_table[0].next_table_high	= 15;
	arm_thumb16_asm_lv4_table[0].next_table_low	= 11;

	arm_thumb16_ld_st_table[10].next_table		= arm_thumb16_ld_st_lv1_table;
	arm_thumb16_ld_st_table[10].next_table_high 	= 10;
	arm_thumb16_ld_st_table[10].next_table_low 	= 9;

	arm_thumb16_ld_st_table[11].next_table 		= arm_thumb16_ld_st_lv2_table;
	arm_thumb16_ld_st_table[11].next_table_high 	= 10;
	arm_thumb16_ld_st_table[11].next_table_low 	= 9;

	/* Directing to Special data Instructions and B&EX instructions*/
	arm_thumb16_asm_lv3_table[1].next_table 	= arm_thumb16_spcl_data_brex_table;
	arm_thumb16_asm_lv3_table[1].next_table_high	= 9;
	arm_thumb16_asm_lv3_table[1].next_table_low	= 7;

	arm_thumb16_spcl_data_brex_table[0].next_table 	= arm_thumb16_spcl_data_brex_lv1_table;
	arm_thumb16_spcl_data_brex_table[0].next_table_high	= 6;
	arm_thumb16_spcl_data_brex_table[0].next_table_low	= 6;

	/* Directing to Misellaneous 16 bit thumb2 instructions */
	arm_thumb16_asm_table[2].next_table = arm_thumb16_asm_lv4_table;
	arm_thumb16_asm_table[2].next_table_high = 13;
	arm_thumb16_asm_table[2].next_table_low = 13;

	arm_thumb16_asm_lv4_table[1].next_table 	= arm_thumb16_asm_lv5_table;
	arm_thumb16_asm_lv4_table[1].next_table_high	= 12;
	arm_thumb16_asm_lv4_table[1].next_table_low	= 12;

	arm_thumb16_asm_lv5_table[1].next_table 	= arm_thumb16_misc_table;
	arm_thumb16_asm_lv5_table[1].next_table_high	= 11;
	arm_thumb16_asm_lv5_table[1].next_table_low	= 5;

	for(i = 0; i < 8; i++)
	{
		arm_thumb16_misc_table[(0x78 + i)].next_table 		= arm_thumb16_it_table;
		arm_thumb16_misc_table[(0x78 + i)].next_table_high 	= 3;
		arm_thumb16_misc_table[(0x78 + i)].next_table_low 	= 0;
	}

	/* Directing to PC and SP relative instructions */
	arm_thumb16_asm_lv5_table[0].next_table 	= arm_thumb16_asm_lv6_table;
	arm_thumb16_asm_lv5_table[0].next_table_high	= 11;
	arm_thumb16_asm_lv5_table[0].next_table_low	= 11;

	arm_thumb16_asm_table[3].next_table = arm_thumb16_asm_lv7_table;
	arm_thumb16_asm_table[3].next_table_high = 13;
	arm_thumb16_asm_table[3].next_table_low = 12;

	/* Directing to Software interrupt instructions */
	arm_thumb16_asm_lv7_table[0].next_table 	= arm_thumb16_asm_lv8_table;
	arm_thumb16_asm_lv7_table[0].next_table_high	= 11;
	arm_thumb16_asm_lv7_table[0].next_table_low	= 11;

	/* Directing to unconditional branch instructions */
	arm_thumb16_asm_lv7_table[1].next_table 	= arm_thumb16_asm_lv9_table; // entries [0 to 0xe] of lv9 are inst B //
	arm_thumb16_asm_lv7_table[1].next_table_high	= 11;
	arm_thumb16_asm_lv7_table[1].next_table_low	= 8;



#define DEFINST(_name,_fmt_str,_cat,_op1,_op2,_op3,_op4,_op5,_op6) \
	arm_thumb16_setup_table(#_name, _fmt_str, ARM_THUMB16_CAT_##_cat, _op1, _op2,\
	_op3, _op4, _op5, _op6, ARM_THUMB16_INST_##_name);
#include "asm-thumb.dat"
#undef DEFINST
}

void arm_thumb16_setup_table(char* name , char* fmt_str ,
	enum arm_thumb16_cat_enum cat16 , int op1 , int op2 , int op3 ,
	int op4 , int op5 , int op6, enum arm_thumb16_inst_enum inst_name)
{
	struct arm_thumb16_inst_info_t *current_table;
	/* We initially start with the first table arm_asm_table, with the opcode field as argument */
	current_table = arm_thumb16_asm_table;
	int op[6];
	int i;

	op[0] = op1;
	op[1] = op2;
	op[2] = op3;
	op[3] = op4;
	op[4] = op5;
	op[5] = op6;

	i = 0;
	while(1)
	{
		if(current_table[op[i]].next_table && (op[i] >= 0))
		{
				current_table = current_table[op[i]].next_table;
				i++;
		}
		else
		{
			current_table[op[i]].name = name;
			current_table[op[i]].fmt_str = fmt_str;
			current_table[op[i]].cat16 = cat16;
			current_table[op[i]].size = 2;
			current_table[op[i]].inst_16 = inst_name;

			break;
		}
	}
}


static int arm_token_comp(char *fmt_str, char *token_str, int *token_len)
{
	*token_len = strlen(token_str);
	return !strncmp(fmt_str, token_str, *token_len) &&
		!isalnum(fmt_str[*token_len]);
}


void arm_thumb16_inst_dump(FILE *f , char *str , int inst_str_size , void *inst_ptr ,
	unsigned int inst_index, unsigned int inst_addr)
{
	struct arm_thumb16_inst_t inst;
	int byte_index;
	char *inst_str;
	char **inst_str_ptr;
	char *fmt_str;
	int token_len;

	inst.addr = inst_index;
	for (byte_index = 0; byte_index < 2; ++byte_index)
		inst.dword.bytes[byte_index] = *(unsigned char *) (inst_ptr
			+ byte_index);

	arm_thumb16_inst_decode(&inst); // Change to thumb2
	inst_str = str;
	inst_str_ptr = &str;
	fmt_str = inst.info->fmt_str;

	if (fmt_str)
	{
		while (*fmt_str)
		{
			if (*fmt_str != '%')
			{
				if (!(*fmt_str == ' ' && *inst_str_ptr == inst_str))
					str_printf(inst_str_ptr, &inst_str_size, "%c",
						*fmt_str);
				++fmt_str;
				continue;
			}

			++fmt_str;
			if (arm_token_comp(fmt_str, "rd", &token_len))
				arm_thumb16_inst_dump_RD(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat16);
			else if (arm_token_comp(fmt_str, "rm", &token_len))
				arm_thumb16_inst_dump_RM(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat16);
			else if (arm_token_comp(fmt_str, "rn", &token_len))
				arm_thumb16_inst_dump_RN(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat16);
			else if (arm_token_comp(fmt_str, "immd8", &token_len))
				arm_thumb16_inst_dump_IMMD8(inst_str_ptr, &inst_str_size, &inst,
								inst.info->cat16, inst_addr);
			else if (arm_token_comp(fmt_str, "immd5", &token_len))
				arm_thumb16_inst_dump_IMMD5(inst_str_ptr, &inst_str_size, &inst,
								inst.info->cat16, inst_addr);
			else if (arm_token_comp(fmt_str, "immd3", &token_len))
				arm_thumb16_inst_dump_IMMD3(inst_str_ptr, &inst_str_size, &inst,
								inst.info->cat16);
			else if (arm_token_comp(fmt_str, "cond", &token_len))
				arm_thumb16_inst_dump_COND(inst_str_ptr, &inst_str_size, &inst,
								inst.info->cat16);
			else if (arm_token_comp(fmt_str, "regs", &token_len))
				arm_thumb16_inst_dump_REGS(inst_str_ptr, &inst_str_size, &inst,
											inst.info->cat16);
			else if (arm_token_comp(fmt_str, "x", &token_len))
				arm_thumb16_inst_dump_it_eq_x(inst_str_ptr, &inst_str_size, &inst,
								inst.info->cat16);
			else
				fatal("%s: token not recognized\n", fmt_str);

			fmt_str += token_len;
		}
		fprintf(f, "%s\n", inst_str);
	}
	else
	{
		fprintf (f,"???\n");
	}
}

void arm_thumb16_inst_dump_RD(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int rd;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			rd = inst->dword.movshift_reg_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			rd = inst->dword.addsub_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			rd = inst->dword.immd_oprs_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			rd = inst->dword.dpr_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			rd = ((inst->dword.high_oprs_ins.h1 << 3) |  inst->dword.high_oprs_ins.reg_rd);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			rd = inst->dword.pcldr_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			rd = inst->dword.ldstr_reg_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			rd = inst->dword.ldstr_exts_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			rd = inst->dword.ldstr_immd_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			rd = inst->dword.ldstr_hfwrd_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			rd = inst->dword.sp_immd_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			rd = inst->dword.addsp_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_MISC_REV)
			rd = inst->dword.rev_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_CMP_T2)
			rd = (inst->dword.cmp_t2.N << 3 | inst->dword.cmp_t2.reg_rn);

		else
			fatal("%d: rd fmt not recognized", cat);

		switch (rd)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rd);
			break;
		}
}

void arm_thumb16_inst_dump_RM(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int rm;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			rm = inst->dword.movshift_reg_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			rm = inst->dword.addsub_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			rm = inst->dword.dpr_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			rm = inst->dword.high_oprs_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			rm = inst->dword.ldstr_reg_ins.reg_ro;
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			rm = inst->dword.ldstr_exts_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_REV)
			rm = inst->dword.rev_ins.reg_rm;
		else if (cat == ARM_THUMB16_CAT_CMP_T2)
			rm = inst->dword.cmp_t2.reg_rm;
		else
			fatal("%d: rm fmt not recognized", cat);

		switch (rm)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rm);
			break;
		}
}

void arm_thumb16_inst_dump_RN(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int rn;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			rn = inst->dword.addsub_ins.reg_rs;
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			rn = inst->dword.immd_oprs_ins.reg_rd;
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			rn = inst->dword.ldstr_reg_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			rn = inst->dword.ldstr_exts_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			rn = inst->dword.ldstr_immd_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			rn = inst->dword.ldstr_hfwrd_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			rn = inst->dword.ldm_stm_ins.reg_rb;
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_CBNZ)
			rn = inst->dword.cbnz_ins.reg_rn;
		else
			fatal("%d: rn fmt not recognized", cat);

		switch (rn)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rn);
			break;
		}
}

void arm_thumb16_inst_dump_IMMD8(char **inst_str_ptr , int *inst_str_size ,
	struct arm_thumb16_inst_t *inst , enum arm_thumb16_cat_enum cat ,
	unsigned int inst_addr)
{
	unsigned int immd8;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			immd8 = inst->dword.immd_oprs_ins.offset8;
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			immd8 =(inst->dword.pcldr_ins.immd_8 << 2);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			immd8 = 4 * inst->dword.sp_immd_ins.immd_8;
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: immd8 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			immd8 = 4 * inst->dword.addsp_ins.immd_8;
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			immd8 = 4 * inst->dword.sub_sp_ins.immd_8;
		else if (cat == ARM_THUMB16_CAT_MISC_BR)
			immd8 = inst->dword.cond_br_ins.s_offset;
		else if (cat == ARM_THUMB16_CAT_MISC_UCBR)
			immd8 = inst->dword.br_ins.immd11;
		else if (cat == ARM_THUMB16_CAT_MISC_SVC_INS)
			immd8 = inst->dword.svc_ins.value;
		else
			fatal("%d: immd8 fmt not recognized", cat);

		if(cat == ARM_THUMB16_CAT_MISC_BR)
		{
			if((immd8 >> 7))
			{
				immd8 = ((inst_addr + 4) + ((immd8 << 1) | 0xffffff00));
			}
			else
			{
				immd8 = (inst_addr + 4) + (immd8 << 1);
			}
			str_printf(inst_str_ptr, inst_str_size, "%x",immd8);
		}
		else if(cat == ARM_THUMB16_CAT_MISC_UCBR)
		{
			immd8 = immd8 << 1;
			immd8 = SEXT32(immd8, 12);

			immd8 = inst_addr + 4 + immd8;
			str_printf(inst_str_ptr, inst_str_size, "%x",immd8);
		}

		else
			str_printf(inst_str_ptr, inst_str_size, "#%d",immd8);

}

void arm_thumb16_inst_dump_IMMD3(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int immd3;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			immd3 = inst->dword.addsub_ins.rn_imm;
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			fatal("%d: immd3 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_BR)
			fatal("%d: immd3 fmt not recognized", cat);

		else
			fatal("%d: immd3 fmt not recognized", cat);



		str_printf(inst_str_ptr, inst_str_size, "#%d",immd3);

}

void arm_thumb16_inst_dump_IMMD5(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat,
	unsigned int inst_addr)
{
	unsigned int immd5;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			immd5 = inst->dword.movshift_reg_ins.offset;
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			immd5 = inst->dword.ldstr_immd_ins.offset << 2;
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			immd5 = inst->dword.ldstr_hfwrd_ins.offset;
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			fatal("%d: immd5 fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_CBNZ)
			immd5 = inst->dword.cbnz_ins.immd_5;
		else
			fatal("%d: immd5 fmt not recognized", cat);

		if(cat == ARM_THUMB16_CAT_MISC_CBNZ)
		{
			if((inst_addr + 2) % 4)
				immd5 = (inst_addr + 4) + (immd5 << 1);
			else
				immd5 = (inst_addr + 2) + (immd5 << 1);

			str_printf(inst_str_ptr, inst_str_size, "%x",immd5);
		}
		else
			str_printf(inst_str_ptr, inst_str_size, "#%d",immd5);

}

void arm_thumb16_inst_dump_COND(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int cond;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			cond = inst->dword.if_eq_ins.first_cond;
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			fatal("%d: cond fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_BR)
			cond = inst->dword.cond_br_ins.cond;
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			cond = inst->dword.if_eq_ins.first_cond;

		else
			fatal("%d: rm fmt not recognized", cat);

		switch (cond)
			{
			case (EQ):
			str_printf(inst_str_ptr, inst_str_size, "eq");
			break;

			case (NE):
			str_printf(inst_str_ptr, inst_str_size, "ne");
			break;

			case (CS):
			str_printf(inst_str_ptr, inst_str_size, "cs");
			break;

			case (CC):
			str_printf(inst_str_ptr, inst_str_size, "cc");
			break;

			case (MI):
			str_printf(inst_str_ptr, inst_str_size, "mi");
			break;

			case (PL):
			str_printf(inst_str_ptr, inst_str_size, "pl");
			break;

			case (VS):
			str_printf(inst_str_ptr, inst_str_size, "vs");
			break;

			case (VC):
			str_printf(inst_str_ptr, inst_str_size, "vc");
			break;

			case (HI):
			str_printf(inst_str_ptr, inst_str_size, "hi");
			break;

			case (LS):
			str_printf(inst_str_ptr, inst_str_size, "ls");
			break;

			case (GE):
			str_printf(inst_str_ptr, inst_str_size, "ge");
			break;

			case (LT):
			str_printf(inst_str_ptr, inst_str_size, "lt");
			break;

			case (GT):
			str_printf(inst_str_ptr, inst_str_size, "gt");
			break;

			case (LE):
			str_printf(inst_str_ptr, inst_str_size, "le");
			break;

			case (AL):
			str_printf(inst_str_ptr, inst_str_size, " ");
			break;
			}

}

void arm_thumb16_inst_dump_REGS(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int regs;
	unsigned int i;

		if (cat == ARM_THUMB16_CAT_MOVSHIFT_REG)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_ADDSUB)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IMMD_OPRS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_DPR_INS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_HI_REG_OPRS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_PC_LDR)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_REG)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_EXTS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_IMMD)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_HFWRD)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDSTR_SP_IMMD)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_IF_THEN)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_LDM_STM)
			regs = inst->dword.ldm_stm_ins.reg_list;
		else if (cat == ARM_THUMB16_CAT_MISC_ADDSP_INS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_SUBSP_INS)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_BR)
			fatal("%d: regs fmt not recognized", cat);
		else if (cat == ARM_THUMB16_CAT_MISC_PUSH_POP)
			regs = inst->dword.push_pop_ins.reg_list;
		else
			fatal("%d: regs fmt not recognized", cat);

		regs = (inst->dword.push_pop_ins.m_ext << 14) | regs;
		str_printf(inst_str_ptr, inst_str_size, "{");
		for (i = 1; i < 65536; i *= 2)
		{
			if(regs & (i))
			{
				str_printf(inst_str_ptr, inst_str_size, "r%d ", log_base2(i));
			}
		}

		str_printf(inst_str_ptr, inst_str_size, "}");


}

void arm_thumb16_inst_dump_it_eq_x(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb16_inst_t *inst, enum arm_thumb16_cat_enum cat)
{
	unsigned int first_cond;
	unsigned int mask;

	if (cat == ARM_THUMB16_CAT_IF_THEN)
	{
		mask = inst->dword.if_eq_ins.mask;
		first_cond = inst->dword.if_eq_ins.first_cond;
	}
	else
		fatal("%d: x fmt not recognized", cat);

	if((mask != 0x8))
	{
		if((mask >> 3) ^ (first_cond & 1))
			str_printf(inst_str_ptr, inst_str_size, "e");
		else
			str_printf(inst_str_ptr, inst_str_size, "t");
	}
}

void arm_thumb32_inst_dump(FILE *f , char *str , int inst_str_size , void *inst_ptr ,
	unsigned int inst_index, unsigned int inst_addr)
{
	struct arm_thumb32_inst_t inst;
	int byte_index;
	char *inst_str;
	char **inst_str_ptr;
	char *fmt_str;
	int token_len;

	inst.addr = inst_index;

	for (byte_index = 0; byte_index < 4; ++byte_index)
		inst.dword.bytes[byte_index] = *(unsigned char *) (inst_ptr
			+ ((byte_index + 2) % 4));


	arm_thumb32_inst_decode(&inst); // Change to thumb2
	inst_str = str;
	inst_str_ptr = &str;
	fmt_str = inst.info->fmt_str;

	if (fmt_str)
	{
		while (*fmt_str)
		{
			if (*fmt_str != '%')
			{
				if (!(*fmt_str == ' ' && *inst_str_ptr == inst_str))
					str_printf(inst_str_ptr, &inst_str_size, "%c",
						*fmt_str);
				++fmt_str;
				continue;
			}

			++fmt_str;
			if (arm_token_comp(fmt_str, "rd", &token_len))
				arm_thumb32_inst_dump_RD(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "rn", &token_len))
				arm_thumb32_inst_dump_RN(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "rm", &token_len))
				arm_thumb32_inst_dump_RM(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "rt", &token_len))
				arm_thumb32_inst_dump_RT(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "rt2", &token_len))
				arm_thumb32_inst_dump_RT2(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "ra", &token_len))
				arm_thumb32_inst_dump_RA(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "rdlo", &token_len))
				arm_thumb32_inst_dump_RDLO(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "rdhi", &token_len))
				arm_thumb32_inst_dump_RDHI(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "imm12", &token_len))
				arm_thumb32_inst_dump_IMM12(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "imm8", &token_len))
				arm_thumb32_inst_dump_IMM12(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "imm2", &token_len))
				arm_thumb32_inst_dump_IMM2(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "immd8", &token_len))
				arm_thumb32_inst_dump_IMMD8(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "immd12", &token_len))
				arm_thumb32_inst_dump_IMMD12(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "immd16", &token_len))
				arm_thumb32_inst_dump_IMMD16(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "addr", &token_len))
				arm_thumb32_inst_dump_ADDR(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32, inst_addr);
			else if (arm_token_comp(fmt_str, "regs", &token_len))
				arm_thumb32_inst_dump_REGS(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "shft", &token_len))
				arm_thumb32_inst_dump_SHFT_REG(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "S", &token_len))
				arm_thumb32_inst_dump_S(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "lsb", &token_len))
				arm_thumb32_inst_dump_LSB(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "wid", &token_len))
				arm_thumb32_inst_dump_WID(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);
			else if (arm_token_comp(fmt_str, "cond", &token_len))
				arm_thumb32_inst_dump_COND(inst_str_ptr, &inst_str_size, &inst,
					inst.info->cat32);



			else
				fatal("%s: token not recognized\n", fmt_str);

			fmt_str += token_len;
		}
		fprintf(f, "%s\n", inst_str);
	}
	else
	{
		fprintf (f,"???\n");
	}

}

void arm_thumb32_inst_dump_RD(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int rd;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_PUSH_POP)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_TABLE_BRNCH)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
			rd = inst->dword.data_proc_shftreg.rd;
		else if (cat == ARM_THUMB32_CAT_DPR_IMM)
			rd = inst->dword.data_proc_immd.rd;
		else if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
			rd = inst->dword.data_proc_immd.rd;
		else if (cat == ARM_THUMB32_CAT_BRANCH)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LDSTR_BYTE)
			rd = inst->dword.ldstr_reg.rd;
		else if (cat == ARM_THUMB32_CAT_LDSTR_REG)
			rd = inst->dword.ldstr_reg.rd;
		else if (cat == ARM_THUMB32_CAT_LDSTR_IMMD)
			rd = inst->dword.ldstr_imm.rd;
		else if (cat == ARM_THUMB32_CAT_DPR_REG)
			rd = inst->dword.dproc_reg.rd;
		else if (cat == ARM_THUMB32_CAT_MULT)
			rd = inst->dword.mult.rd;
		else if (cat == ARM_THUMB32_CAT_MULT_LONG)
			fatal("%d: rd fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_BIT_FIELD)
			rd = inst->dword.bit_field.rd;

		else
			fatal("%d: rd fmt not recognized", cat);

		switch (rd)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rd);
			break;
		}

}

void arm_thumb32_inst_dump_RN(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int rn;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			rn = inst->dword.ld_st_mult.rn;
		else if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			rn = inst->dword.ld_st_double.rn;
		else if (cat == ARM_THUMB32_CAT_PUSH_POP)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_TABLE_BRNCH)
			rn = inst->dword.table_branch.rn;
		else if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
			rn = inst->dword.data_proc_shftreg.rn;
		else if (cat == ARM_THUMB32_CAT_DPR_IMM)
			rn = inst->dword.data_proc_immd.rn;
		else if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
			rn = inst->dword.data_proc_immd.rn;
		else if (cat == ARM_THUMB32_CAT_BRANCH)
			fatal("%d: rn fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LDSTR_BYTE)
			rn = inst->dword.ldstr_reg.rn;
		else if (cat == ARM_THUMB32_CAT_LDSTR_REG)
			rn = inst->dword.ldstr_reg.rn;
		else if (cat == ARM_THUMB32_CAT_LDSTR_IMMD)
			rn = inst->dword.ldstr_imm.rn;
		else if (cat == ARM_THUMB32_CAT_DPR_REG)
			rn = inst->dword.dproc_reg.rn;
		else if (cat == ARM_THUMB32_CAT_MULT)
			rn = inst->dword.mult.rn;
		else if (cat == ARM_THUMB32_CAT_MULT_LONG)
			rn = inst->dword.mult_long.rn;
		else if (cat == ARM_THUMB32_CAT_BIT_FIELD)
			rn = inst->dword.bit_field.rn;

		else
			fatal("%d: rn fmt not recognized", cat);

		switch (rn)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rn);
			break;
		}

}

void arm_thumb32_inst_dump_RM(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int rm;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_PUSH_POP)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_TABLE_BRNCH)
			rm = inst->dword.table_branch.rm;
		else if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
			rm = inst->dword.data_proc_shftreg.rm;
		else if (cat == ARM_THUMB32_CAT_DPR_IMM)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_BRANCH)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LDSTR_BYTE)
			rm = inst->dword.ldstr_reg.rm;
		else if (cat == ARM_THUMB32_CAT_LDSTR_REG)
			rm = inst->dword.ldstr_reg.rm;
		else if (cat == ARM_THUMB32_CAT_LDSTR_IMMD)
			fatal("%d: rm fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_DPR_REG)
			rm = inst->dword.dproc_reg.rm;
		else if (cat == ARM_THUMB32_CAT_MULT)
			rm = inst->dword.mult.rm;
		else if (cat == ARM_THUMB32_CAT_MULT_LONG)
			rm = inst->dword.mult_long.rm;
		else if (cat == ARM_THUMB32_CAT_BIT_FIELD)
			fatal("%d: rm fmt not recognized", cat);

		else
			fatal("%d: rm fmt not recognized", cat);

		switch (rm)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rm);
			break;
		}

}

void arm_thumb32_inst_dump_RT(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int rt;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			fatal("%d: rt fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			rt = inst->dword.ld_st_double.rt;
		else
			fatal("%d: rt fmt not recognized", cat);

		switch (rt)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rt);
			break;
		}
}

void arm_thumb32_inst_dump_RT2(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int rt2;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			fatal("%d: rt fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			rt2 = inst->dword.ld_st_double.rt2;
		else
			fatal("%d: rt2 fmt not recognized", cat);

		switch (rt2)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rt2);
			break;
		}
}

void arm_thumb32_inst_dump_RA(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int ra;


		if (cat == ARM_THUMB32_CAT_MULT)
			ra = inst->dword.mult.ra;
		else
			fatal("%d: ra fmt not recognized", cat);

		switch (ra)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", ra);
			break;
		}
}

void arm_thumb32_inst_dump_RDLO(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int rdlo;


		if (cat == ARM_THUMB32_CAT_MULT_LONG)
			rdlo = inst->dword.mult_long.rdlo;
		else
			fatal("%d: rdlo fmt not recognized", cat);

		switch (rdlo)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rdlo);
			break;
		}
}

void arm_thumb32_inst_dump_RDHI(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int rdhi;


		if (cat == ARM_THUMB32_CAT_MULT_LONG)
			rdhi = inst->dword.mult_long.rdhi;
		else
			fatal("%d: rdhi fmt not recognized", cat);

		switch (rdhi)
		{
		case (r13):
			str_printf(inst_str_ptr, inst_str_size, "sp");
		break;

		case (r14):
			str_printf(inst_str_ptr, inst_str_size, "lr");
		break;
		case (r15):

			str_printf(inst_str_ptr, inst_str_size, "pc");
		break;

		default:
			str_printf(inst_str_ptr, inst_str_size, "r%d", rdhi);
			break;
		}
}

void arm_thumb32_inst_dump_S(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int sign;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_PUSH_POP)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_TABLE_BRNCH)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
			sign = inst->dword.data_proc_shftreg.sign;
		else if (cat == ARM_THUMB32_CAT_DPR_IMM)
			sign = inst->dword.data_proc_immd.sign;
		else if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
			sign = inst->dword.data_proc_immd.sign;
		else if (cat == ARM_THUMB32_CAT_BRANCH)
			sign = inst->dword.branch.sign;
		else if (cat == ARM_THUMB32_CAT_LDSTR_BYTE)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LDSTR_REG)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_LDSTR_IMMD)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_DPR_REG)
			sign = inst->dword.dproc_reg.sign;
		else if (cat == ARM_THUMB32_CAT_MULT)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_MULT_LONG)
			fatal("%d: S fmt not recognized", cat);
		else if (cat == ARM_THUMB32_CAT_BIT_FIELD)
			fatal("%d: sign fmt not recognized", cat);

		else
			fatal("%d: sign fmt not recognized", cat);
		if(sign)
			str_printf(inst_str_ptr, inst_str_size, "s");
}

void arm_thumb32_inst_dump_REGS(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int regs;
	unsigned int i;

		if (cat == ARM_THUMB32_CAT_LD_ST_MULT)
			regs = inst->dword.ld_st_mult.reglist;
		else if (cat == ARM_THUMB32_CAT_PUSH_POP)
			regs = inst->dword.push_pop.reglist;

		else
			fatal("%d: regs fmt not recognized", cat);

		str_printf(inst_str_ptr, inst_str_size, "{");
		for (i = 1; i < 65536; i *= 2)
		{
			if(regs & (i))
			{
				str_printf(inst_str_ptr, inst_str_size, "r%d ", log_base2(i));
			}
		}

		str_printf(inst_str_ptr, inst_str_size, "}");

}

void arm_thumb32_inst_dump_SHFT_REG(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int shift;
	unsigned int type;

	if (cat == ARM_THUMB32_CAT_DPR_SHFTREG)
	{
		type = inst->dword.data_proc_shftreg.type;
		shift = (inst->dword.data_proc_shftreg.imm3 << 2) | (inst->dword.data_proc_shftreg.imm2);
	}

	else
		fatal("%d: shft fmt not recognized", cat);

	if(shift)
	{
		switch(type)
		{
		case (ARM_OPTR_LSL):
			str_printf(inst_str_ptr, inst_str_size, "{lsl #%d}", shift);
		break;

		case (ARM_OPTR_LSR):
			str_printf(inst_str_ptr, inst_str_size, "{lsr #%d}", shift);
		break;

		case (ARM_OPTR_ASR):
			str_printf(inst_str_ptr, inst_str_size, "{asr #%d}", shift);
		break;

		case (ARM_OPTR_ROR):
			str_printf(inst_str_ptr, inst_str_size, "{ror #%d}", shift);
		break;
		}
	}

}

void arm_thumb32_inst_dump_IMM12(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int immd12;
	unsigned int idx;
	unsigned int wback;
	unsigned int add;

		if (cat == ARM_THUMB32_CAT_LDSTR_IMMD)
			immd12 = inst->dword.ldstr_imm.immd12;
		else if (cat == ARM_THUMB32_CAT_LDSTR_BYTE)
			immd12 = inst->dword.ldstr_imm.immd12;
		else
			fatal("%d: imm12 fmt not recognized", cat);

		if(inst->dword.ldstr_imm.add)
			str_printf(inst_str_ptr, inst_str_size, "#%d",immd12);
		else
		{
			idx = (immd12 & 0x00000400) >> 10;
			add = (immd12 & 0x00000200) >> 9;
			wback = (immd12 & 0x00000100) >> 8;
			if(add)
			{
				if(idx == 1 && wback == 0)
					str_printf(inst_str_ptr, inst_str_size, "[#%d]",(immd12 & 0x000000ff));
				else if (idx == 1 && wback == 1)
					str_printf(inst_str_ptr, inst_str_size, "[#%d]!",(immd12 & 0x000000ff));
				else if (idx == 0 && wback == 1)
					str_printf(inst_str_ptr, inst_str_size, "#%d",(immd12 & 0x000000ff));
			}
			else
			{
				if(idx == 1 && wback == 0)
					str_printf(inst_str_ptr, inst_str_size, "[#-%d]",(immd12 & 0x000000ff));
				else if (idx == 1 && wback == 1)
					str_printf(inst_str_ptr, inst_str_size, "[#-%d]!",(immd12 & 0x000000ff));
				else if (idx == 0 && wback == 1)
					str_printf(inst_str_ptr, inst_str_size, "#-%d",(immd12 & 0x000000ff));

			}
		}

}

void arm_thumb32_inst_dump_IMMD12(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int immd8;
	unsigned int immd3;
	unsigned int i;
	unsigned int imm4;
	unsigned int imm5;
	unsigned int shft;
	unsigned int const_val;

		if (cat == ARM_THUMB32_CAT_DPR_IMM)
		{
			immd8 = inst->dword.data_proc_immd.immd8;
			immd3 = inst->dword.data_proc_immd.immd3;
			i = inst->dword.data_proc_immd.i_flag;
		}
		else if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
		{
			immd8 = inst->dword.data_proc_immd.immd8;
			immd3 = inst->dword.data_proc_immd.immd3;
			i = inst->dword.data_proc_immd.i_flag;
		}
		else
			fatal("%d: immd12 fmt not recognized", cat);

		imm4 = (i << 3) | (immd3);

		if(imm4 < 4)
		{
			switch(imm4)
			{
			case(0) :
				const_val =  immd8;
			break;

			case(1) :
				const_val = (immd8 << 16) | immd8;
			break;

			case(2) :
				const_val = (immd8 << 24) | (immd8 << 8);
			break;

			case(3) :
				const_val = (immd8 << 24) | (immd8 << 16) | (immd8 << 8) | immd8;
			break;

			}
		}
		else
		{
			imm5 = (imm4 << 1) | ((0x00000008 & immd8) >> 8);

			const_val = (immd8 << 24) | 0x10000000;
			shft = (imm5 - 8);

			const_val = (const_val >> shft);
		}

		str_printf(inst_str_ptr, inst_str_size, "#%d", const_val);
}

void arm_thumb32_inst_dump_IMMD8(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int immd8;

		if (cat == ARM_THUMB32_CAT_LD_ST_DOUBLE)
			immd8 = (inst->dword.ld_st_double.immd8 << 2);
		else
			fatal("%d: immd12 fmt not recognized", cat);


		if(immd8)
		{
			if(inst->dword.ld_st_double.add_sub)
			{
				if(inst->dword.ld_st_double.index == 1 && inst->dword.ld_st_double.wback == 0)
					str_printf(inst_str_ptr, inst_str_size, "#%d",(immd8));
				else if (inst->dword.ld_st_double.index == 1 && inst->dword.ld_st_double.wback == 1)
					str_printf(inst_str_ptr, inst_str_size, "#%d!",(immd8));
				else if (inst->dword.ld_st_double.index == 0 && inst->dword.ld_st_double.wback == 0)
					str_printf(inst_str_ptr, inst_str_size, "#%d",(immd8));
			}
			else
			{
				if(inst->dword.ld_st_double.index == 1 && inst->dword.ld_st_double.wback == 0)
					str_printf(inst_str_ptr, inst_str_size, "#-%d",(immd8));
				else if (inst->dword.ld_st_double.index == 1 && inst->dword.ld_st_double.wback == 1)
					str_printf(inst_str_ptr, inst_str_size, "#-%d!",(immd8));
				else if (inst->dword.ld_st_double.index == 0 && inst->dword.ld_st_double.wback == 1)
					str_printf(inst_str_ptr, inst_str_size, "#-%d",(immd8));

			}
		}

}

void arm_thumb32_inst_dump_IMM2(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int immd2;


		if (cat == ARM_THUMB32_CAT_LDSTR_BYTE)
		{
			immd2 = inst->dword.ldstr_reg.immd2;
		}
		else if (cat == ARM_THUMB32_CAT_LDSTR_REG)
		{
			immd2 = inst->dword.ldstr_reg.immd2;
		}
		else
			fatal("%d: imm2 fmt not recognized", cat);

		str_printf(inst_str_ptr, inst_str_size, "#%d", immd2);
}

void arm_thumb32_inst_dump_COND(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int cond;

	if (cat == ARM_THUMB32_CAT_BRANCH_COND)
		{
			cond = inst->dword.branch.cond;
		}
		else
			fatal("%d: cond fmt not recognized", cat);

		switch (cond)
			{
			case (EQ):
			str_printf(inst_str_ptr, inst_str_size, "eq");
			break;

			case (NE):
			str_printf(inst_str_ptr, inst_str_size, "ne");
			break;

			case (CS):
			str_printf(inst_str_ptr, inst_str_size, "cs");
			break;

			case (CC):
			str_printf(inst_str_ptr, inst_str_size, "cc");
			break;

			case (MI):
			str_printf(inst_str_ptr, inst_str_size, "mi");
			break;

			case (PL):
			str_printf(inst_str_ptr, inst_str_size, "pl");
			break;

			case (VS):
			str_printf(inst_str_ptr, inst_str_size, "vs");
			break;

			case (VC):
			str_printf(inst_str_ptr, inst_str_size, "vc");
			break;

			case (HI):
			str_printf(inst_str_ptr, inst_str_size, "hi");
			break;

			case (LS):
			str_printf(inst_str_ptr, inst_str_size, "ls");
			break;

			case (GE):
			str_printf(inst_str_ptr, inst_str_size, "ge");
			break;

			case (LT):
			str_printf(inst_str_ptr, inst_str_size, "lt");
			break;

			case (GT):
			str_printf(inst_str_ptr, inst_str_size, "gt");
			break;

			case (LE):
			str_printf(inst_str_ptr, inst_str_size, "le");
			break;

			case (AL):
			str_printf(inst_str_ptr, inst_str_size, " ");
			break;
			}

}



void arm_thumb32_inst_dump_LSB(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int immd2;
	unsigned int immd3;


		if (cat == ARM_THUMB32_CAT_BIT_FIELD)
		{
			immd2 = inst->dword.bit_field.immd2;
			immd3 = inst->dword.bit_field.immd3;
		}
		else
			fatal("%d: imm2 fmt not recognized", cat);


		str_printf(inst_str_ptr, inst_str_size, "#%d", ((immd3 << 2) | immd2));
}

void arm_thumb32_inst_dump_WID(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int msb;
	unsigned int immd2;
	unsigned int immd3;
	unsigned int lsb;



		if (cat == ARM_THUMB32_CAT_BIT_FIELD)
		{
			msb = inst->dword.bit_field.msb;
			immd2 = inst->dword.bit_field.immd2;
			immd3 = inst->dword.bit_field.immd3;
		}
		else
			fatal("%d: imm2 fmt not recognized", cat);

		lsb = (immd3 << 2) | immd2;
		str_printf(inst_str_ptr, inst_str_size, "#%d", (msb - lsb + 1));
}

void arm_thumb32_inst_dump_IMMD16(char **inst_str_ptr, int *inst_str_size,
	struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat)
{
	unsigned int immd16;
	unsigned int immd8;
	unsigned int immd3;
	unsigned int i;
	unsigned int immd4;


		if (cat == ARM_THUMB32_CAT_DPR_BIN_IMM)
		{
			immd8 = inst->dword.data_proc_immd.immd8;
			immd3 = inst->dword.data_proc_immd.immd3;
			i = inst->dword.data_proc_immd.i_flag;
			immd4 = inst->dword.data_proc_immd.rn;
		}

		else
			fatal("%d: immd16 fmt not recognized", cat);

		immd16 = (immd4 << 12) | (i << 11) | (immd3 << 8) | immd8;

		str_printf(inst_str_ptr, inst_str_size, "#%d	; 0x%x", immd16, immd16);
}

void arm_thumb32_inst_dump_ADDR(char **inst_str_ptr, int *inst_str_size,
		struct arm_thumb32_inst_t *inst, enum arm_thumb32_cat_enum cat,
		unsigned int inst_addr)
{
	unsigned int addr;

	addr = 0;
		if (cat == ARM_THUMB32_CAT_BRANCH)
		{
			addr = (inst->dword.branch_link.sign << 24)
			| ((!(inst->dword.branch.j1 ^ inst->dword.branch_link.sign)) << 23)
			| ((!(inst->dword.branch.j2 ^ inst->dword.branch_link.sign)) << 22)
			| (inst->dword.branch_link.immd10 << 12)
			| (inst->dword.branch_link.immd11 << 1);
			addr = SEXT32(addr,25);
		}
		else if (cat == ARM_THUMB32_CAT_BRANCH_LX)
		{
			addr = (inst->dword.branch_link.sign << 24)
			| ((!(inst->dword.branch.j1 ^ inst->dword.branch_link.sign)) << 23)
			| ((!(inst->dword.branch.j2 ^ inst->dword.branch_link.sign)) << 22)
			| (inst->dword.branch_link.immd10 << 12)
			| ((inst->dword.branch_link.immd11 & 0xfffffffe) << 1);
			addr = SEXT32(addr,25);
		}
		else if (cat == ARM_THUMB32_CAT_BRANCH_COND)
		{
			addr = (inst->dword.branch.sign << 20)
			| (((inst->dword.branch.j2)) << 19)
			| (((inst->dword.branch.j1)) << 18)
			| (inst->dword.branch.immd6 << 12)
			| (inst->dword.branch.immd11 << 1);
			addr = SEXT32(addr,21);
		}
		else
			fatal("%d: addr fmt not recognized", cat);

		/* FIXME : Changed from +4 to +2 */
		addr = (inst_addr + 2) + (addr);
		str_printf(inst_str_ptr, inst_str_size, "#%d	; 0x%x", addr, addr);
}


void arm_thumb32_inst_decode(struct arm_thumb32_inst_t *inst)
{
	struct arm_thumb32_inst_info_t *current_table;
	/* We initially start with the first table mips_asm_table, with the opcode field as argument */
	current_table = arm_thumb32_asm_table;
	int current_table_low = 27;
	int current_table_high = 28;
	unsigned int thumb32_table_arg;
	int loop_iteration = 0;

	thumb32_table_arg =  BITS32(*(unsigned int*)inst->dword.bytes, current_table_high, current_table_low);

	/* Find next tables if the instruction belongs to another table */
	while (1) {
		if (current_table[thumb32_table_arg].next_table && loop_iteration < 8) {
			current_table_high = current_table[thumb32_table_arg].next_table_high;
			current_table_low = current_table[thumb32_table_arg].next_table_low;
			current_table = current_table[thumb32_table_arg].next_table;
			thumb32_table_arg = BITS32(*(unsigned int*)inst->dword.bytes, current_table_high, current_table_low);
			loop_iteration++;
		}
		else if (loop_iteration > 8) {
			fatal("Can not find the correct table containing the instruction\n");
		}
		else
			break;

	}

	inst->info = &current_table[thumb32_table_arg];
}

void arm_thumb16_inst_decode(struct arm_thumb16_inst_t *inst)
{
	struct arm_thumb16_inst_info_t *current_table;
		/* We initially start with the first table mips_asm_table, with the opcode field as argument */
		current_table = arm_thumb16_asm_table;
		int current_table_low = 14;
		int current_table_high = 15;
		unsigned int thumb16_table_arg;
		int loop_iteration = 0;

		thumb16_table_arg =  BITS16(*(unsigned short*)inst->dword.bytes, current_table_high, current_table_low);

		/* Find next tables if the instruction belongs to another table */
		while (1) {
			if (current_table[thumb16_table_arg].next_table && loop_iteration < 6) {
				current_table_high = current_table[thumb16_table_arg].next_table_high;
				current_table_low = current_table[thumb16_table_arg].next_table_low;
				current_table = current_table[thumb16_table_arg].next_table;
				thumb16_table_arg = BITS16(*(unsigned short*)inst->dword.bytes, current_table_high, current_table_low);
				loop_iteration++;
			}
			else if (loop_iteration > 6) {
				fatal("Can not find the correct table containing the instruction\n");
			}
			else

				break;

		}

		inst->info = &current_table[thumb16_table_arg];
}

void arm_disasm_done()
{
	/* Thumb 16 tables */
	free(arm_thumb16_asm_table);

	free(arm_thumb16_shft_ins_table);
	free(arm_thumb16_shft_ins_lv2_table);

	free(arm_thumb16_asm_lv1_table);
	free(arm_thumb16_asm_lv2_table);
	free(arm_thumb16_asm_lv3_table);
	free(arm_thumb16_asm_lv4_table);
	free(arm_thumb16_asm_lv5_table);
	free(arm_thumb16_asm_lv6_table);
	free(arm_thumb16_asm_lv7_table);
	free(arm_thumb16_asm_lv8_table);
	free(arm_thumb16_asm_lv9_table);

	free(arm_thumb16_data_proc_table);
	free(arm_thumb16_spcl_data_brex_table);
	free(arm_thumb16_spcl_data_brex_lv1_table);
	free(arm_thumb16_ld_st_table);
	free(arm_thumb16_ld_st_lv1_table);
	free(arm_thumb16_ld_st_lv2_table);
	free(arm_thumb16_misc_table);
	free(arm_thumb16_it_table);

	/* Thumb 32 tables */
	free(arm_thumb32_asm_table);
	free(arm_thumb32_asm_lv1_table);
	free(arm_thumb32_asm_lv2_table);
	free(arm_thumb32_asm_lv3_table);
	free(arm_thumb32_asm_lv4_table);
	free(arm_thumb32_asm_lv5_table);
	free(arm_thumb32_asm_lv6_table);
	free(arm_thumb32_asm_lv7_table);
	free(arm_thumb32_asm_lv8_table);
	free(arm_thumb32_asm_lv9_table);
	free(arm_thumb32_asm_lv10_table);
	free(arm_thumb32_asm_lv11_table);
	free(arm_thumb32_asm_lv12_table);
	free(arm_thumb32_asm_lv13_table);
	free(arm_thumb32_asm_lv14_table);
	free(arm_thumb32_asm_lv15_table);


	free(arm_thumb32_asm_ldst_mul_table);
	free(arm_thumb32_asm_ldst_mul1_table);
	free(arm_thumb32_asm_ldst_mul2_table);
	free(arm_thumb32_asm_ldst_mul3_table);
	free(arm_thumb32_asm_ldst_mul4_table);
	free(arm_thumb32_asm_ldst_mul5_table);
	free(arm_thumb32_asm_ldst_mul6_table);

	free(arm_thumb32_asm_ldst_dual_table);
	free(arm_thumb32_asm_ldst1_dual_table);
	free(arm_thumb32_asm_ldst2_dual_table);
	free(arm_thumb32_asm_ldst3_dual_table);

	free(arm_thumb32_dproc_shft_reg_table);
	free(arm_thumb32_dproc_shft_reg1_table);
	free(arm_thumb32_dproc_shft_reg2_table);
	free(arm_thumb32_dproc_shft_reg3_table);
	free(arm_thumb32_dproc_shft_reg4_table);
	free(arm_thumb32_dproc_shft_reg5_table);
	free(arm_thumb32_dproc_shft_reg6_table);

	free(arm_thumb32_dproc_imm_table);
	free(arm_thumb32_dproc_imm1_table);
	free(arm_thumb32_dproc_imm2_table);
	free(arm_thumb32_dproc_imm3_table);
	free(arm_thumb32_dproc_imm4_table);
	free(arm_thumb32_dproc_imm5_table);
	free(arm_thumb32_dproc_imm6_table);

	free(arm_thumb32_dproc_reg_table);
	free(arm_thumb32_dproc_reg1_table);
	free(arm_thumb32_dproc_reg2_table);
	free(arm_thumb32_dproc_reg3_table);
	free(arm_thumb32_dproc_reg4_table);
	free(arm_thumb32_dproc_reg5_table);
	free(arm_thumb32_dproc_reg6_table);
	free(arm_thumb32_dproc_reg7_table);

	free(arm_thumb32_dproc_misc_table);
	free(arm_thumb32_dproc_misc1_table);

	free(arm_thumb32_st_single_table);
	free(arm_thumb32_st_single1_table);
	free(arm_thumb32_st_single2_table);
	free(arm_thumb32_st_single3_table);
	free(arm_thumb32_st_single4_table);
	free(arm_thumb32_st_single5_table);
	free(arm_thumb32_st_single6_table);

	free(arm_thumb32_ld_byte_table);
	free(arm_thumb32_ld_byte1_table);
	free(arm_thumb32_ld_byte2_table);
	free(arm_thumb32_ld_byte3_table);
	free(arm_thumb32_ld_byte4_table);
	free(arm_thumb32_ld_byte5_table);
	free(arm_thumb32_ld_byte6_table);

	free(arm_thumb32_ld_hfword_table);
	free(arm_thumb32_ld_hfword1_table);
	free(arm_thumb32_ld_hfword2_table);

	free(arm_thumb32_ld_word_table);
	free(arm_thumb32_ld_word1_table);

	free(arm_thumb32_mult_table);
	free(arm_thumb32_mult1_table);

	free(arm_thumb32_dproc_bin_imm_table);
	free(arm_thumb32_dproc_bin_imm1_table);
	free(arm_thumb32_dproc_bin_imm2_table);
	free(arm_thumb32_dproc_bin_imm3_table);

	free(arm_thumb32_mult_long_table);
	free(arm_thumb32_mov_table);
	free(arm_thumb32_mov1_table);

	free(arm_thumb32_brnch_ctrl_table);
	free(arm_thumb32_brnch_ctrl1_table);
}

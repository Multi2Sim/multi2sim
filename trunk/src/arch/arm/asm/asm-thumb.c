/*
 * asm-thumb.c
 *
 *  Created on: Jan 30, 2013
 *      Author: ukidaveyash
 */

#include <ctype.h>
#include <string.h>

#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/mhandle/mhandle.h>

#include "asm.h"
#include "asm-thumb.h"

/* Pointers to the tables of instructions */
struct arm_thumb16_inst_info_t *arm_thumb16_asm_table;
struct arm_thumb16_inst_info_t *arm_thumb16_shft_ins_table;
struct arm_thumb16_inst_info_t *arm_thumb16_shft_ins_lv2_table;

struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv1_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv2_table;
struct arm_thumb16_inst_info_t *arm_thumb16_asm_lv3_table;

struct arm_thumb16_inst_info_t *arm_thumb16_data_proc_table;
struct arm_thumb16_inst_info_t *arm_thumb16_spcl_data_brex_table;
struct arm_thumb16_inst_info_t *arm_thumb16_ld_st_table;
struct arm_thumb16_inst_info_t *arm_thumb16_misc_table;

/* Hard-coded instructions */


void arm_thumb16_disasm_init()
{
	int i;

	arm_thumb16_asm_table 			= xcalloc(8, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_shft_ins_table		= xcalloc(16, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_shft_ins_lv2_table		= xcalloc(16, sizeof(struct arm_thumb16_inst_info_t));

	arm_thumb16_asm_lv1_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv2_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_asm_lv3_table		= xcalloc(4, sizeof(struct arm_thumb16_inst_info_t));

	arm_thumb16_data_proc_table		= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_spcl_data_brex_table	= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_ld_st_table			= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));
	arm_thumb16_misc_table			= xcalloc(64, sizeof(struct arm_thumb16_inst_info_t));




	arm_thumb16_asm_table[0].next_table 	= arm_thumb16_shft_ins_table;
	arm_thumb16_asm_table[0].next_table_high= 13;
	arm_thumb16_asm_table[0].next_table_low	= 11;

	arm_thumb16_shft_ins_table[3].next_table = arm_thumb16_shft_ins_lv2_table;
	arm_thumb16_shft_ins_table[3].next_table_high = 10;
	arm_thumb16_shft_ins_table[3].next_table_low = 9;

	arm_thumb16_asm_table[1].next_table 	= arm_thumb16_asm_lv1_table;
	arm_thumb16_asm_table[1].next_table_high= 13;
	arm_thumb16_asm_table[1].next_table_low	= 13;

	arm_thumb16_asm_lv1_table[0].next_table 	= arm_thumb16_asm_lv2_table;
	arm_thumb16_asm_lv1_table[0].next_table_high	= 12;
	arm_thumb16_asm_lv1_table[0].next_table_low	= 12;

	arm_thumb16_asm_lv1_table[1].next_table 	= arm_thumb16_ld_st_table;
	arm_thumb16_asm_lv1_table[1].next_table_high	= 12;//TBC
	arm_thumb16_asm_lv1_table[1].next_table_low	= 12;//TBC

	arm_thumb16_asm_lv2_table[0].next_table 	= arm_thumb16_asm_lv3_table;
	arm_thumb16_asm_lv2_table[0].next_table_high	= 11;
	arm_thumb16_asm_lv2_table[0].next_table_low	= 10;

	arm_thumb16_asm_lv2_table[1].next_table 	= arm_thumb16_ld_st_table;
	arm_thumb16_asm_lv2_table[1].next_table_high	= 11;
	arm_thumb16_asm_lv2_table[1].next_table_low	= 10;

	arm_thumb16_asm_lv3_table[0].next_table 	= arm_thumb16_data_proc_table;
	arm_thumb16_asm_lv3_table[0].next_table_high	= 11;	// TBC
	arm_thumb16_asm_lv3_table[0].next_table_low	= 10;	// TBC

	arm_thumb16_asm_lv3_table[1].next_table 	= arm_thumb16_spcl_data_brex_table;
	arm_thumb16_asm_lv3_table[1].next_table_high	= 11;	// TBC
	arm_thumb16_asm_lv3_table[1].next_table_low	= 10;	// TBC




}

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

#include <misc.h>
#include <assert.h>
#include <string.h>
#include <debug.h>
#include <ctype.h>
#include <elf-format.h>
#include <mhandle.h>

#include "southern-islands-asm.h"



/* Table containing information of all instructions */
static struct si_inst_info_t si_inst_info[SI_INST_COUNT];

#define SI_INST_INFO_SMRD_OPCODE_SIZE 32
#define SI_INST_INFO_SOPP_OPCODE_SIZE 22

/* String lengths for printing assembly */
#define MAX_INST_STR_SIZE 60
#define MAX_SDST_STR_SIZE 10

/* Pointers to 'si_inst_info' table indexed by instruction opcode */
static struct si_inst_info_t *si_inst_info_smrd[SI_INST_INFO_SMRD_OPCODE_SIZE];
static struct si_inst_info_t *si_inst_info_sopp[SI_INST_INFO_SOPP_OPCODE_SIZE];

/* Helper functions to dump assembly code to a file */
void si_inst_dump_sopp(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f);
void si_inst_dump_smrd(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f);




/*
 * Initialization/finalization of disassembler
 */

void si_disasm_init()
{
	struct si_inst_info_t *info;
	int i;

	/* Type size assertions */
	assert(sizeof(union si_reg_t) == 4);

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size) \
	info = &si_inst_info[SI_INST_##_name]; \
	info->inst = SI_INST_##_name; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->fmt = SI_FMT_##_fmt; \
	info->opcode = _opcode; \
	info->size = _size;
#include "southern-islands-asm.dat"
#undef DEFINST
	
	/* Tables of pointers to 'si_inst_info' */
	for (i = 1; i < SI_INST_COUNT; i++)
	{
		info = &si_inst_info[i];

		if (info->fmt == SI_FMT_SMRD) 
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_SMRD_OPCODE_SIZE - 1));
			si_inst_info_smrd[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOPP)
		{
			assert(IN_RANGE(info->opcode, 0, SI_INST_INFO_SOPP_OPCODE_SIZE - 1));
			si_inst_info_sopp[info->opcode] = info;
			continue;
		}
		/* TODO FILL IN REMAINING FORMATS */
		else 
		{

		}

		fprintf(stderr, "warning: '%s' not indexed\n", info->name);
	}
}


void si_disasm_done()
{

}



/*
 * Functions for decoding instructions
 */

int si_inst_decode(void *buf, struct si_inst_t *inst) 
{
	int inst_size = 4; /* Bytes */

	/* Zero-out instruction structure */
	memset(inst, 0, sizeof(struct si_inst_t));

	/* All instructions will be at least 32-bits */
	memcpy(&inst->micro_inst, buf, 4);

	/* Use the encoding field to determine the instruction type */
	if (inst->micro_inst.sopp.enc == 0x17f)
	{
		assert(si_inst_info_sopp[inst->micro_inst.sopp.op]);
		inst->info = si_inst_info_sopp[inst->micro_inst.sopp.op];
	}
	else if (inst->micro_inst.smrd.enc == 0x18) 
	{
		/* SDST 125 is reserved */
		assert(inst->micro_inst.smrd.sdst != 125);

		assert(si_inst_info_smrd[inst->micro_inst.smrd.op]);
		inst->info = si_inst_info_smrd[inst->micro_inst.smrd.op];
	}
	/* TODO FILL IN REMAINING FORMATS */
	else 
	{
		printf("Unimplemented format. Instruction is: %08X\n", ((unsigned int*)buf)[0]);
		exit(0);
	}

	return inst_size;
}

void si_disasm_buffer(struct elf_buffer_t *buffer, FILE *f)
{
	void *inst_buf = buffer->ptr;
	int inst_count = 0;
	int rel_addr = 0;

	/* Disassemble */
	while (inst_buf)
	{
		struct si_inst_t inst;
		int inst_size;

		/* Parse the instruction */
	       	inst_size = si_inst_decode(inst_buf, &inst);

		inst_count++;
		rel_addr += inst_size;
		
		/* Dump the instruction */
		if (inst.info->fmt == SI_FMT_SOPP)
		{
			si_inst_dump_sopp(&inst, rel_addr, inst_buf, f);
		}
		else if (inst.info->fmt == SI_FMT_SMRD) 
		{
			si_inst_dump_smrd(&inst, rel_addr, inst_buf, f);
		}
		/* TODO FILL IN REMAINING FORMATS */
		else 
		{
			printf("Unknown instruction: %08X\n", ((unsigned int*)inst_buf)[0]);
			exit(0);
		}

		/* Increment instruction pointer */
		inst_buf += inst_size;
	}

}




/* 
 * Functions to print assembly output to file
 */

void si_inst_dump_sopp(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f)
{
	/* TODO Implement */
	assert("si_inst_dump_sopp not implemented yet");
}

void si_inst_dump_smrd(struct si_inst_t* inst, unsigned int rel_addr, void* buf, FILE *f)
{
	struct si_fmt_smrd_t *smrd = &inst->micro_inst.smrd;

	/* The sbase field is missing the LSB, so multiply by 2 */
	int sdst = smrd->sdst;
	int sdst_end;
	int sbase = smrd->sbase*2;
	int sbase_end;

	int multi_dword = 0;

	/* TODO Handle the case where 'imm' is 0 */
	assert(smrd->imm); 

	/* S_LOAD_DWORD */
	if (IN_RANGE(smrd->op, 0, 4))
	{
		/* SBASE specifies two consecutive SGPRs */
		sbase_end = sbase + 1;
		
		if (smrd->op != 0) 
		{
			/* Multi-dword */
			multi_dword = 1;
			switch (smrd->op)
			{
			case 1:
				sdst_end = sdst+1;
				break;
			case 2:
				sdst_end = sdst+3;
				break;
			case 3:
				sdst_end = sdst+7;
				break;
			case 4:
				sdst_end = sdst+15;
				break;
			default:
				assert("Invalid smrd opcode");
			}
		}
	}
	/* S_BUFFER_LOAD_DWORD */
	else if (IN_RANGE(smrd->op, 8, 12))
	{
		/* SBASE specifies four consecutive SGPRs */
		sbase_end = sbase + 3;

		if (smrd->op != 8) 
		{
			/* Multi-dword */
			multi_dword = 1;
			switch (smrd->op)
			{
			case 1:
				sdst_end = sdst+1;
				break;
			case 2:
				sdst_end = sdst+3;
				break;
			case 3:
				sdst_end = sdst+7;
				break;
			case 4:
				sdst_end = sdst+15;
				break;
			default:
				assert("Invalid smrd opcode");
			}
		}
	}
	/* S_MEMTIME */
	else if(smrd->op == 30) 
	{

	}
	/* S_DCACHE_INV */
	else if(smrd->op == 31) 
	{

	}

	/* Write the assembly to file */
	/* TODO Get rid of magic numbers */
	char inst_str[MAX_INST_STR_SIZE];
	char sdst_str[MAX_SDST_STR_SIZE];

	if (multi_dword)
	{
		snprintf(sdst_str, MAX_SDST_STR_SIZE, "s[%d:%d]", sdst, sdst_end);
	}
	else
	{
		snprintf(sdst_str, MAX_SDST_STR_SIZE, "s%d", sdst);
	}
	snprintf(inst_str, MAX_INST_STR_SIZE, "  %s  %s, s[%d:%d], 0x%02x", 
		inst->info->fmt_str, sdst_str, sbase, sbase_end, smrd->offset);
	fprintf(f, "%-60s// %08X: %08X\n", inst_str, rel_addr, ((unsigned int*)buf)[0]);
}


/* GPU disassembler tool */
void si_disasm(char *path)
{
        struct elf_file_t *elf_file;
        struct elf_symbol_t *symbol;
        struct elf_section_t *section;

        struct si_bin_file_t *amd_bin;

        char kernel_name[MAX_STRING_SIZE];

        int i;

        /* Initialize disassembler */
        si_disasm_init();

        /* Decode external ELF */
        elf_file = elf_file_create_from_path(path);
        for (i = 0; i < list_count(elf_file->symbol_table); i++)
        {
                /* Get symbol and section */
                symbol = list_get(elf_file->symbol_table, i);
                section = list_get(elf_file->section_list, symbol->section);
                if (!section)
                        continue;

                /* If symbol is '__OpenCL_XXX_kernel', it points to internal ELF */
                if (str_prefix(symbol->name, "__OpenCL_") && str_suffix(symbol->name, "_kernel"))
                {
                        /* Decode internal ELF */
                        str_substr(kernel_name, sizeof(kernel_name), symbol->name, 9, strlen(symbol->name) - 16);
                        amd_bin = si_bin_file_create(section->buffer.ptr + symbol->value, symbol->size, kernel_name);

                        /* Get kernel name */
                        printf("**\n** Disassembly for '__kernel %s'\n**\n\n", kernel_name);
                        si_disasm_buffer(&amd_bin->enc_dict_entry_southern_islands->sec_text_buffer, stdout);
                        printf("\n\n\n");

                        /* Free internal ELF */
                        si_bin_file_free(amd_bin);
                }
        }

        /* Free external ELF */
        elf_file_free(elf_file);
        si_disasm_done();

        /* End */
        mhandle_done();
        exit(0);
}


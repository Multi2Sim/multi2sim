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


#include <ctype.h>

#include <lib/class/array.h>
#include <lib/class/elf-reader.h>
#include <lib/class/string.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/elf-format.h>
#include <lib/util/elf-encode.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"




/*
 * Class 'FrmAsm'
 */

void FrmAsmCreate(FrmAsm *self)
{
	FrmInstInfo *info;

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _category, _op) \
	info = &self->inst_info[FRM_INST_##_name]; \
	info->opcode = FRM_INST_##_name; \
	info->category = FrmInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->op = _op; \
	info->size = 8; \
	self->dec_table[_op] = info;
#include "asm.dat"
#undef DEFINST

}


void FrmAsmDestroy(FrmAsm *self)
{
}


void FrmAsmDisassembleBinary(FrmAsm *self, char *path)
{
	ELFReader *reader;
	ELFSection *section;

	FrmInst *inst;

	int inst_index;


	/* Initialization */
	reader = new(ELFReader, path);
	inst = new(FrmInst, self);

	/* Dump disassembly */
	printf("\n\tcode for sm_20\n");
	ArrayForEach(reader->section_array, section, ELFSection)
	{
		/* Determine if section is .text.kernel_name */
		if (!strncmp(section->name->text, ".text.", 6))
		{
			/* Decode and dump instructions */
			printf("\t\tFunction : %s\n", section->name->text + 6);
			for (inst_index = 0; inst_index < section->buffer->size / 8; ++inst_index)
			{
				FrmInstDecode(inst, inst_index * 8, section->buffer->ptr + inst_index * 8);
				FrmInstDumpHex(inst, stdout);
				FrmInstDump(inst, stdout);
				printf(";\n");
			}
			printf("\t\t......................................\n\n\n");
		}

		if (!strncmp(section->name->text, ".rodata", 7))
		{
			FILE *fp = fopen(".rodata", "wb");
			fwrite(section->buffer->ptr, 1, section->buffer->size, fp);
			fclose(fp);
		}
	}

	/* Free external ELF */
	delete(reader);
	delete(inst);
}


void FrmAsmDisassembleBuffer(FrmAsm *self, void *ptr, int size)
{
	FrmInst *inst;

	int inst_index;

	inst = new(FrmInst, self);
	for (inst_index = 0; inst_index < size / 8; ++inst_index)
	{
		FrmInstDecode(inst, inst_index * 8, ptr + inst_index * 8);
		FrmInstDumpHex(inst, stdout);
		FrmInstDump(inst, stdout);
		printf(";\n");
	}
	delete(inst);
}

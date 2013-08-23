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

#include <assert.h>
#include <string.h>
#include <ctype.h>

#include <lib/class/array.h>
#include <lib/class/elf-reader.h>
#include <lib/class/class.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "alu-group.h"
#include "asm.h"
#include "bin-file.h"
#include "inst.h"
#include "opengl-bin-file.h"


/*
 * Class 'EvgAsm'
 */

void EvgAsmCreate(EvgAsm *self)
{
	EvgInstInfo *info;
	int i;

	/* Type size assertions */
	assert(sizeof(EvgInstReg) == 4);

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _fmt0, _fmt1, _fmt2, _category, _op, _flags) \
	info = &self->inst_info[EVG_INST_##_name]; \
	info->opcode = EVG_INST_##_name; \
	info->category = EvgInstCategory##_category; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->fmt[0] = EvgInstFormat##_fmt0; \
	info->fmt[1] = EvgInstFormat##_fmt1; \
	info->fmt[2] = EvgInstFormat##_fmt2; \
	info->op = _op; \
	info->flags = _flags; \
	info->size = (EvgInstFormat##_fmt0 ? 1 : 0) + \
			(EvgInstFormat##_fmt1 ? 1 : 0) + \
			(EvgInstFormat##_fmt2 ? 1 : 0);
#include "asm.dat"
#undef DEFINST

	/* Tables of pointers to 'inst_info' */
	for (i = 1; i < EvgInstOpcodeCount; i++)
	{
		info = &self->inst_info[i];
		if (info->fmt[1] == EvgInstFormatCfWord1 ||
			info->fmt[1] == EvgInstFormatCfAllocExportWord1Buf ||
			info->fmt[1] == EvgInstFormatCfAllocExportWord1Swiz)
		{
			assert(IN_RANGE(info->op, 0, EVG_INST_INFO_CF_LONG_SIZE - 1));
			self->inst_info_cf_long[info->op] = info;
			continue;
		}
		if (info->fmt[1] == EvgInstFormatCfAluWord1 ||
			info->fmt[1] == EvgInstFormatCfAluWord1Ext)
		{
			assert(IN_RANGE(info->op, 0, EVG_INST_INFO_CF_SHORT_SIZE - 1));
			self->inst_info_cf_short[info->op] = info;
			continue;
		}
		if (info->fmt[1] == EvgInstFormatAluWord1Op2) {
			assert(IN_RANGE(info->op, 0, EVG_INST_INFO_ALU_LONG_SIZE - 1));
			self->inst_info_alu_long[info->op] = info;
			continue;
		}
		if (info->fmt[1] == EvgInstFormatAluWord1Op3 ||
			info->fmt[1] == EvgInstFormatAluWord1LdsIdxOp)
		{
			assert(IN_RANGE(info->op, 0, EVG_INST_INFO_ALU_SHORT_SIZE - 1));
			self->inst_info_alu_short[info->op] = info;
			continue;
		}
		if (info->fmt[0] == EvgInstFormatTexWord0 || info->fmt[0] == EvgInstFormatVtxWord0 || info->fmt[0] == EvgInstFormatMemRdWord0) {
			assert(IN_RANGE(info->op, 0, EVG_INST_INFO_TEX_SIZE - 1));
			self->inst_info_tex[info->op] = info;
			continue;
		}
		fprintf(stderr, "warning: '%s' not indexed\n", info->name);
	}
}


void EvgAsmDestroy(EvgAsm *self)
{
}


/* Disassemble entire ELF buffer containing Evergreen ISA.
 * The ELF buffer representing an entire kernel '.text' section can be used here. */
static void EvgAsmDisassembleBuffer(EvgAsm *self, struct elf_buffer_t *buffer, FILE *f)
{
	void *cf_buf;
	int inst_count;
	int cf_inst_count;
	int sec_inst_count;
	int loop_idx;

	EvgInst *inst;
	EvgInst *cf_inst;
	EvgALUGroup *alu_group;

	/* Initialize */
	cf_buf = buffer->ptr;
	inst_count = 0;
	cf_inst_count = 0;
	sec_inst_count = 0;
	loop_idx = 0;

	/* Instructions */
	inst = new(EvgInst, self);
	cf_inst = new(EvgInst, self);
	alu_group = new(EvgALUGroup, self);

	/* Disassemble */
	while (cf_buf)
	{
		/* CF Instruction */
		cf_buf = EvgInstDecodeCF(cf_inst, cf_buf);
                if (cf_inst->info->flags & EvgInstFlagDecLoopIdx)
                {
                        assert(loop_idx > 0);
                        loop_idx--;
                }

		EvgInstDump(cf_inst, cf_inst_count, loop_idx, f);
		cf_inst_count++;
		inst_count++;

		/* ALU Clause */
		if (cf_inst->info->fmt[0] == EvgInstFormatCfAluWord0)
		{
			void *alu_buf;
			void *alu_buf_end;

			alu_buf = buffer->ptr + cf_inst->words[0].cf_alu_word0.addr * 8;
			alu_buf_end = alu_buf + (cf_inst->words[1].cf_alu_word1.count + 1) * 8;
			while (alu_buf < alu_buf_end)
			{
				alu_buf = EvgALUGroupDecode(alu_group, alu_buf, sec_inst_count);
				EvgALUGroupDump(alu_group, loop_idx, f);
				sec_inst_count++;
				inst_count++;
			}
		}

		/* TEX Clause */
		if (cf_inst->info->opcode == EVG_INST_TC)
		{
			char *tex_buf;
			char *tex_buf_end;

			tex_buf = buffer->ptr + cf_inst->words[0].cf_word0.addr * 8;
			tex_buf_end = tex_buf + (cf_inst->words[1].cf_word1.count + 1) * 16;
			while (tex_buf < tex_buf_end)
			{
				tex_buf = EvgInstDecodeTC(inst, tex_buf);
				EvgInstDump(inst, sec_inst_count, loop_idx, f);
				sec_inst_count++;
				inst_count++;
			}
		}

		/* Increase loop depth counter */
                if (cf_inst->info->flags & EvgInstFlagIncLoopIdx)
                        loop_idx++;
	}

	/* Free instructions */
	delete(inst);
	delete(cf_inst);
	delete(alu_group);
}


void EvgAsmDisassembleBinary(EvgAsm *self, char *path)
{
	ELFReader *reader;
	ELFSymbol *symbol;
	ELFSection *section;

	struct evg_bin_file_t *amd_bin;

	char kernel_name[MAX_STRING_SIZE];

	/* Decode external ELF */
	reader = new(ELFReader, path);
	ArrayForEach(reader->symbol_array, symbol, ELFSymbol)
	{
		/* Get symbol and section */
		section = asELFSection(ArrayGet(reader->section_array, symbol->sym->st_shndx));
		if (!section)
			continue;

		/* If symbol is '__OpenCL_XXX_kernel', it points to internal ELF */
		if (str_prefix(symbol->name, "__OpenCL_") && str_suffix(symbol->name, "_kernel"))
		{
			/* Decode internal ELF */
			str_substr(kernel_name, sizeof(kernel_name), symbol->name, 9, strlen(symbol->name) - 16);
			amd_bin = evg_bin_file_create(section->buffer->ptr +
					symbol->sym->st_value, symbol->sym->st_size, kernel_name);

			/* Get kernel name */
			printf("**\n** Disassembly for '__kernel %s'\n**\n\n", kernel_name);
			EvgAsmDisassembleBuffer(self, &amd_bin->enc_dict_entry_evergreen->sec_text_buffer, stdout);
			printf("\n\n\n");

			/* Free internal ELF */
			evg_bin_file_free(amd_bin);
		}
	}

	/* Free external ELF */
	delete(reader);
}


void EvgAsmDisassembleOpenGLBinary(EvgAsm *self, char *path, int index)
{
	void *file_buffer;
	int file_size;

	struct evg_opengl_bin_file_t *amd_opengl_bin;
	struct evg_opengl_shader_t *amd_opengl_shader;

	/* Load file into memory buffer */
	file_buffer = read_buffer(path, &file_size);
	if(!file_buffer)
		fatal("%s:Invalid file!", path);

	/* Analyze the file and initialize structure */
	amd_opengl_bin = evg_opengl_bin_file_create(file_buffer, file_size, path);

	free_buffer(file_buffer);

	/* Basic info of the shader binary */
	printf("This shader binary contains %d shaders\n\n", list_count(amd_opengl_bin->shader_list));
	if (index > list_count(amd_opengl_bin->shader_list) || index <= 0 )
	{
		fatal("Shader index out of range! Please choose <index> from 1 ~ %d", list_count(amd_opengl_bin->shader_list));
	}

	/* Disaseemble */
	amd_opengl_shader = list_get(amd_opengl_bin->shader_list, index -1 );
	printf("**\n** Disassembly for shader %d\n**\n\n", index);
	EvgAsmDisassembleBuffer(self, &amd_opengl_shader->isa_buffer, stdout);
	printf("\n\n\n");

	/* Free */
	evg_opengl_bin_file_free(amd_opengl_bin);
}

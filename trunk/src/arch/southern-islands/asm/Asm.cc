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

#include <cassert>
#include <iomanip>
#include <sstream>

#include <lib/cpp/Misc.h>

#include "Asm.h"
#include "Inst.h"


using namespace Misc;
using namespace SI;
using namespace std;


Asm::Asm()
{
	InstInfo *info;
	int i;

	/* Type size assertions */
	assert(sizeof(InstReg) == 4);

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _fmt, _op, _size, _flags) \
	info = &inst_info[INST_##_name]; \
	info->opcode = INST_##_name; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->fmt = InstFormat##_fmt; \
	info->op = _op; \
	info->size = _size; \
	info->flags = (InstFlag) _flags;
#include "asm.dat"
#undef DEFINST

	/* Tables of pointers to 'inst_info' */
	for (i = 1; i < InstOpcodeCount; i++)
	{
		info = &inst_info[i];

		if (info->fmt == InstFormatSOPP)
		{
			assert(InRange(info->op, 0, dec_table_sopp_count - 1));
			dec_table_sopp[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatSOPC)
		{
			assert(InRange(info->op, 0, dec_table_sopc_count - 1));
			dec_table_sopc[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatSOP1)
		{
			assert(InRange(info->op, 0, dec_table_sop1_count - 1));
			dec_table_sop1[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatSOPK)
		{
			assert(InRange(info->op, 0, dec_table_sopk_count - 1));
			dec_table_sopk[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatSOP2)
		{
			assert(InRange(info->op, 0, dec_table_sop2_count - 1));
			dec_table_sop2[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatSMRD) 
		{
			assert(InRange(info->op, 0, dec_table_smrd_count - 1));
			dec_table_smrd[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatVOP3a || info->fmt == InstFormatVOP3b)
		{
			int i;

			assert(InRange(info->op, 0, dec_table_vop3_count - 1));
			dec_table_vop3[info->op] = info;
			if (info->flags & InstFlagOp8)
			{
				for (i = 1; i < 8; i++)
				{
					dec_table_vop3[info->op + i] = 
						info;
				}
			}
			if (info->flags & InstFlagOp16)
			{
				for (i = 1; i < 16; i++)
				{
					dec_table_vop3[info->op + i] = 
						info;
				}
			}
			continue;
		}
		else if (info->fmt == InstFormatVOPC)
		{
			assert(InRange(info->op, 0, dec_table_vopc_count - 1));
			dec_table_vopc[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatVOP1)
		{
			assert(InRange(info->op, 0, dec_table_vop1_count - 1));
			dec_table_vop1[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatVOP2)
		{
			assert(InRange(info->op, 0, dec_table_vop2_count - 1));
			dec_table_vop2[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatVINTRP)
		{
			assert(InRange(info->op, 0, dec_table_vintrp_count - 1));
			dec_table_vintrp[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatDS)
		{
			assert(InRange(info->op, 0, dec_table_ds_count - 1));
			dec_table_ds[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatMTBUF)
		{
			assert(InRange(info->op, 0, dec_table_mtbuf_count - 1));
			dec_table_mtbuf[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatMUBUF)
		{
			assert(InRange(info->op, 0, dec_table_mubuf_count - 1));
			dec_table_mubuf[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatMIMG)
		{
			assert(InRange(info->op, 0, dec_table_mimg_count - 1));
			dec_table_mimg[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatEXP)
		{
			assert(InRange(info->op, 0, dec_table_exp_count - 1));
			dec_table_exp[info->op] = info;
			continue;
		}
		else 
		{
			cerr << "warning: '" << info->name
					<< "' not indexed\n";
		}
	}
}


void Asm::DisassembleBuffer(ostream& os, char *buffer, int size)
{
	stringstream ss;

	char *original_buffer = buffer;

	int inst_count = 0;
	int rel_addr = 0;

	int label_addr[size / 4];	/* A list of created labels sorted by rel_addr. */

	int *next_label = &label_addr[0];	/* The next label to dump. */
	int *end_label = &label_addr[0];	/* The address after the last label. */

	InstFormat format;
	InstBytes *bytes;

	Inst inst(this);

	/* Read through instructions to find labels. */
	while (buffer < original_buffer + size)
	{
		/* Decode instruction */
		inst.Decode(buffer, rel_addr);
		format = inst.GetFormat();
		bytes = inst.GetBytes();

		/* If ENDPGM, break. */
		if (format == InstFormatSOPP && bytes->sopp.op == 1)
			break;

		/* If the instruction branches, insert the label into 
		 * the sorted list. */
		if (format == InstFormatSOPP &&
			(bytes->sopp.op >= 2 && 
			 bytes->sopp.op <= 9))
		{
			short simm16 = bytes->sopp.simm16;
			int se_simm = simm16;
			int label = rel_addr + (se_simm * 4) + 4;

			/* Find position to insert label. */
			int *t_label = &label_addr[0];

			while (t_label < end_label && *t_label < label)
				t_label++;

			if (label != *t_label || t_label == end_label)
			{

				/* Shift labels after position down. */
				int *t2_label = end_label;

				while (t2_label > t_label)
				{
					*t2_label = *(t2_label - 1);
					t2_label--;
				}
				end_label++;

				/* Insert the new label. */
				*t_label = label;
			}

		}

		buffer += inst.GetSize();
		rel_addr += inst.GetSize();
	}


	/* Reset to disassemble. */
	buffer = original_buffer;
	rel_addr = 0;

	/* Disassemble */
	while (buffer < original_buffer + size)
	{
		/* Parse the instruction */
		inst.Decode(buffer, rel_addr);
		format = inst.GetFormat();
		bytes = inst.GetBytes();
		inst_count++;

		/* Dump a label if necessary. */
		if (*next_label == rel_addr && next_label != end_label)
		{
			os << "label_" << setw(4) << setfill('0') << uppercase
					<< hex << rel_addr / 4 << ":\n"
					<< setfill(' ') << nouppercase << dec;
			next_label++;
		}

		/* Dump the instruction */
		ss.str("");
		ss << ' ';
		inst.Dump(ss);

		/* Spaces */
		if (ss.str().length() < 59)
			ss << string(59 - ss.str().length(), ' ');

		/* Hex dump */
		os << ss.str();
		os << " // " << setw(8) << setfill('0') << hex
				<< uppercase << rel_addr << ": "
				<< setfill(' ') << nouppercase << dec;
		os << setw(8) << setfill('0') << hex
				<< uppercase << bytes->word[0]
				<< setfill(' ') << nouppercase << dec;
		if (inst.GetSize() == 8)
			os << ' ' << setw(8) << setfill('0') << hex
					<< uppercase << bytes->word[1]
					<< setfill(' ') << nouppercase << dec;
		os << '\n';

		/* Break at end of program. */
		if (format == InstFormatSOPP && bytes->sopp.op == 1)
			break;

		/* Increment instruction pointer */
		buffer += inst.GetSize();
		rel_addr += inst.GetSize();
	}
}

//////////////
extern "C" {
#include <string.h>
#include <lib/util/string.h>
#include <lib/util/elf-format.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include "opengl-bin-file.h"
#include "bin-file.h"
}

void Asm::DisassembleBinary(std::string path)
{
	struct elf_file_t *elf_file;
	struct elf_symbol_t *symbol;
	struct elf_section_t *section;
	struct SIBinary *amd_bin;

	char kernel_name[200];

	char *buffer;

	int size;
	int i;

	/* Decode external ELF */
	elf_file = elf_file_create_from_path(const_cast<char *>(path.c_str()));
	for (i = 0; i < list_count(elf_file->symbol_table); i++)
	{
		/* Get symbol and section */
		symbol = (struct elf_symbol_t *) list_get(elf_file->symbol_table, i);
		section = (struct elf_section_t *) list_get(elf_file->section_list, symbol->section);
		if (!section)
			continue;

		/* If symbol is '__OpenCL_XXX_kernel', it points 
		 * to internal ELF */
		if (str_prefix(symbol->name, (char *) "__OpenCL_") && 
			str_suffix(symbol->name, (char *) "_kernel"))
		{
			/* Decode internal ELF */
			str_substr(kernel_name, sizeof(kernel_name), 
				symbol->name, 9, strlen(symbol->name) - 16);
			amd_bin = si_binary_create(
				(char *) section->buffer.ptr + symbol->value, 
				symbol->size, kernel_name);

			/* Get kernel name */
			printf("**\n** Disassembly for '__kernel %s'\n**\n\n",
					kernel_name);
			buffer = (char *) amd_bin->enc_dict_entry_southern_islands->sec_text_buffer.ptr;
			size = amd_bin->enc_dict_entry_southern_islands->sec_text_buffer.size;
			DisassembleBuffer(cout, buffer, size);
			printf("\n\n\n");

			/* Free internal ELF */
			si_binary_free(amd_bin);
		}
	}

	/* Free external ELF */
	elf_file_free(elf_file);
}


void Asm::DisassembleOpenGLBinary(string path, int shader_index)
{
	struct opengl_si_program_binary_t *program_bin;
	struct opengl_si_shader_binary_t *shader;
	void *file_buffer;
	int file_size;

	/* Load file into memory buffer */
	file_buffer = read_buffer(const_cast<char *>(path.c_str()), &file_size);
	if(!file_buffer)
		fatal("%s:Invalid file!", const_cast<char *>(path.c_str()));

	/* Analyze the file and initialize structure */	
	program_bin = opengl_si_program_binary_create(file_buffer, file_size, const_cast<char *>(path.c_str()));
	free_buffer(file_buffer);

	/* Basic info of the shader binary */
	printf("This program binary contains %d shaders\n\n", 
		list_count(program_bin->shader_bins));
	if (shader_index > list_count(program_bin->shader_bins) ||
			shader_index <= 0 )
	{
		fatal("Shader index out of range! Please choose <index> "
			"from 1 ~ %d", list_count(program_bin->shader_bins));
	}

	/* Disassemble */
	shader = (struct opengl_si_shader_binary_t *)list_get(program_bin->shader_bins, 
			shader_index - 1);
	printf("**\n** Disassembly for shader %d\n**\n\n", shader_index);
	DisassembleBuffer(cout, (char *)shader->isa->ptr,
			shader->isa->size);
	opengl_si_shader_binary_debug_meta(shader);
	printf("\n\n\n");

	/* Free */
	opengl_si_program_binary_free(program_bin);
}




/*
 * C Wrapper
 */

struct SIAsmWrap *SIAsmWrapCreate()
{
	Asm *as = new Asm();
	return (SIAsmWrap *) as;
}


void SIAsmWrapFree(struct SIAsmWrap *self)
{
	Asm *as = (Asm *) self;
	delete as;
}


void SIAsmWrapDisassembleBinary(struct SIAsmWrap *self, char *path)
{
	Asm *as = (Asm *) self;
	as->DisassembleBinary(path);
}


void SIAsmWrapDisassembleOpenGLBinary(struct SIAsmWrap *self, char *path,
		int shader_index)
{
	Asm *as = (Asm *) self;
	as->DisassembleOpenGLBinary(path, shader_index);
}


void *SIAsmWrapGetInstInfo(struct SIAsmWrap *self, int index)
{
	Asm *as = (Asm *) self;
	return (void *) as->GetInstInfo(index);
}


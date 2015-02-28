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
#include <fstream>
#include <sstream>

#include <lib/cpp/Misc.h>

#include "Asm.h"
#include "Binary.h"
#include "Inst.h"


namespace SI
{

Asm::Asm()
{
	InstInfo *info;
	int i;

	// Type size assertions
	assert(sizeof(InstReg) == 4);

	// Read information about all instructions
#define DEFINST(_name, _fmt_str, _fmt, _op, _size, _flags) \
	info = &inst_info[INST_##_name]; \
	info->opcode = INST_##_name; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->fmt = InstFormat##_fmt; \
	info->op = _op; \
	info->size = _size; \
	info->flags = (InstFlag) _flags;
#include "Inst.def"
#undef DEFINST

	// Tables of pointers to 'inst_info'
	for (i = 1; i < InstOpcodeCount; i++)
	{
		info = &inst_info[i];

		if (info->fmt == InstFormatSOPP)
		{
			assert(misc::inRange(info->op, 0, dec_table_sopp_count - 1));
			dec_table_sopp[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatSOPC)
		{
			assert(misc::inRange(info->op, 0, dec_table_sopc_count - 1));
			dec_table_sopc[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatSOP1)
		{
			assert(misc::inRange(info->op, 0, dec_table_sop1_count - 1));
			dec_table_sop1[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatSOPK)
		{
			assert(misc::inRange(info->op, 0, dec_table_sopk_count - 1));
			dec_table_sopk[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatSOP2)
		{
			assert(misc::inRange(info->op, 0, dec_table_sop2_count - 1));
			dec_table_sop2[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatSMRD) 
		{
			assert(misc::inRange(info->op, 0, dec_table_smrd_count - 1));
			dec_table_smrd[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatVOP3a || info->fmt == InstFormatVOP3b)
		{
			int i;

			assert(misc::inRange(info->op, 0, dec_table_vop3_count - 1));
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
			assert(misc::inRange(info->op, 0, dec_table_vopc_count - 1));
			dec_table_vopc[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatVOP1)
		{
			assert(misc::inRange(info->op, 0, dec_table_vop1_count - 1));
			dec_table_vop1[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatVOP2)
		{
			assert(misc::inRange(info->op, 0, dec_table_vop2_count - 1));
			dec_table_vop2[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatVINTRP)
		{
			assert(misc::inRange(info->op, 0, dec_table_vintrp_count - 1));
			dec_table_vintrp[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatDS)
		{
			assert(misc::inRange(info->op, 0, dec_table_ds_count - 1));
			dec_table_ds[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatMTBUF)
		{
			assert(misc::inRange(info->op, 0, dec_table_mtbuf_count - 1));
			dec_table_mtbuf[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatMUBUF)
		{
			assert(misc::inRange(info->op, 0, dec_table_mubuf_count - 1));
			dec_table_mubuf[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatMIMG)
		{
			assert(misc::inRange(info->op, 0, dec_table_mimg_count - 1));
			dec_table_mimg[info->op] = info;
			continue;
		}
		else if (info->fmt == InstFormatEXP)
		{
			assert(misc::inRange(info->op, 0, dec_table_exp_count - 1));
			dec_table_exp[info->op] = info;
			continue;
		}
		else 
		{
			std::cerr << "warning: '" << info->name
					<< "' not indexed\n";
		}
	}
}


void Asm::DisassembleBuffer(std::ostream& os, const char *buffer, int size)
{
	std::stringstream ss;

	const char *original_buffer = buffer;

	int inst_count = 0;
	int rel_addr = 0;

	int label_addr[size / 4];	// A list of created labels sorted by rel_addr.

	int *next_label = &label_addr[0];	// The next label to dump.
	int *end_label = &label_addr[0];	// The address after the last label.

	InstFormat format;
	InstBytes *bytes;

	Inst inst(this);

	// Read through instructions to find labels.
	while (buffer < original_buffer + size)
	{
		// Decode instruction
		inst.Decode(buffer, rel_addr);
		format = inst.getFormat();
		bytes = inst.getBytes();

		// If ENDPGM, break.
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

			// Find position to insert label.
			int *t_label = &label_addr[0];

			while (t_label < end_label && *t_label < label)
				t_label++;

			if (label != *t_label || t_label == end_label)
			{

				// Shift labels after position down.
				int *t2_label = end_label;

				while (t2_label > t_label)
				{
					*t2_label = *(t2_label - 1);
					t2_label--;
				}
				end_label++;

				// Insert the new label.
				*t_label = label;
			}

		}

		buffer += inst.getSize();
		rel_addr += inst.getSize();
	}


	// Reset to disassemble.
	buffer = original_buffer;
	rel_addr = 0;

	// Disassemble
	while (buffer < original_buffer + size)
	{
		// Parse the instruction
		inst.Decode(buffer, rel_addr);
		format = inst.getFormat();
		bytes = inst.getBytes();
		inst_count++;

		// Dump a label if necessary.
		if (*next_label == rel_addr && next_label != end_label)
		{
			os << misc::fmt("label_%04X:\n", rel_addr / 4);
			next_label++;
		}

		// Dump the instruction
		ss.str("");
		ss << ' ';
		inst.Dump(ss);

		// Spaces
		if (ss.str().length() < 59)
			ss << std::string(59 - ss.str().length(), ' ');

		// Hex dump
		os << ss.str();
		os << misc::fmt(" // %08X: %08X", rel_addr, bytes->word[0]);
		if (inst.getSize() == 8)
			os << misc::fmt(" %08X", bytes->word[1]);
		os << '\n';

		// Break at end of program.
		if (format == InstFormatSOPP && bytes->sopp.op == 1)
			break;

		// Increment instruction pointer
		buffer += inst.getSize();
		rel_addr += inst.getSize();
	}
}

void Asm::DisassembleBinary(std::string path)
{
	// Load ELF file
	ELFReader::File file(path);

	// Decode external ELF
	for (int i = 0; i < file.getNumSymbols(); i++)
	{
		// Get symbol
		ELFReader::Symbol *symbol = file.getSymbol(i);
		std::string symbol_name = symbol->getName();

		/* If symbol is '__OpenCL_XXX_kernel', it points 
		 * to internal ELF */
		if (misc::StringPrefix(symbol_name, "__OpenCL_") &&
				misc::StringSuffix(symbol_name, "_kernel"))
		{
			// Symbol must point to valid content
			if (!symbol->getBuffer())
				throw Error(misc::fmt(
					"%s: symbol '%s' without content",
					path.c_str(), symbol_name.c_str()));

			// Get kernel name
			std::string kernel_name = symbol_name.substr(9,
					symbol_name.length() - 16);
			std::cout << "**\n** Disassembly for '__kernel " <<
					kernel_name << "'\n**\n\n";

			// Create internal ELF
			Binary binary(symbol->getBuffer(), symbol->getSize(), kernel_name);

			// Get section with Southern Islands ISA
			BinaryDictEntry *si_dict_entry = binary.GetSIDictEntry();
			ELFReader::Section *section = si_dict_entry->text_section;

			// Disassemble
			DisassembleBuffer(std::cout, section->getBuffer(), section->getSize());
			std::cout << "\n\n\n";
		}
	}
}

}  // namespace SI



//////////////
extern "C" {
#include <lib/util/list.h>
#include "opengl-bin-file.h"
}
///////////////

using namespace SI;

void Asm::DisassembleOpenGLBinary(std::string path, int shader_index)
{
	struct opengl_si_program_binary_t *program_bin;
	struct opengl_si_shader_binary_t *shader;

	// Open file
	std::ifstream f(path);
	if (!f)
		throw misc::Error(misc::fmt("%s: cannot open file",
				path.c_str()));

	// Load file into string
	std::stringstream ss;
	ss << f.rdbuf();
	std::string s = ss.str();

	// Analyze the file and initialize structure	
	program_bin = opengl_si_program_binary_create(s.c_str(), s.length(),
			path.c_str());

	// Basic info of the shader binary
	std::cout << "This program binary contains "
			<< list_count(program_bin->shader_bins)
			<< " shaders\n\n";
	if (shader_index > list_count(program_bin->shader_bins) ||
			shader_index <= 0 )
		throw misc::Error(misc::fmt("Shader index out of range.\n"
				"\tPlease choose <index> "
				"from 1 ~ %d",
				list_count(program_bin->shader_bins)));

	// Disassemble
	shader = (struct opengl_si_shader_binary_t *)
			list_get(program_bin->shader_bins,
			shader_index - 1);
	std::cout << "**\n** Disassembly for shader " << shader_index << "\n**\n\n";
	DisassembleBuffer(std::cout, (char *)shader->isa->ptr,
			shader->isa->size);
	opengl_si_shader_binary_debug_meta(shader);
	std::cout << "\n\n\n";

	// Free
	opengl_si_program_binary_free(program_bin);
}


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

#ifndef ARCH_SOUTHERN_ISLANDS_ASM_H
#define ARCH_SOUTHERN_ISLANDS_ASM_H

#include <cassert>
#include <iostream>

#include <lib/cpp/Error.h>

#include "Inst.h"


namespace SI
{


class Asm
{
	static const int dec_table_sopp_count = 24;
	static const int dec_table_sopc_count = 17;
	static const int dec_table_sop1_count = 54;
	static const int dec_table_sopk_count = 22;
	static const int dec_table_sop2_count = 45;
	static const int dec_table_smrd_count = 32;
	static const int dec_table_vop3_count = 453;
	static const int dec_table_vopc_count = 248;
	static const int dec_table_vop1_count = 69;
	static const int dec_table_vop2_count = 50;
	static const int dec_table_vintrp_count = 4;
	static const int dec_table_ds_count = 212;
	static const int dec_table_mtbuf_count = 8;
	static const int dec_table_mubuf_count = 114;
	static const int dec_table_mimg_count = 97;
	static const int dec_table_exp_count = 1;

	// Info about Southern Islands instructions.
	InstInfo inst_info[InstOpcodeCount];

	// Pointers to elements in 'inst_info'
	InstInfo *dec_table_sopp[dec_table_sopp_count];
	InstInfo *dec_table_sopc[dec_table_sopc_count];
	InstInfo *dec_table_sop1[dec_table_sop1_count];
	InstInfo *dec_table_sopk[dec_table_sopk_count];
	InstInfo *dec_table_sop2[dec_table_sop2_count];
	InstInfo *dec_table_smrd[dec_table_smrd_count];
	InstInfo *dec_table_vop3[dec_table_vop3_count];
	InstInfo *dec_table_vopc[dec_table_vopc_count];
	InstInfo *dec_table_vop1[dec_table_vop1_count];
	InstInfo *dec_table_vop2[dec_table_vop2_count];
	InstInfo *dec_table_vintrp[dec_table_vintrp_count];
	InstInfo *dec_table_ds[dec_table_ds_count];
	InstInfo *dec_table_mtbuf[dec_table_mtbuf_count];
	InstInfo *dec_table_mubuf[dec_table_mubuf_count];
	InstInfo *dec_table_mimg[dec_table_mimg_count];
	InstInfo *dec_table_exp[dec_table_exp_count];

	void DisassembleBuffer(std::ostream& os, const char *buffer, int size);

public:

	/// Error in Southern Islands disassembler
	class Error : public misc::Error
	{
	public:

		/// Constructor
		Error(const std::string &message) : misc::Error(message)
		{
			AppendPrefix("Southern Islands Disassembler");
		}
	};

	// Constructor
	Asm();

	// Disassemblers
	void DisassembleBinary(std::string path);
	void DisassembleOpenGLBinary(std::string path, int shader_index);

	// Getters
	InstInfo *getInstInfo(int index) { assert(index >= 0 && index <
			InstOpcodeCount); return &inst_info[index]; }
	InstInfo *getDecTableSopp(int index) { assert(index >= 0 && index <
			dec_table_sopp_count); return dec_table_sopp[index]; }
	InstInfo *getDecTableSopc(int index) { assert(index >= 0 && index <
			dec_table_sopc_count); return dec_table_sopc[index]; }
	InstInfo *getDecTableSop1(int index) { assert(index >= 0 && index <
			dec_table_sop1_count); return dec_table_sop1[index]; }
	InstInfo *getDecTableSopk(int index) { assert(index >= 0 && index <
			dec_table_sopk_count); return dec_table_sopk[index]; }
	InstInfo *getDecTableSop2(int index) { assert(index >= 0 && index <
			dec_table_sop2_count); return dec_table_sop2[index]; }
	InstInfo *getDecTableSmrd(int index) { assert(index >= 0 && index <
			dec_table_smrd_count); return dec_table_smrd[index]; }
	InstInfo *getDecTableVop3(int index) { assert(index >= 0 && index <
			dec_table_vop3_count); return dec_table_vop3[index]; }
	InstInfo *getDecTableVopc(int index) { assert(index >= 0 && index <
			dec_table_vopc_count); return dec_table_vopc[index]; }
	InstInfo *getDecTableVop1(int index) { assert(index >= 0 && index <
			dec_table_vop1_count); return dec_table_vop1[index]; }
	InstInfo *getDecTableVop2(int index) { assert(index >= 0 && index <
			dec_table_vop2_count); return dec_table_vop2[index]; }
	InstInfo *getDecTableVintrp(int index) { assert(index >= 0 && index <
			dec_table_vintrp_count); return dec_table_vintrp[index]; }
	InstInfo *getDecTableDs(int index) { assert(index >= 0 && index <
			dec_table_ds_count); return dec_table_ds[index]; }
	InstInfo *getDecTableMtbuf(int index) { assert(index >= 0 && index <
			dec_table_mtbuf_count); return dec_table_mtbuf[index]; }
	InstInfo *getDecTableMubuf(int index) { assert(index >= 0 && index <
			dec_table_mubuf_count); return dec_table_mubuf[index]; }
	InstInfo *getDecTableMimg(int index) { assert(index >= 0 && index <
			dec_table_mimg_count); return dec_table_mimg[index]; }
	InstInfo *getDecTableExp(int index) { assert(index >= 0 && index <
			dec_table_exp_count); return dec_table_exp[index]; }
};


}  // namespace SI

#endif


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

#ifndef ARCH_SOUTHERN_ISLANDS_DISASSEMBLER_DISASSEMBLER_H
#define ARCH_SOUTHERN_ISLANDS_DISASSEMBLER_DISASSEMBLER_H

#include <cassert>
#include <iostream>

#include <arch/common/Disassembler.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/Error.h>

#include "Instruction.h"


namespace SI
{


class Disassembler : public comm::Disassembler
{
	// Unique instance of Southern Islands Disassembler                          
	static std::unique_ptr<Disassembler> instance;  

	// Binary file provided by the user for disassembly                                                    
	static std::string binary_file;

	// Number of instructions in each category
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
	Instruction::Info inst_info[Instruction::OpcodeCount];

	// Pointers to elements in 'inst_info'
	Instruction::Info *dec_table_sopp[dec_table_sopp_count];
	Instruction::Info *dec_table_sopc[dec_table_sopc_count];
	Instruction::Info *dec_table_sop1[dec_table_sop1_count];
	Instruction::Info *dec_table_sopk[dec_table_sopk_count];
	Instruction::Info *dec_table_sop2[dec_table_sop2_count];
	Instruction::Info *dec_table_smrd[dec_table_smrd_count];
	Instruction::Info *dec_table_vop3[dec_table_vop3_count];
	Instruction::Info *dec_table_vopc[dec_table_vopc_count];
	Instruction::Info *dec_table_vop1[dec_table_vop1_count];
	Instruction::Info *dec_table_vop2[dec_table_vop2_count];
	Instruction::Info *dec_table_vintrp[dec_table_vintrp_count];
	Instruction::Info *dec_table_ds[dec_table_ds_count];
	Instruction::Info *dec_table_mtbuf[dec_table_mtbuf_count];
	Instruction::Info *dec_table_mubuf[dec_table_mubuf_count];
	Instruction::Info *dec_table_mimg[dec_table_mimg_count];
	Instruction::Info *dec_table_exp[dec_table_exp_count];

	// Private constructor for singleton
	Disassembler();

	// Disassembler the content of the given buffer.
	void DisassembleBuffer(std::ostream &os, const char *buffer, int size);

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

	/// Register options in the command line                                 
	static void RegisterOptions();                                           

	/// Process command-line options                                         
	static void ProcessOptions();  

	/// Get the only instance of the Southern Islands disassembler. If the       
	/// instance does not exist yet, it will be created, and will remain     
	/// allocated until the end of the execution.                            
	static Disassembler *getInstance(); 

	/// Destroy the disassembler singleton if allocated
	static void Destroy() { instance = nullptr; }
	
	/// Disassemble the binary file given in \a path
	void DisassembleBinary(const std::string &path);

	Instruction::Info *getInstInfo(int index)
	{
		assert(index >= 0 && index < Instruction::OpcodeCount);
		return &inst_info[index];
	}

	Instruction::Info *getDecTableSopp(int index)
	{
		assert(index >= 0 && index < dec_table_sopp_count);
		return dec_table_sopp[index];
	}

	Instruction::Info *getDecTableSopc(int index)
	{
		assert(index >= 0 && index < dec_table_sopc_count);
		return dec_table_sopc[index];
	}

	Instruction::Info *getDecTableSop1(int index)
	{
		assert(index >= 0 && index < dec_table_sop1_count);
		return dec_table_sop1[index];
	}

	Instruction::Info *getDecTableSopk(int index)
	{
		assert(index >= 0 && index < dec_table_sopk_count);
		return dec_table_sopk[index];
	}

	Instruction::Info *getDecTableSop2(int index)
	{
		assert(index >= 0 && index < dec_table_sop2_count);
		return dec_table_sop2[index];
	}

	Instruction::Info *getDecTableSmrd(int index)
	{
		assert(index >= 0 && index < dec_table_smrd_count);
		return dec_table_smrd[index];
	}

	Instruction::Info *getDecTableVop3(int index)
	{
		assert(index >= 0 && index < dec_table_vop3_count);
		return dec_table_vop3[index];
	}

	Instruction::Info *getDecTableVopc(int index)
	{
		assert(index >= 0 && index < dec_table_vopc_count);
		return dec_table_vopc[index];
	}

	Instruction::Info *getDecTableVop1(int index)
	{
		assert(index >= 0 && index < dec_table_vop1_count);
		return dec_table_vop1[index];
	}

	Instruction::Info *getDecTableVop2(int index)
	{
		assert(index >= 0 && index < dec_table_vop2_count);
		return dec_table_vop2[index];
	}

	Instruction::Info *getDecTableVintrp(int index)
	{
		assert(index >= 0 && index < dec_table_vintrp_count);
		return dec_table_vintrp[index];
	}

	Instruction::Info *getDecTableDs(int index)
	{
		assert(index >= 0 && index < dec_table_ds_count);
		return dec_table_ds[index];
	}

	Instruction::Info *getDecTableMtbuf(int index)
	{
		assert(index >= 0 && index < dec_table_mtbuf_count);
		return dec_table_mtbuf[index];
	}

	Instruction::Info *getDecTableMubuf(int index)
	{
		assert(index >= 0 && index < dec_table_mubuf_count);
		return dec_table_mubuf[index];
	}

	Instruction::Info *getDecTableMimg(int index)
	{
		assert(index >= 0 && index < dec_table_mimg_count);
		return dec_table_mimg[index];
	}

	Instruction::Info *getDecTableExp(int index)
	{
		assert(index >= 0 && index < dec_table_exp_count);
		return dec_table_exp[index];
	}
};


}  // namespace SI

#endif


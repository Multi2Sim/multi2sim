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

#ifndef MIPS_ASM_ASM_H
#define MIPS_ASM_ASM_H

#include "Inst.h"

namespace MIPS
{


class Asm
{
public:
	// Decoding tables
	InstInfo *dec_table;
	InstInfo *dec_table_special;
	InstInfo *dec_table_special_movci;
	InstInfo *dec_table_special_srl;
	InstInfo *dec_table_special_srlv;

	InstInfo *dec_table_regimm;

	InstInfo *dec_table_cop0;
	InstInfo *dec_table_cop0_c0;
	InstInfo *dec_table_cop0_notc0;
	InstInfo *dec_table_cop0_notc0_mfmc0;

	InstInfo *dec_table_cop1;
	InstInfo *dec_table_cop1_bc1;
	InstInfo *dec_table_cop1_s;
	InstInfo *dec_table_cop1_s_movcf;
	InstInfo *dec_table_cop1_d;
	InstInfo *dec_table_cop1_d_movcf;
	InstInfo *dec_table_cop1_w;
	InstInfo *dec_table_cop1_l;
	InstInfo *dec_table_cop1_ps;

	InstInfo *dec_table_cop2;
	InstInfo *dec_table_cop2_bc2;

	InstInfo *dec_table_special2;

	InstInfo *dec_table_special3;
	InstInfo *dec_table_special3_bshfl;

	// Creator and destructor
	Asm();
	~Asm();

	// Disassemble binary file given a path
	void DisassembleBinary(std::string path);
};



} // namespace MIPS


#endif  // MIPS_ASM_ASM_H


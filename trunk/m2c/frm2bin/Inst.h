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

#ifndef FRM2BIN_INST_H
#define FRM2BIN_INST_H

#include <arch/fermi/asm/Asm.h>
#include <arch/fermi/asm/Inst.h>

#ifdef __cplusplus

#include <vector>

namespace frm2bin
{
class Inst
{
	/* Instruction opcode. This field should match the content of
	 * info->info->opcode. */
	Fermi::InstOpcode opcode;

	/* Invariable information related with this instruction */
	InstInfo *info;

	/* Instruction bytes generated */
	Fermi::InstBytes bytes;

	/* List of arguments */
	std::vector<std::unique_ptr<Arg>> args;

	/* List of modifiers */
	std::vector<std::unique_ptr<Mod>> mods;

	/* address of current instruction in the program,
	 * useful for encoding of instByte.tgt.target when
	 * argument is literal   */
	long long int addr;

	/* Instruction size in bytes, 4 or 8 */
	int size;

	/* Predicate register number */
	int pred_num;
};

/* Struct for predicate type */
struct PredType
{
	/* Predicate register to use */
	unsigned int number;
};

} /* namespace frm2bin */

#endif  /* __cplusplus */


#endif /* FRM2BIN_INST_H */

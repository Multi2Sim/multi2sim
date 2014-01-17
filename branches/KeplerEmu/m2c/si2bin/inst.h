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

#ifndef M2C_SI2BIN_INST_H_OLD
#define M2C_SI2BIN_INST_H_OLD

#include <stdio.h>
#include <arch/southern-islands/asm/Wrapper.h>
#include <lib/class/class.h>



/*
 * Class 'Si2binInst'
 */

CLASS_BEGIN(Si2binInst, Object)

	/* FIXME - bring to parent */
	int size;

	/* FIXME - bring to parent */
	SIInstBytes bytes;

	SIInstOpcode opcode;
	Si2binInstInfo *info;

	/* List of arguments. Each element is of type 'Si2binArg' */
	List *arg_list;

	/* For LLVM-to-SI back-end: basic block that the instruction
	 * belongs to. */
	Llvm2siBasicBlock *basic_block;

	/* Comment attached to the instruction, which will be dumped together
	 * with it. */
	String *comment;

CLASS_END(Si2binInst)


/* Create a new instruction with an opcode of type 'SIInstOpcode', as
 * defined in the Southern Islands disassembler. The argument list in 'arg_list'
 * is composed of objects of type 'Si2binArg'. All these objects, as well as
 * the argument list itself, will be freed by the class destructor. */
void Si2binInstCreate(Si2binInst *self, SIInstOpcode opcode, List *arg_list);

/* Create a new instruction with the opcode corresponding to the first instruction
 * named 'name' that matches the number and type of arguments passed in 'arg_list'.
 * The list of arguments in 'arg_list' and 'arg_list' itself will be freed
 * internally in the class destructor. */
void Si2binInstCreateWithName(Si2binInst *self, char *name, List *arg_list);

void Si2binInstDestroy(Si2binInst *self);

void Si2binInstDump(Si2binInst *self, FILE *f);
void Si2binInstDumpAssembly(Si2binInst *self, FILE *f);

/* Attach a comment to the instruction */
void Si2binInstAddComment(Si2binInst *self, char *comment);

/* Populate fields 'inst_bytes' and 'size' based on the instruction and the value
 * of its arguments. */
void Si2binInstGenerate(Si2binInst *self);

#endif


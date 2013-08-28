/*
 *  Multi2Sim
 *  Copyright (C) 2013 Chris Barton (barton.ch@husky.neu.edu)
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

#include <stdio.h>

#include <llvm-c/Core.h>

#include "ret-error.h"
#include "cl2llvm.h"

extern LLVMBasicBlockRef current_basic_block;

int check_for_ret()
{
	if (LLVMGetLastInstruction(current_basic_block) != NULL)
	{
		if (LLVMGetInstructionOpcode(LLVMGetLastInstruction(
			current_basic_block)) != LLVMRet)
			return 1;
		else
			return 0;
	}
	else
		return 1;
}

int check_for_ret_w_error()
{
	if (LLVMGetLastInstruction(current_basic_block) != NULL)
	{
		if (LLVMGetInstructionOpcode(LLVMGetLastInstruction(
			current_basic_block)) != LLVMRet)
			return 1;
		else
		{
			cl2llvm_yyerror("terminator in middle of basic block");
			return 0;
		}
	}
	else
		return 1;
}


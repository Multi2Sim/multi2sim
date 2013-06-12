/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <llvm-c/BitReader.h>
#include <llvm-c/Core.h>

#include "basic-block.h"
#include "function.h"
#include "translate.h"


/* Private variables */
static FILE *llvm2si_outf;


void llvm2si_translate(char *source_file, char *output_file)
{
	LLVMMemoryBufferRef memory_buffer;
	LLVMModuleRef module;
	LLVMValueRef llfunction;
	LLVMBool err;

	char *message;

	struct llvm2si_function_t *function;
	struct llvm2si_basic_block_t *basic_block;

	/* Create memory buffer with source file */
	err = LLVMCreateMemoryBufferWithContentsOfFile(source_file,
			&memory_buffer, &message);
	if (err)
		fatal("%s: %s", source_file, message);

	/* Read module */
	err = LLVMParseBitcode(memory_buffer, &module, &message);
	if (err)
		fatal("%s: %s", source_file, message);
	
	/* Open output file */
	llvm2si_outf = fopen(output_file, "w");
	if (!llvm2si_outf)
		fatal("%s: cannot open output file", output_file);

	/* Translate all functions */
	for (llfunction = LLVMGetFirstFunction(module); llfunction;
			llfunction = LLVMGetNextFunction(llfunction))
	{
		/* Create function */
		function = llvm2si_function_create(llfunction);

		/* Create a basic block and generate header code in it */
		basic_block = llvm2si_basic_block_create("entry");
		llvm2si_function_add_basic_block(function, basic_block);

		/* Emit code for function header */
		llvm2si_function_emit_header(function, basic_block);

		/* Emit code to load function arguments */
		llvm2si_function_emit_args(function, basic_block);

		/* Free function */
		llvm2si_function_free(function);
	}

	/* Close output file */
	fclose(llvm2si_outf);

	/* Free module and memory buffer */
	LLVMDisposeModule(module);
	LLVMDisposeMemoryBuffer(memory_buffer);
}


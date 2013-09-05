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


#include <m2c/common/ctree.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <llvm-c/BitReader.h>

#include "basic-block.h"
#include "function.h"
#include "llvm2si.h"
#include "phi.h"
#include "symbol.h"
#include "symbol-table.h"


/*
 * Private Functions
 */

static void llvm2si_compile_file(char *source_file, char *output_file)
{
	LLVMMemoryBufferRef llmem_buf;
	LLVMModuleRef llmodule;
	LLVMValueRef llfunction;
	LLVMAttribute llattr;
	LLVMBool err;

	char *message;
	FILE *f;

	Llvm2siFunction *function;

	/* Create memory buffer with source file */
	err = LLVMCreateMemoryBufferWithContentsOfFile(source_file,
			&llmem_buf, &message);
	if (err)
		fatal("%s: %s", source_file, message);

	/* Read module */
	err = LLVMParseBitcode(llmem_buf, &llmodule, &message);
	if (err)
		fatal("%s: %s", source_file, message);

	/* Open output file */
	f = fopen(output_file, "w");
	if (!f)
		fatal("%s: cannot open output file", output_file);

	/* Translate all functions */
	for (llfunction = LLVMGetFirstFunction(llmodule); llfunction;
			llfunction = LLVMGetNextFunction(llfunction))
	{
		/* Ignore built-in functions. Built-in function declarations are
		 * generated by the CL-to-LLVM front-end with the 'nounwind'
		 * attribute. */
		llattr = LLVMGetFunctionAttr(llfunction);
		if (llattr & LLVMNoUnwindAttribute)
			continue;

		/* Create function */
		function = new(Llvm2siFunction, llfunction);

		/* Emit code for function */
		Llvm2siFunctionEmitHeader(function);
		Llvm2siFunctionEmitArgs(function);
		Llvm2siFunctionEmitBody(function);
		Llvm2siFunctionEmitPhi(function);
		Llvm2siFunctionEmitControlFlow(function);

		/* Dump code */
		Llvm2siFunctionDump(asObject(function), f);

		/* Done */
		delete(function);
	}

	/* Close output file */
	fclose(f);

	/* Free module and memory buffer */
	LLVMDisposeModule(llmodule);
	LLVMDisposeMemoryBuffer(llmem_buf);
}




/*
 * Public Functions
 */

void Llvm2siCreate(Llvm2si *self)
{
}


void Llvm2siDestroy(Llvm2si *self)
{
}


void Llvm2siCompile(Llvm2si *self, struct list_t *source_file_list,
		struct list_t *output_file_list)
{
	char *source_file;
	char *output_file;

	int index;

	LIST_FOR_EACH(source_file_list, index)
	{
		/* Get file names */
		source_file = list_get(source_file_list, index);
		output_file = list_get(output_file_list, index);
		assert(source_file);
		assert(output_file);

		/* Compile one file */
		llvm2si_compile_file(source_file, output_file);
	}
}

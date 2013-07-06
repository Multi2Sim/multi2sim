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

#include <assert.h>
#include <stdio.h>

#include <m2c/si2bin/arg.h>
#include <m2c/si2bin/inst.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/config.h>
#include <lib/util/debug.h>
#include <lib/util/linked-list.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <llvm-c/BitReader.h>
#include <llvm-c/Core.h>

#include "basic-block.h"
#include "ctree.h"
#include "function.h"
#include "llvm2si.h"


/* Public Variables */

char *llvm2si_config_file_name = "";
char *llvm2si_debug_file_name = "";
int llvm2si_debug_category;



/*
 * Private Functions
 */

/* List of control trees created during the parsing of the configuration file,
 * to keep track of loaded control trees. */
static struct linked_list_t *llvm2si_ctree_list;

static struct llvm2si_ctree_t *llvm2si_get_ctree(char *name)
{
	struct llvm2si_ctree_t *ctree;

	/* Search control tree */
	LINKED_LIST_FOR_EACH(llvm2si_ctree_list)
	{
		ctree = linked_list_get(llvm2si_ctree_list);
		if (!strcmp(ctree->name, name))
			return ctree;
	}

	/* Not found */
	return NULL;
}

static void llvm2si_process_command(char *string)
{
	struct list_t *token_list;
	char *command;

	/* Get list of tokens */
	token_list = str_token_list_create(string, " ");
	command = list_get(token_list, 0);
	if (!command)
		fatal("%s: empty command", __FUNCTION__);
	
	/* Process command */
	if (!strcasecmp(command, "LoadCTree"))
	{
		struct llvm2si_ctree_t *ctree;
		struct config_t *ctree_config;

		char *file_name;
		char *ctree_name;

		/* Syntax: LoadCTree <file> <name> */
		if (token_list->count != 3)
			fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command);
		file_name = list_get(token_list, 1);
		ctree_name = list_get(token_list, 2);

		/* Open control tree INI file */
		ctree_config = config_create(file_name);
		config_load(ctree_config);
		
		/* Load control tree */
		ctree = llvm2si_ctree_create(ctree_name);
		llvm2si_ctree_read_from_config(ctree, ctree_config, ctree_name);
		linked_list_add(llvm2si_ctree_list, ctree);

		/* Close */
		config_free(ctree_config);
	}
	else if (!strcasecmp(command, "SaveCTree"))
	{
		struct llvm2si_ctree_t *ctree;
		struct config_t *ctree_config;

		char *file_name;
		char *ctree_name;

		/* Syntax: SaveCTree <file> <name> */
		if (token_list->count != 3)
			fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command);
		file_name = list_get(token_list, 1);
		ctree_name = list_get(token_list, 2);

		/* Get control tree */
		ctree = llvm2si_get_ctree(ctree_name);
		if (!ctree)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name);

		/* Save control tree in INI file */
		ctree_config = config_create(file_name);
		llvm2si_ctree_write_to_config(ctree, ctree_config);
		config_save(ctree_config);
		config_free(ctree_config);
	}
	else if (!strcasecmp(command, "RenameCTree"))
	{
		struct llvm2si_ctree_t *ctree;

		char *ctree_name;
		char *ctree_name2;

		/* Syntax: RenameCTree <ctree> <name> */
		if (token_list->count != 3)
			fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command);
		ctree_name = list_get(token_list, 1);
		ctree_name2 = list_get(token_list, 2);

		/* Get control tree */
		ctree = llvm2si_get_ctree(ctree_name);
		if (!ctree)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name);

		/* Rename */
		ctree->name = str_set(ctree->name, ctree_name2);
	}
	else if (!strcasecmp(command, "CompareCTree"))
	{
		struct llvm2si_ctree_t *ctree1;
		struct llvm2si_ctree_t *ctree2;

		char *ctree_name1;
		char *ctree_name2;

		/* Syntax: CompareCTree <ctree1> <ctree2> */
		if (token_list->count != 3)
			fatal("%s: %s: invalid number of arguments",
					__FUNCTION__, command);
		ctree_name1 = list_get(token_list, 1);
		ctree_name2 = list_get(token_list, 2);

		/* Get first control tree */
		ctree1 = llvm2si_get_ctree(ctree_name1);
		if (!ctree1)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name1);

		/* Get second control tree */
		ctree2 = llvm2si_get_ctree(ctree_name2);
		if (!ctree2)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name2);

		/* Compare them */
		llvm2si_ctree_compare(ctree1, ctree2);
	}
	else if (!strcasecmp(command, "StructuralAnalysis"))
	{
		struct llvm2si_ctree_t *ctree;
		char *ctree_name;

		/* Syntax: StructuralAnalysis <ctree> */
		if (token_list->count != 2)
			fatal("%s: %s: invalid syntax",
					__FUNCTION__, command);
		ctree_name = list_get(token_list, 1);

		/* Get control tree */
		ctree = llvm2si_get_ctree(ctree_name);
		if (!ctree)
			fatal("%s: %s: invalid control tree",
					__FUNCTION__, ctree_name);

		/* Structural analysis */
		llvm2si_ctree_structural_analysis(ctree);
	}
	else
		fatal("%s: invalid command: %s", __FUNCTION__, command);
	
	/* Free tokens */
	str_token_list_free(token_list);
}


static void llvm2si_read_config(void)
{
	struct config_t *config;

	char var[MAX_STRING_SIZE];
	char *section;
	char *value;

	int index;

	/* No file specified */
	if (!*llvm2si_config_file_name)
		return;

	/* Create list of control trees */
	llvm2si_ctree_list = linked_list_create();

	/* Load configuration */
	config = config_create(llvm2si_config_file_name);
	config_load(config);

	/* Process commands */
	section = "Commands";
	for (index = 0;; index++)
	{
		/* Read next command */
		snprintf(var, sizeof var, "Command[%d]", index);
		value = config_read_string(config, section, var, NULL);
		if (!value)
			break;

		/* Process command */
		llvm2si_process_command(value);
	}
	
	/* Free list of control trees */
	LINKED_LIST_FOR_EACH(llvm2si_ctree_list)
		llvm2si_ctree_free(linked_list_get(llvm2si_ctree_list));
	linked_list_free(llvm2si_ctree_list);

	/* Close configuration file */
	config_check(config);
	config_free(config);
}


static void llvm2si_compile_file(char *source_file, char *output_file)
{
	LLVMMemoryBufferRef llmem_buf;
	LLVMModuleRef llmodule;
	LLVMValueRef llfunction;
	LLVMAttribute llattr;
	LLVMBool err;

	char *message;
	FILE *f;

	struct llvm2si_function_t *function;
	struct llvm2si_basic_block_t *basic_block;

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
		function = llvm2si_function_create(llfunction);

		/* Emit code for function */
		llvm2si_function_emit_header(function);
		llvm2si_function_emit_args(function);

		/* Create a basic block and generate body in it */
		basic_block = llvm2si_basic_block_create(NULL);
		llvm2si_function_add_basic_block(function,
				basic_block);

		llvm2si_function_emit_body(function, basic_block);

		/* Dump and free function */
		llvm2si_function_dump(function, f);
		llvm2si_function_free(function);
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

void llvm2si_init(void)
{
	llvm2si_read_config();
}


void llvm2si_done(void)
{
}


void llvm2si_compile(struct list_t *source_file_list,
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


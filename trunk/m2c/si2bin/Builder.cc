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

#include "Builder.h"

namespace si2bin
{

/* Global builder */
Builder builder;


Builder::Builder()
{
	/* Initialize hash table and list with instruction information. */
	for (int i = 0; i < SI::InstOpcodeCount; i++)
	{
		/* Instruction info from disassembler */
		InstInfo *info = &inst_info_array[i];
		SI::InstInfo *inst_info = as.getInstInfo(i);
		if (!inst_info->name || !inst_info->fmt_str)
		{
			info->info = nullptr;
			info->next = nullptr;
			continue;
		}

		/* Insert instruction info structure into hash table. There could
		 * be already an instruction encoding with the same name but a
		 * different encoding. They all form a linked list. */
		InstInfo *prev_info = getInstInfo(inst_info->name);
		if (prev_info)
		{
			/* Non vop3 instructions are added first into list.
			 * Add vop3 version to end of list */
			prev_info->next = info;

			/* Non vop3 instructions are added first but vop3 version
			 * is added to the front of list */
			//info->next = prev_info;
			//hash_table_set(si2bin_inst_info_table, info->name, info);
		}
		else
		{
			inst_info_table[inst_info->name] = info;
		}
	}
}


#if 0
void Si2binDestroy(Si2bin *self)
{
	/* Free list and hash table */
	ArrayDeleteObjects(self->inst_info_array);
	delete(self->inst_info_array);
	delete(self->inst_info_table);

	/* Symbol table */
	HashTableDeleteObjects(self->symbol_table);
	delete(self->symbol_table);

	/* List of tasks */
	ListDeleteObjects(self->task_list);
	delete(self->task_list);
}


void Si2binCompile(Si2bin *self, struct list_t *source_file_list,
		struct list_t *bin_file_list)
{
	int index;
	char *output_file;
	FILE *f;
	
	LIST_FOR_EACH(source_file_list, index)
	{
		/* Open source file */
		si2bin_source_file = list_get(source_file_list, index);
		si2bin_yyin = fopen(si2bin_source_file, "r");
		if (!si2bin_yyin)
			fatal("%s: cannot open input file", si2bin_source_file);

		/* Open output file */
		output_file = list_get(bin_file_list, index);
		f = fopen(output_file, "wb");
		if (!f)
			fatal("%s: cannot output output file", output_file);

		/* Create output buffer */
		si2bin_outer_bin = si2bin_outer_bin_create();
		bin_buffer = new(ELFWriterBuffer);

		/* Parse input */
		si2bin_yyparse();

		/* Close source file */
		fclose(si2bin_yyin);
		
		/* Dump output buffer and free it */
		si2bin_outer_bin_generate(si2bin_outer_bin, bin_buffer);
		
		/* Write contents of buffer to file */
		ELFWriterBufferWriteToFile(bin_buffer, f);

		/* Free Outer ELF and bin_buffer */
		si2bin_outer_bin_free(si2bin_outer_bin);
		delete(bin_buffer);
	}
}


void Si2binDumpSymbolTable(Si2bin *self, FILE *f)
{
	Si2binSymbol *symbol;
	String *name;

	fprintf(f, "Symbol Table:\n");
	HashTableForEach(self->symbol_table, name, String)
	{
		symbol = asSi2binSymbol(HashTableGet(self->symbol_table,
				asObject(name)));
		fprintf(f, "\t");
		Si2binSymbolDump(symbol, f);
		fprintf(f, "\n");
	}
}


void Si2binDumpTaskList(Si2bin *self, FILE *f)
{
	Si2binTask *task;
	int index;

	index = 0;
	fprintf(f, "Task list:\n");
	ListForEach(self->task_list, task, Si2binTask)
	{
		fprintf(f, "\ttask %d: ", index);
		Si2binTaskDump(task, f);
		fprintf(f, "\n");
		index++;
	}
}
#endif

}  /* namespace si2bin */

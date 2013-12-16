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

#include <lib/cpp/ELFWriter.h>
#include <lib/cpp/Misc.h>

#include "Context.h"
#include "Binary.h"
#include "Token.h"


using namespace misc;

namespace si2bin
{

void Si2binConfig::Register(CommandLine &command_line)
{
	// Option --si2bin <file>
	command_line.RegisterString("--si2bin", source_file,
			"Creates an AMD Southern Islands GPU compliant ELF "
			"from the assembly file provided in <arg> using the "
			"internal Southern Islands Assembler.");
}

void Si2binConfig::Process()
{
	// Run Southern Islands Assembler
	if (!source_file.empty())
	{
		output_file = "output.bin";

		Context *context = Context::getInstance();
		context->Compile(source_file, output_file);
		exit(0);
	}
}



/*
 * Class 'InstInfo'
 */

InstInfo::InstInfo(SI::InstInfo *info)
{
	// Initialize
	this->info = info;
	next = nullptr;
	name = info->name;
	opcode = info->opcode;

	// Create list of tokens from format string
	StringTokenize(info->fmt_str, str_tokens, ", ");
	assert(str_tokens.size());
	name = str_tokens[0];
	for (unsigned i = 1; i < str_tokens.size(); i++)
	{
		// Get token type
		bool error;
		TokenType type = (TokenType) token_type_map.MapStringCase(
				str_tokens[i], error);
		if (error)
			panic("%s: invalid token string: %s",
					__FUNCTION__, str_tokens[i].c_str());

		// Add token
		tokens.emplace_back(new Token(type));
	}
}



/*
 * Class 'Context'
 */


// Global context
std::unique_ptr<Context> Context::instance;

// Config
Si2binConfig Context::config;

Context::Context()
{
	// Initialize hash table and list with instruction information.
	for (int i = 0; i < SI::InstOpcodeCount; i++)
	{
		// Instruction info from disassembler
		SI::InstInfo *inst_info = as.getInstInfo(i);
		if (!inst_info->name || !inst_info->fmt_str)
			continue;

		// Create info and add to array
		InstInfo *info = new InstInfo(inst_info);
		inst_info_array[i].reset(info);

		/* Insert instruction info structure into hash table. There could
		 * be already an instruction encoding with the same name but a
		 * different encoding. They all form a linked list. */
		InstInfo *prev_info = getInstInfo(inst_info->name);
		if (prev_info)
		{
			/* Non vop3 instructions are added first into list.
			 * Add vop3 version to end of list */
			info->InsertAfter(prev_info);

			/* Non vop3 instructions are added first but vop3 version
			 * is added to the front of list */
			//info->next = prev_info;
			//hash_table_set(si2bin_inst_info_table, info->name, info);
		}
		else
		{
			inst_info_table[info->getName()] = info;
		}
	}
}

Context *Context::getInstance()
{
	//Instance already exists
	if (instance.get())
		return instance.get();

	//Create Instance
	instance.reset(new Context());
	return instance.get();
}

void Context::Compile(const std::string &source_file, const std::string &output_file)
{

	FILE *f;
	
		// Open source file
		yyin = fopen(source_file.c_str(), "r");
		if (!yyin)
			fatal("%s: cannot open input file", source_file.c_str());

		// Open output file
		f = fopen(output_file.c_str(), "wb");
		if (!f)
			fatal("%s: cannot output output file", output_file.c_str());
		
		std::ofstream of(output_file);

		// Create output buffer
		this->outer_bin = new OuterBin();

		// Parse input
		//yyparse();

		// Close source file
		fclose(yyin);
		
		// Dump output
		this->outer_bin->Generate(of);
		
		of.close();

		delete outer_bin;

}


#if 0
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

}  // namespace si2bin

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

#include "Binary.h"
#include "Context.h"
#include "Token.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>


void si2bin_yyerror(const char *s)
{
	fprintf(stderr, "INPUT_FILE:%d: error: %s\n",
			si2bin_yylineno, s);
	exit(1);
}


void si2bin_yyerror_fmt(const char *fmt, ...)
{

	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "INPUT_FILE:%d: error: ",
			si2bin_yylineno);
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	fflush(NULL);
	exit(1);
}



namespace si2bin
{

std::string MachineName = "tahiti";

bool Context::active = false;




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
	misc::StringTokenize(info->fmt_str, str_tokens, ", ");
	assert(str_tokens.size());
	name = str_tokens[0];
	for (unsigned i = 1; i < str_tokens.size(); i++)
	{
		// Get token type
		bool error;
		TokenType type = (TokenType) token_type_map.MapStringCase(
				str_tokens[i], error);
		if (error)
			misc::panic("%s: invalid token string: %s",
					__FUNCTION__, str_tokens[i].c_str());

		// Add token
		tokens.emplace_back(new Token(type));
	}
}



//
// Class 'Context'
//

// Input file, as set by user
std::string Context::source_file;

// Output file, as set by user
std::string Context::output_file;

// Singleton instance
std::unique_ptr<Context> Context::instance;


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
		InstInfo *prev_info = getInstInfo(info->getName());
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

	// Set Unique Id default
	uniqueid = 1024;

}

Context *Context::getInstance()
{
	// Instance already exists
	if (instance.get())
		return instance.get();

	// Create Instance
	instance.reset(new Context());
	return instance.get();
}

void Context::Parse(const std::string &in, const std::string &out)
{
	// Open source file
	si2bin_yyin = fopen(in.c_str(), "r");
	if (!si2bin_yyin)
		misc::fatal("%s: cannot open input file", in.c_str());

	// Open output file
	std::ofstream of(out);

	// Create output buffer
	this->outer_bin = new OuterBin();

	// Parse input
	si2bin_yyparse();

	// Close source file
	fclose(si2bin_yyin);

	// Dump output
	this->outer_bin->Generate(of);

	of.close();

	delete outer_bin;
}


void Context::RegisterOptions()
{
	// Get command line object
	misc::CommandLine *command_line = misc::CommandLine::getInstance();

	// Category
	command_line->setCategory("Southern Islands assembler");

	// Option --si2bin <file>
	command_line->RegisterBool("--si2bin", active,
			"Interpret the source files as Southern Islands "
			"assembly code (.s) and produce one final binary for "
			"each input file (.bin) using the Southern Islands "
			"assembler.");
}


void Context::ProcessOptions()
{
}


}  // namespace si2bin

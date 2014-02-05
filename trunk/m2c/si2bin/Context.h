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

#ifndef M2C_SI2BIN_CONTEXT_H
#define M2C_SI2BIN_CONTEXT_H

#include <array>
#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

#include <arch/southern-islands/asm/Asm.h>
#include <lib/cpp/CommandLine.h>
#include <lib/cpp/ELFWriter.h>

#include "Binary.h"
#include "Inst.h"
#include "InternalBinary.h"
#include "Metadata.h"
#include "Symbol.h"
#include "Task.h"
#include "Token.h"

//Global Variables and functions for Parser.yy
int si2bin_yylex(void);
int si2bin_yyparse(void);
void si2bin_yyerror(const char *s);
void si2bin_yyerror_fmt(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

extern int si2bin_yylineno;
extern FILE *si2bin_yyin;
extern char *si2bin_yytext;
extern char *si2bin_source_file;

namespace si2bin
{

extern std::string MachineName;
class InstInfo
{
	/* There can be multiple instruction encodings for the same instruction
	 * name. This points to the next one in the list. */
	InstInfo *next;

	// Associated info structure in disassembler
	SI::InstInfo *info;

	// List of tokens in format string
	std::vector<std::string> str_tokens;
	std::vector<std::unique_ptr<Token>> tokens;

	// Instruction name. This string is equal to str_tokens[0]
	std::string name;

	// Instruction opcode as a unique integer identifier
	SI::InstOpcode opcode;

public:

	InstInfo(SI::InstInfo *info);

	// Getters

	const std::string &getName() { return name; }
	SI::InstOpcode getOpcode() { return opcode; }
	SI::InstInfo *getInfo() { return info; }
	InstInfo *getNext() { return next; }
	size_t getNumTokens() { return tokens.size(); }

	const std::string &getTokenStr(size_t index) {
		assert(index < tokens.size());
		return str_tokens[index];
	}

	Token *getToken(size_t index) {
		assert(index < tokens.size());
		return tokens[index].get();
	}

	/* Insert after another object of type InstInfo with the same name in
	 * the main hash table. */
	void InsertAfter(InstInfo *info) {
		assert(!info->next);
		info->next = this;
	}
};



class Context
{
	// Information with all Southern Islands instructions
	std::array<std::unique_ptr<InstInfo>, SI::InstOpcodeCount> inst_info_array;

	/* Hash table indexed by an instruction name. Each entry contains a
	 * linked list of instructions with that name. */
	std::unordered_map<std::string, InstInfo *> inst_info_table;
	
	/* Hash table indexed by a symbol name. Each entry contains a
	 * linked list of symbols with that name. */
	std::unordered_map<std::string, std::unique_ptr<Symbol>> symbol_table;

	std::vector<std::unique_ptr<Task>> task_list;

	// Variables needed for parsing
	OuterBin *outer_bin;
	Metadata *metadata;
	InnerBin *inner_bin;
	InnerBinEntry *entry;
	ELFWriter::Buffer *text_buffer;
	int uniqueid;

	// Southern Islands disassembler
	SI::Asm as;

	static std::unique_ptr<Context> instance;

public:

	// Constructor
	Context();

	/* Return instruction information associated with a given opcode, or
	 * null if the opcode is invalid. */
	InstInfo *getInstInfo(SI::InstOpcode opcode) {
		return (opcode > SI::InstOpcodeInvalid &&
				opcode < SI::InstOpcodeCount) ?
			inst_info_array[opcode].get() : nullptr;
	}

	/* Return the head of a linked list of InstInfo structures associated
	 * with an instruction name, or null if the name is invalid. */
	InstInfo *getInstInfo(const std::string &name) {
		auto it = inst_info_table.find(name);
		return it == inst_info_table.end() ? nullptr : it->second;
	}
	
	static Context *getInstance();


	Symbol *getSymbol(const std::string &name){
		auto it = symbol_table.find(name);
		return it == symbol_table.end() ? nullptr : it->second.get();
	}

	Symbol *NewSymbol(const std::string &name){
		symbol_table.insert(std::make_pair(name, std::unique_ptr<Symbol>(new Symbol(name))));
		return symbol_table.at(name).get();
	}

	int GetUniqueId() { return this->uniqueid; }
	OuterBin *GetOuterBin() { return this->outer_bin; }
	Metadata *GetMetadata() { return this->metadata; }
	InnerBin *GetInnerBin() { return this->inner_bin; }
	InnerBinEntry *GetEntry() { return this->entry; }
	ELFWriter::Buffer *GetTextBuffer() { return this->text_buffer; }
	

	void SetUniqueId(int uniqueid) { this->uniqueid = uniqueid; }
	void SetOuterBin(OuterBin *outer_bin) { this->outer_bin = outer_bin; }
	void SetMetadata(Metadata *metadata) { this->metadata = metadata; }
	void SetInnerBin(InnerBin *inner_bin) { this->inner_bin = inner_bin; }
	void SetEntry(InnerBinEntry *entry) { this->entry = entry; }
	void SetTextBuffer(ELFWriter::Buffer *text_buffer) { this->text_buffer = text_buffer; }

	void NewTask(int offset, Symbol *symbol, ELFWriter::Buffer *buffer){
		task_list.push_back(std::unique_ptr<Task>(new Task(offset, symbol, buffer)));
	}
	
	void TaskProcess() {
		for (auto &task : task_list) task->Process(); 
		task_list.clear();
	};

	void SymbolTableClear() { symbol_table.clear(); };
	
	
	void Compile(const std::string &source_file, const std::string &output_file);
};

}  // namespace si2bin



#endif

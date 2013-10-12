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

#ifndef M2C_SI2BIN_SI2BIN_H_OLD
#define M2C_SI2BIN_SI2BIN_H_OLD

#include <arch/southern-islands/asm/Asm.h>
#include <lib/class/class.h>


/* Forward declaration */
struct list_t;



/*
 * Class 'Si2bin'
 */

CLASS_BEGIN(Si2bin, Object)

	/* Parent - FIXME */
	struct SIAsmWrap *parent;

	/* Symbol table, elements of type 'Si2binSymbol' */
	HashTable *symbol_table;

	/* List of tasks, elements of type 'Si2binTask' */
	List *task_list;

	/* List indexed by an instruction opcode (SIInstOpcode). Each element
	 * of the list if of type 'Si2binInstInfo'. */
	Array *inst_info_array;

	/* Hash table indexed by an instruction name, returning the associated entry in
	 * of type 'Si2binInstInfo'. The name of the instruction is
	 * extracted from the first token of the format string. */
	HashTable *inst_info_table;

CLASS_END(Si2bin)

void Si2binCreate(Si2bin *self);
void Si2binDestroy(Si2bin *self);

void Si2binCompile(Si2bin *self,
		struct list_t *source_file_list,
		struct list_t *bin_file_list);

void Si2binDumpSymbolTable(Si2bin *self, FILE *f);




/*
 * Public
 */

/* Forward declarations */
struct list_t;
struct elf_enc_buffer_t;

/* Command-line options */
extern char *si2bin_machine_name;

/* Output buffer of the currently assembled file.
 * Internal use only. */
extern struct si2bin_outer_bin_t *si2bin_outer_bin;
extern struct si2bin_inner_bin_entry_t *si2bin_entry;
extern struct si2bin_inner_bin_t *si2bin_inner_bin;
extern struct si2bin_metadata_t *si2bin_metadata;
extern int si2bin_uniqueid;

int si2bin_yylex(void);
int si2bin_yyparse(void);
void si2bin_yyerror(const char *s);
void si2bin_yyerror_fmt(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

/* Variables used by parser */
extern int si2bin_yylineno;
extern FILE *si2bin_yyin;
extern char *si2bin_yytext;

/* Global Southern Islands assembler variable */
extern Si2bin *si2bin;

#endif


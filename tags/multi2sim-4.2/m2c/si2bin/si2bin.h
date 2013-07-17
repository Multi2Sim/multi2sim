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

#ifndef M2C_SI2BIN_SI2BIN_H
#define M2C_SI2BIN_SI2BIN_H

#include <stdio.h>

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

extern int si2bin_yylineno;
extern FILE *si2bin_yyin;
extern char *si2bin_yytext;


/*
 * Public functions
 */

void si2bin_init(void);
void si2bin_done(void);
void si2bin_compile(struct list_t *source_file_list,
		struct list_t *bin_file_list);


#endif


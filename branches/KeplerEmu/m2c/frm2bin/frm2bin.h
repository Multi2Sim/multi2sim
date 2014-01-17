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

#ifndef M2C_FRM2BIN_FRM2BIN_H
#define M2C_FRM2BIN_FRM2BIN_H

#include <lib/class/class.h>

/* Forward declarations */
struct list_t;


/*
 * Class 'Frm2bin'
 */

CLASS_BEGIN(Frm2bin, Object)

CLASS_END(Frm2bin)

void Frm2binCreate(Frm2bin *self);
void Frm2binDestroy(Frm2bin *self);

void Frm2binCompile(Frm2bin *self,
		struct list_t *source_file_list,
		struct list_t *bin_file_list);

void Frm2binBinaryGenerate(Frm2bin *self, Frm2binBinary *cubinary);


/*
 * Public
 */

extern struct frm2bin_inner_bin_t *frm2bin_inner_bin;
extern struct frm2bin_outer_bin_t *frm2bin_outer_bin;
extern struct frm2bin_inner_bin_entry_t *frm2bin_entry;

/* Command-line option for assembler set */
extern int frm2bin_assemble;

/* output buffer, text_section only now */
extern struct elf_enc_buffer_t *text_section_buffer;
extern Frm2binBinary *cubinary;

int frm2bin_yylex(void);

int frm2bin_yyparse(void);

void frm2bin_yyerror(const char *s);
void frm2bin_yyerror_fmt(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

extern int frm2bin_yylineno;
extern FILE *frm2bin_yyin;
extern char *frm2bin_yytext;

extern struct FrmAsmWrap *frm_asm;


#endif


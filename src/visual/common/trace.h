/*
 *  Multi2Sim Tools
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

#ifndef VISUAL_COMMON_TRACE_H
#define VISUAL_COMMON_TRACE_H

#include <stdio.h>


struct vi_trace_t;

struct vi_trace_t *vi_trace_create(char *file_name);
void vi_trace_free(struct vi_trace_t *trace);


struct vi_trace_line_t;

struct vi_trace_line_t *vi_trace_line_create_from_file(FILE *f);
struct vi_trace_line_t *vi_trace_line_create_from_trace(struct vi_trace_t *trace);
void vi_trace_line_free(struct vi_trace_line_t *line);

void vi_trace_line_dump(struct vi_trace_line_t *line, FILE *f);
void vi_trace_line_dump_plain_text(struct vi_trace_line_t *line, FILE *f);

long int vi_trace_line_get_offset(struct vi_trace_line_t *line);

char *vi_trace_line_get_command(struct vi_trace_line_t *line);
char *vi_trace_line_get_symbol(struct vi_trace_line_t *line, char *symbol_name);
int vi_trace_line_get_symbol_int(struct vi_trace_line_t *line, char *symbol_name);
long long vi_trace_line_get_symbol_long_long(struct vi_trace_line_t *line, char *symbol_name);
unsigned int vi_trace_line_get_symbol_hex(struct vi_trace_line_t *line, char *symbol_name);


#endif


/*
 *  Multi2Sim Tools
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <memvisual-private.h>

#include <ctype.h>
#include <zlib.h>
#include <hash-table.h>


/*
 * Structures
 */

struct trace_line_t
{
	int id;

	char *command;

	struct hash_table_t *symbol_table;

	long int start_offset;
	long int end_offset;
};

struct trace_file_t
{
	char *name;

	gzFile *f;
};


/*
 * Trace Line
 */

struct trace_line_t *trace_line_create(struct trace_file_t *file, int id)
{
	struct trace_line_t *line;

	char buf[4096];
	char *buf_ptr;

	long int start_offset;
	long int end_offset;

	/* Read line from trace file */
	start_offset = gztell(file->f);
	buf_ptr = gzgets(file->f, buf, sizeof buf);
	end_offset = gztell(file->f);

	/* Empty line */
	if (!buf_ptr)
		return NULL;

	/* Line too long */
	if (strlen(buf) == sizeof(buf) - 1)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Allocate */
	line = calloc(1, sizeof(struct trace_line_t));
	if (!line)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	line->id = id;
	line->start_offset = start_offset;
	line->end_offset = end_offset;
	line->symbol_table = hash_table_create(13, FALSE);

	/* Read command */
	while (isspace(*buf_ptr))
		buf_ptr++;
	line->command = buf_ptr;
	while (isalnum(*buf_ptr))
		buf_ptr++;
	if (*buf_ptr)
		*buf_ptr++ = '\0';
	if (!strlen(line->command))
		fatal("%s: line %d: invalid command", file->name, id);
	line->command = strdup(line->command);
	if (!line->command)
		fatal("%s: out of memory", __FUNCTION__);

	/* Read symbols */
	while (*buf_ptr)
	{
		char *symbol_name;
		char *symbol_value;

		/* Read symbol name */
		while (isspace(*buf_ptr))
			buf_ptr++;
		symbol_name = buf_ptr;
		while (isalnum(*buf_ptr))
			buf_ptr++;
		if (*buf_ptr != '=')
			fatal("%s: line %d: invalid format", file->name, id);
		*buf_ptr++ = '\0';

		/* Read symbol value */
		if (*buf_ptr == '"')
		{
			symbol_value = ++buf_ptr;
			while (*buf_ptr && *buf_ptr != '"')
				buf_ptr++;
			if (*buf_ptr != '"')
				fatal("%s: line %d: invalid format", file->name, id);
			*buf_ptr++ = '\0';
		}
		else
		{
			symbol_value = buf_ptr;
			while (isalnum(*buf_ptr))
				buf_ptr++;
			if (*buf_ptr)
				*buf_ptr++ = '\0';
		}
		if (!*symbol_value)
			fatal("%s: line %d: invalid format", file->name, id);

		/* Duplicate value */
		symbol_value = strdup(symbol_value);
		if (!symbol_value)
			fatal("%s: out of memory", __FUNCTION__);

		/* Insert in symbol table */
		hash_table_insert(line->symbol_table, symbol_name, symbol_value);
	}

	/* Return */
	return line;
}


void trace_line_free(struct trace_line_t *line)
{
	hash_table_free(line->symbol_table);
	free(line);
}




/*
 * Trace file
 */

struct trace_file_t *trace_file_open(char *file_name)
{
	struct trace_file_t *file;

	/* Allocate */
	file = calloc(1, sizeof(struct trace_file_t));
	if (!file)
		fatal("%s: out of memory", __FUNCTION__);

	/* Name */
	file->name = strdup(file_name);
	if (!file->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Open */
	file->f = gzopen(file_name, "r");
	if (!file->f)
		fatal("%s: cannot open trace file or invalid format", file_name);

	/* Return */
	return file;
}


void trace_file_close(struct trace_file_t *file)
{
	free(file->name);
	free(file);
}

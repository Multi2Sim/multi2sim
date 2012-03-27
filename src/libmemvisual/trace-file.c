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



struct trace_file_t
{
	char *name;
	gzFile *f;

	/* Last line number read from zip file with a call to
	 * 'trace_line_create_from_trace_file'. */
	int line_num;
};


struct trace_line_t
{
	/* Line number */
	int line_num;

	/* Command and symbols */
	char *command;
	struct hash_table_t *symbol_table;

	/* Offset in the file where it was read from */
	long int offset;
};




/*
 * Trace Line
 */

#define isidchar(c) (isalnum((c)) || (c) == '.' || (c) == '_' || (c) =='-')


/*
Read trace line from a file with the following format.
If the file reached the end (i.e., could not read the first
field in the table below), return NULL.

Size			Type	Description
-----------------------------------------------------
4			int	line_num
4			int	command_size
command_size		char*	command
4			int	num_symbols
-------- repeat 'num_symbols' times -----------------
4			int	symbol_name_size
symbol_name_size	char*	symbol_name
4			int	symbol_value_size
symbol_value_size	char*	symbol_value
-----------------------------------------------------
*/

struct trace_line_t *trace_line_create_from_file(FILE *f)
{
	struct trace_line_t *line;

	long int offset;

	int count;
	int i;

	int line_num;
	int num_symbols;

	char buf[4096];

	/* Read trace line number */
	offset = ftell(f);
	count = fread(&line_num, 1, 4, f);
	if (!count)
		return NULL;
	if (count != 4)
		panic("%s: invalid format", __FUNCTION__);

	/* Allocate */
	line = calloc(1, sizeof(struct trace_line_t));
	if (!line)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	line->offset = offset;
	line->line_num = line_num;
	line->symbol_table = hash_table_create(13, FALSE);

	/* Read command */
	str_read_from_file(f, buf, sizeof buf);
	line->command = strdup(buf);
	if (!line->command)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Number of symbols */
	count = fread(&num_symbols, 1, 4, f);
	if (count != 4)
		panic("%s: invalid format", __FUNCTION__);
	
	/* Read symbols */
	for (i = 0; i < num_symbols; i++)
	{
		char symbol_name[200];
		char *symbol_value;

		/* Read symbol */
		str_read_from_file(f, symbol_name, sizeof symbol_name);
		str_read_from_file(f, buf, sizeof buf);
		symbol_value = strdup(buf);
		if (!symbol_value)
			fatal("%s: out of memory", __FUNCTION__);

		/* Insert in symbol table */
		hash_table_insert(line->symbol_table, symbol_name, symbol_value);
	}

	/* Return */
	return line;
}


struct trace_line_t *trace_line_create_from_trace_file(struct trace_file_t *file)
{
	struct trace_line_t *line;

	long int offset;

	char buf[4096];
	char *buf_ptr;

	/* Read line from trace file */
	offset = gztell(file->f);
	buf_ptr = gzgets(file->f, buf, sizeof buf);

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
	file->line_num++;
	line->offset = offset;
	line->line_num = file->line_num;
	line->symbol_table = hash_table_create(13, FALSE);

	/* Read command */
	while (isspace(*buf_ptr))
		buf_ptr++;
	line->command = buf_ptr;
	while (isidchar(*buf_ptr))
		buf_ptr++;
	if (*buf_ptr)
		*buf_ptr++ = '\0';
	if (!strlen(line->command))
		fatal("%s: line %d: invalid command", file->name, file->line_num);
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
		while (isidchar(*buf_ptr))
			buf_ptr++;
		if (*buf_ptr != '=')
			fatal("%s: line %d: invalid format", file->name, file->line_num);
		*buf_ptr++ = '\0';

		/* Read symbol value */
		if (*buf_ptr == '"')
		{
			symbol_value = ++buf_ptr;
			while (*buf_ptr && *buf_ptr != '"')
				buf_ptr++;
			if (*buf_ptr != '"')
				fatal("%s: line %d: invalid format", file->name, file->line_num);
			*buf_ptr++ = '\0';
		}
		else
		{
			symbol_value = buf_ptr;
			while (isidchar(*buf_ptr))
				buf_ptr++;
			if (*buf_ptr)
				*buf_ptr++ = '\0';
		}
		if (!*symbol_value)
			fatal("%s: line %d: invalid format", file->name, file->line_num);

		/* Duplicate value */
		symbol_value = strdup(symbol_value);
		if (!symbol_value)
			fatal("%s: out of memory", __FUNCTION__);

		/* Insert in symbol table */
		hash_table_insert(line->symbol_table, symbol_name, symbol_value);

		/* Trailing blanks */
		while (isspace(*buf_ptr))
			buf_ptr++;
	}

	/* Return */
	return line;
}


void trace_line_free(struct trace_line_t *line)
{
	char *symbol_name;
	char *symbol_value;

	HASH_TABLE_FOR_EACH(line->symbol_table, symbol_name, symbol_value)
		free(symbol_value);
	free(line->command);
	hash_table_free(line->symbol_table);
	free(line);
}


/* Dump in binary format */
void trace_line_dump(struct trace_line_t *line, FILE *f)
{
	int count;
	int num_symbols;

	char *symbol_name;
	char *symbol_value;

	/* Dump line number */
	count = fwrite(&line->line_num, 1, 4, f);
	if (count != 4)
		fatal("%s: error writing to file", __FUNCTION__);
	
	/* Dump command */
	str_write_to_file(f, line->command);

	/* Dump number of symbols */
	num_symbols = hash_table_count(line->symbol_table);
	count = fwrite(&num_symbols, 1, 4, f);
	if (count != 4)
		fatal("%s: error writing to file", __FUNCTION__);
	
	/* Dump symbols */
	HASH_TABLE_FOR_EACH(line->symbol_table, symbol_name, symbol_value)
	{
		str_write_to_file(f, symbol_name);
		str_write_to_file(f, symbol_value);
	}
}


/* Dump in human-readable format */
void trace_line_dump_plain_text(struct trace_line_t *line, FILE *f)
{
	char *symbol_name;
	char *symbol_value;

	/* Invalid line */
	if (!line)
	{
		fprintf(f, "(nil)");
		return;
	}

	/* Print */
	fprintf(f, "%s", line->command);
	HASH_TABLE_FOR_EACH(line->symbol_table, symbol_name, symbol_value)
		fprintf(f, " %s=\"%s\"", symbol_name, symbol_value);
	fprintf(f, "\n");
}


long int trace_line_get_offset(struct trace_line_t *line)
{
	return line->offset;
}


char *trace_line_get_command(struct trace_line_t *line)
{
	return line->command;
}


char *trace_line_get_symbol_value(struct trace_line_t *line, char *symbol_name)
{
	return hash_table_get(line->symbol_table, symbol_name);
}


int trace_line_get_symbol_value_int(struct trace_line_t *line, char *symbol_name)
{
	char *value;

	value = hash_table_get(line->symbol_table, symbol_name);
	return value ? atoi(value) : 0;
}


long long trace_line_get_symbol_value_long_long(struct trace_line_t *line, char *symbol_name)
{
	char *value;

	value = hash_table_get(line->symbol_table, symbol_name);
	return value ? atoll(value) : 0;
}


unsigned int trace_line_get_symbol_value_hex(struct trace_line_t *line, char *symbol_name)
{
	char *value;
	unsigned int hex;

	value = hash_table_get(line->symbol_table, symbol_name);
	if (value)
	{
		if (strlen(value) >= 2 && !strncasecmp(value, "0x", 2))
			value += 2;
		sscanf(value, "%x", &hex);
		return hex;
	}
	else
		return 0;
}



/*
 * Trace file
 */


struct trace_file_t *trace_file_create(char *file_name)
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


void trace_file_free(struct trace_file_t *file)
{
	gzclose(file->f);
	free(file->name);
	free(file);
}


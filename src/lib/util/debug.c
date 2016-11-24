/*
 *  Libstruct
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

#include <assert.h>
#include <stdarg.h>

#include <lib/mhandle/mhandle.h>

#include "debug.h"
#include "list.h"


enum debug_status_t
{
	debug_status_invalid = 0,
	debug_status_on,
	debug_status_off
};

struct debug_category_t
{
	enum debug_status_t status;
	int space_count;

	/* File name and descriptor */
	char *file_name;
	FILE *f;
};

static struct list_t *debug_category_list;


void debug_init(void)
{
	struct debug_category_t *c;

	/* Initialize list of categories */
	debug_category_list = list_create();

	/* Create an invalid category at index 0 */
	c = xcalloc(1, sizeof(struct debug_category_t));
	list_add(debug_category_list, c);
}


void debug_done(void)
{
	struct debug_category_t *c;
	int i;

	/* Close all files */
	for (i = 0; i < list_count(debug_category_list); i++)
	{
		c = list_get(debug_category_list, i);
		if (c->file_name)
			free(c->file_name);
		if (c->f && c->f != stdout && c->f != stderr)
			fclose(c->f);
		free(c);
	}

	/* Free list */
	list_free(debug_category_list);
}


int debug_new_category(char *file_name)
{
	struct debug_category_t *c;

	/* If file name is empty, return empty category at index 0 */
	if (!file_name || !*file_name)
		return 0;

	/* Initialize */
	c = xcalloc(1, sizeof(struct debug_category_t));
	c->status = debug_status_on;
	c->file_name = xstrdup(file_name);

	/* Assign file */
	if (!strcmp(file_name, "stdout"))
	{
		c->f = stdout;
	}
	else if (!strcmp(file_name, "stderr"))
	{
		c->f = stderr;
	}
	else
	{
		c->f = fopen(file_name, "wt");
		if (!c->f)
			fatal("%s: cannot open debug file", file_name);
	}

	/* Add to list and return index */
	list_add(debug_category_list, c);
	return list_count(debug_category_list) - 1;
}


void __debug_on(int category)
{
	struct debug_category_t *c;

	assert(category > 0);
	c = list_get(debug_category_list, category);

	assert(c);
	c->status = debug_status_on;
}


void __debug_off(int category)
{
	struct debug_category_t *c;

	assert(category > 0);
	c = list_get(debug_category_list, category);

	assert(c);
	c->status = debug_status_off;
}


int __debug_status(int category)
{
	struct debug_category_t *c;

	assert(category > 0);
	c = list_get(debug_category_list, category);

	/* Return TRUE if debug is on */
	assert(c);
	return c->status == debug_status_on;
}


FILE *__debug_file(int category)
{
	struct debug_category_t *c;

	assert(category > 0);
	c = list_get(debug_category_list, category);

	assert(c);
	return c->f;
}


void __debug_flush(int category)
{
	struct debug_category_t *c;

	assert(category > 0);
	c = list_get(debug_category_list, category);

	assert(c);
	if (c->f)
		fflush(c->f);
}


void __debug_tab(int category, int space_count)
{
	struct debug_category_t *c;

	assert(category > 0);
	c = list_get(debug_category_list, category);

	assert(c);
	c->space_count = space_count;
}


void __debug_tab_inc(int category, int space_count)
{
	struct debug_category_t *c;

	assert(category > 0);
	c = list_get(debug_category_list, category);

	assert(c);
	c->space_count += space_count;
	if (c->space_count < 0)
		c->space_count = 0;
}


void __debug_tab_dec(int category, int space_count)
{
	__debug_tab_inc(category, -space_count);
}


void __debug(int category, char *fmt, ...)
{
	struct debug_category_t *c;
	va_list va;
	char spc[200];

	/* Get category */
	assert(category > 0);
	c = list_get(debug_category_list, category);
	assert(c);
	if (c->status == debug_status_off)
		return;
	
	/* Print spaces */
	if (c->space_count >= sizeof(spc))
		c->space_count = sizeof(spc) - 1;
	memset(spc, ' ', c->space_count);
	spc[c->space_count] = '\0';
	fprintf(c->f, "%s", spc);
	
	/* Print message */
	va_start(va, fmt);
	vfprintf(c->f, fmt, va);

	/* Flush */
#ifndef NDEBUG
	fflush(c->f);
#endif
}


void __debug_buffer(int category, char *buffer_name, void *buffer, int size)
{
	char buf[200];
	char *buf_ptr;

	unsigned char c;

	int trunc = 0;

	/* Clear buffer */
	memset(buf, 0, sizeof buf);
	strcpy(buf, "\"");
	buf_ptr = &buf[1];

	/* Truncate size to */
	if (size > 40)
	{
		size = 40;
		trunc = 1;
	}

	/* Dump */
	for (;;)
	{
		c = * (unsigned char *) buffer;

		/* Finish */
		if (!size)
		{
			strcpy(buf_ptr, trunc ? "\"..." : "\"");
			break;
		}

		/* Characters */
		if (c >= 32)
		{
			*buf_ptr = c;
			buf_ptr++;
		}
		else if (!c)
		{
			strcpy(buf_ptr, "\\0");
			buf_ptr += 2;
		}
		else if (c == '\n')
		{
			strcpy(buf_ptr, "\\n");
			buf_ptr += 2;
		}
		else if (c == '\t')
		{
			strcpy(buf_ptr, "\\t");
			buf_ptr += 2;
		}
		else
		{
			sprintf(buf_ptr, "\\%02x", c);
			buf_ptr += 3;
		}

		/* Next byte */
		buffer++;
		size--;
	}

	/* Print buffer */
	__debug(category, "%s=%s\n", buffer_name, buf);
}


void fatal(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "fatal: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	fflush(NULL);
	exit(1);
}


void panic(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "panic: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	fflush(NULL);
	abort();
}


void warning(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "warning: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
}


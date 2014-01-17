/*
 *  Libesim
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
#include <zlib.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "esim.h"


static gzFile trace_file;
static struct list_t *trace_category_list;

enum trace_status_t
{
	trace_status_invalid = 0,
	trace_status_on,
	trace_status_off
};

struct trace_category_t
{
	enum trace_status_t status;
};


void trace_init(char *file_name)
{
	struct trace_category_t *c;

	/* Do nothing is no file name was given */
	if (!file_name || !*file_name)
		return;

	/* Open destination file */
	trace_file = gzopen(file_name, "wt");
	if (!trace_file)
		fatal("%s: cannot open trace file", file_name);

	/* Initialize list of categories */
	trace_category_list = list_create();

	/* Create an invalid category at index 0 */
	c = xcalloc(1, sizeof(struct trace_category_t));
	list_add(trace_category_list, c);
}


void trace_done(void)
{
	/* Nothing if trace is inactive */
	if (!trace_file)
		return;

	/* Close trace file */
	gzclose(trace_file);

	/* Free categories */
	while (trace_category_list->count)
		free(list_remove_at(trace_category_list, 0));
	list_free(trace_category_list);
}


int trace_new_category(void)
{
	/* Create new category */
	struct trace_category_t *c;

	/* If trace system not initialized, return invalid cateogry */
	if (!trace_file)
		return 0;

	/* Initialize */
	c = xcalloc(1, sizeof(struct trace_category_t));
	c->status = trace_status_on;

	/* Add to list and return index */
	list_add(trace_category_list, c);
	return list_count(trace_category_list) - 1;
}


int __trace_status(int category)
{
	struct trace_category_t *c;

	assert(category > 0);
	c = list_get(trace_category_list, category);

	/* Return TRUE if debug is on */
	assert(c);
	return c->status == trace_status_on;
}


/* Cycle when last effective call to 'trace' was made */
static long long trace_last_cycle = -1;

void __trace(int category, int print_cycle, char *fmt, ...)
{
	struct trace_category_t *c;
	va_list va;
	char buf[4096];
	int len;
	long long cycle;

	/* Get category */
	assert(category > 0);
	c = list_get(trace_category_list, category);
	assert(c);
	assert(c->status);
	if (c->status == trace_status_off)
		return;

	/* Print message */
	va_start(va, fmt);
	len = vsnprintf(buf, sizeof buf, fmt, va);

	/* Message exceeded buffer */
	if (len + 1 == sizeof buf)
		fatal("%s: buffer too small", __FUNCTION__);

	/* Dump current cycle */
	if (print_cycle)
	{
		cycle = esim_cycle();
		if (cycle > trace_last_cycle)
		{
			gzprintf(trace_file, "c clk=%lld\n", cycle);
			trace_last_cycle = cycle;
		}
	}

	/* Dump message */
	gzwrite(trace_file, buf, len);
}


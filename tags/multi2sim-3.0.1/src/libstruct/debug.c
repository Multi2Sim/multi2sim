/*
 *  Libstruct
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <mhandle.h>

#include "debug.h"


struct category_t {
	int status;
	int space_count;
	FILE *f;
	char *filename;
};

static struct category_t *category_list = NULL;
static int category_list_size = 0;
static int category_count = 0;


void debug_init(void)
{
}


void debug_done(void)
{
	int i;
	FILE *f;
	for (i = 0; i < category_count; i++) {
		f = category_list[i].f;
		if (f && f != stdout && f != stderr)
			fclose(f);
	}
	if (category_list)
		free(category_list);
	category_list = NULL;
	category_list_size = 0;
	category_count = 0;
}


int debug_new_category(void)
{
	struct category_t *c;
	if (category_count == category_list_size) {
		category_list_size += 10;
		category_list = realloc(category_list, sizeof(struct category_t) * category_list_size);
		if (!category_list)
			abort();
	}
	c = &category_list[category_count];
	c->status = 1;
	c->f = NULL;
	c->filename = NULL;
	return category_count++;
}


FILE *debug_assign_file(int category, char *filename)
{
	FILE *f;
	if (category < 0 || category >= category_count)
		return NULL;
	if (!filename || !*filename)
		f = NULL;
	else if (!strcmp(filename, "stdout"))
		f = stdout;
	else if (!strcmp(filename, "stderr"))
		f = stderr;
	else
		f = fopen(filename, "wt");
	category_list[category].f = f;
	category_list[category].filename = filename;
	return f;
}


void debug_on(int category)
{
	if (category < 0 || category >= category_count)
		return;
	category_list[category].status = 1;
}


void debug_off(int category)
{
	if (category < 0 || category >= category_count)
		return;
	category_list[category].status = 1;
}


int debug_status(int category)
{
	struct category_t *c;
	if (category < 0 || category >= category_count)
		return 0;
	c = &category_list[category];
	return c->status && c->f;
}


FILE *debug_file(int category)
{
	if (category < 0 || category >= category_count)
		return NULL;
	return category_list[category].f;
}


void debug_flush(int category)
{
	struct category_t *c;
	if (category < 0 || category >= category_count)
		return;
	c = &category_list[category];
	if (c->status && c->f)
		fflush(c->f);
}


void debug_tab(int category, int space_count)
{
	struct category_t *c;
	if (category < 0 || category >= category_count)
		return;
	c = &category_list[category];
	c->space_count = space_count;
}


void debug_tab_inc(int category, int space_count)
{
	struct category_t *c;
	if (category < 0 || category >= category_count)
		return;
	c = &category_list[category];
	c->space_count += space_count;
	if (c->space_count < 0)
		c->space_count = 0;
}


void debug_tab_dec(int category, int space_count)
{
	debug_tab_inc(category, -space_count);
}


void debug(int category, char *fmt, ...)
{
	struct category_t *c;
	va_list va;
	char spc[200];

	/* Get category */
	if (category < 0 || category >= category_count)
		return;
	c = &category_list[category];
	if (!c->status || !c->f)
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
	fflush(c->f);
}


void fatal(char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "fatal: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	exit(1);
}


void panic(char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "panic: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	abort();
}


void warning(char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "warning: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
}


void dump_buf(char **pbuf, int *psize, char *fmt, ...) {
	va_list va;
	int len;
	if (*psize <= 1)
		return;
	va_start(va, fmt);
	len = vsnprintf(*pbuf, *psize, fmt, va);
	if (len >= *psize)
		len = *psize - 1;
	*psize -= len;
	*pbuf += len;
}

#endif

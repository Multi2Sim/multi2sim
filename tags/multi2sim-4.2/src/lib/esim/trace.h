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

#ifndef LIB_ESIM_TRACE_H
#define LIB_ESIM_TRACE_H

void trace_init(char *file_name);
void trace_done(void);

int trace_new_category(void);

#define trace_status(category) ((category) ? __trace_status((category)) : 0)
int __trace_status(int category);

#define trace(category, ...) ((category) ? \
	__trace((category), 1, __VA_ARGS__) : (void) 0)
#define trace_header(category, ...) ((category) ? \
	__trace((category), 0, __VA_ARGS__) : (void) 0)
void __trace(int category, int print_cycle, char *fmt, ...)
	__attribute__ ((format (printf, 3, 4)));


#endif


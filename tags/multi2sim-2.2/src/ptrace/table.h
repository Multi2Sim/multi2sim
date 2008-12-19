/*
 *  Multi2Sim-Ptrace
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

#ifndef TABLE_H
#define TABLE_H

#include <stdio.h>
#include <stdlib.h>


struct table_t;

struct table_t *table_create(char *name, int width, int height,
	int data_width, int ylbl_width);

void table_set_title(struct table_t *t, char *title);
void table_setcell(struct table_t *t, int x, int y, char *str);
void table_setxlbl(struct table_t *t, int x, char *str);
void table_setylbl(struct table_t *t, int y, char *str);
int table_visible(struct table_t *t, int x, int y);
char *table_ylbl(struct	table_t *t, int y);
void table_print(struct	table_t *t, FILE *f);

#endif

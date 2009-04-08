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

#include "table.h"

#include <string.h>

#define MOD(X, Y) (((X) + (Y)) % (Y))

struct table_t {
	char name[128];
	int width;
	int height;
	
	char ***data;
	int data_width;
	
	char **xlabel;
	char **ylabel;
	int ylbl_width;
	
	int xfirst;
	int yfirst;
	int xhead;
	int yhead;
	
	int total_width;
	int total_height;
};




static void rep(FILE *f, char c, int n)
{
	int i;
	for (i = 0; i < n; i++)
		fprintf(f, "%c", c);
}


static void printspc(FILE *f, char *s, int n)
{
	fprintf(f, "%s", s);
	rep(f, ' ', n - strlen(s));
}


static void table_clear(struct table_t *t)
{
	int i, j;
	
	/* Cells */
	for (i = 0; i < t->width; i++)
		for (j = 0; j < t->height; j++)
			t->data[i][j][0] = '\0';
			
	/* Labels */
	for (i = 0; i < t->width; i++)
		t->xlabel[i][0] = '\0';
	for (i = 0; i < t->height; i++)
		t->ylabel[i][0] = '\0';

}


static void horiz_scroll(struct table_t *t)
{
	int i;
	for (i = 0; i < t->height; i++)
		t->data[t->xhead][i][0] = '\0';
	t->xlabel[t->xhead][0] = '\0';
	t->xfirst++;
	t->xhead = MOD(t->xhead + 1, t->width);
}


static void vert_scroll(struct table_t *t)
{
	int i;
	for (i = 0; i < t->width; i++)
		t->data[i][t->yhead][0] = '\0';
	t->ylabel[t->yhead][0] = '\0';
	t->yfirst++;
	t->yhead = MOD(t->yhead + 1, t->height);
}


static int touch_cell(struct table_t *t, int x, int y)
{
	int reset = 0;
	
	if (x < t->xfirst || y < t->yfirst)
		return 0;
	
	if (x >= t->xfirst + t->width * 2 - 1) {
		reset = 1;
		t->xfirst = x - t->width + 1;
	}
	if (y >= t->yfirst + t->height * 2 - 1) {
		reset = 1;
		t->yfirst = y - t->height + 1;
	}
	if (reset)
		table_clear(t);
	
	while (x >= t->xfirst + t->width)
		horiz_scroll(t);
	while (y >= t->yfirst + t->height)
		vert_scroll(t);
	return 1;
}




struct table_t *table_create(char *name, int width, int height,
	int data_width, int ylbl_width)
{
	struct table_t *t;
	int i, j;
	
	t = (struct table_t *) calloc(1, sizeof(struct table_t));
	strncpy(t->name, name, 128);
	t->width = width;
	t->height = height;
	t->data_width = data_width;
	t->ylbl_width = ylbl_width;
	t->total_width = (data_width + 1) * width + ylbl_width;
	t->total_height = height + 2;
	
	t->data = (char ***) calloc(width, sizeof(char **));
	for (i = 0; i < width; i++) {
		t->data[i] = (char **) calloc(height, sizeof(char *));
		for (j = 0; j < height; j++)
			t->data[i][j] = (char *) calloc(1, t->data_width + 1);
	}
	
	t->xlabel = (char **) calloc(width, sizeof(char *));
	for (i = 0; i < width; i++)
		t->xlabel[i] = (char *) calloc(1, t->data_width + 1);
	t->ylabel = (char **) calloc(height, sizeof(char *));
	for (i = 0; i < height; i++)
		t->ylabel[i] = (char *) calloc(1, t->ylbl_width + 1);
	
	return t;
}


void table_setcell(struct table_t *t, int x, int y, char *str)
{
	if (!touch_cell(t, x, y))
		return;
	
	x = MOD(x - t->xfirst + t->xhead, t->width);
	y = MOD(y - t->yfirst + t->yhead, t->height);
	strncpy(t->data[x][y], str, t->data_width);
	t->data[x][y][t->data_width] = '\0';
}


void table_setxlbl(struct table_t *t, int x, char *str)
{
	if (!touch_cell(t, x, t->yfirst))
		return;
	x = MOD(x - t->xfirst + t->xhead, t->width);
	strncpy(t->xlabel[x], str, t->data_width);
	t->xlabel[x][t->data_width] = '\0';
}


void table_setylbl(struct table_t *t, int y, char *str)
{
	if (!touch_cell(t, t->xfirst, y))
		return;
	y = MOD(y - t->yfirst + t->yhead, t->height);
	strncpy(t->ylabel[y], str, t->ylbl_width);
	t->ylabel[y][t->ylbl_width] = '\0';
}


void table_set_title(struct table_t *t, char *title)
{
	strcpy(t->name, title);
}


int table_visible(struct table_t *t, int x, int y)
{
	return
		x >= t->xfirst && x < t->xfirst + t->width &&
		y >= t->yfirst && y < t->yfirst + t->height;
}


char *table_ylbl(struct	table_t *t, int y)
{
	if (!table_visible(t, t->xfirst, y))
		return NULL;
	return t->ylabel[MOD(y - t->yfirst + t->yhead, t->height)];
}


void table_print(struct	table_t *t, FILE *f)
{
	int i, j, x, y;

	rep(f, '*', 5);
	fprintf(f, " %s ", t->name);
	fflush(stdout);
	rep(f, '*', t->total_width - strlen(t->name) - 7);
	rep(f, '\n', 1);
	
	
	rep(f, ' ', t->ylbl_width + 1);
	for (i = 0; i < t->width; i++)
		printspc(f, t->xlabel[(i + t->xhead) % t->width], t->data_width + 1);
	rep(f, '\n', 1);
	
	for (i = 0; i < t->height; i++) {
		y = MOD(i + t->yhead, t->height);
		printspc(f, t->ylabel[y], t->ylbl_width + 1);
		
		for (j = 0; j < t->width; j++) {
			x = MOD(j + t->xhead, t->width);
			printspc(f, t->data[x][y], t->data_width + 1);
		}
		
		rep(f, '\n', 1);
	}
}


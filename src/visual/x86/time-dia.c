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

#include <visual-x86.h>

struct vi_x86_time_dia_t
{
	GtkWidget *widget;
	GtkWidget *tree_view;

	GtkListStore *list_store;

	int widget_width;
	int widget_height;
};


static void vi_x86_time_dia_widget_destroy(GtkWidget *widget, struct vi_x86_time_dia_t *time_dia)
{
	vi_x86_time_dia_free(time_dia);
}


static void vi_x86_time_dia_size_allocate(GtkWidget *widget, GdkRectangle *allocation,
	struct vi_x86_time_dia_t *time_dia)
{
	int widget_width;
	int widget_height;

	widget_width = gtk_widget_get_allocated_width(time_dia->widget);
	widget_height = gtk_widget_get_allocated_height(time_dia->widget);

	if (widget_width != time_dia->widget_width ||
		widget_height != time_dia->widget_height)
		vi_x86_time_dia_refresh(time_dia);
}




/*
 * Public Functions
 */

#define VI_X86_TIME_DIA_CELL_WIDTH		100
#define VI_X86_TIME_DIA_CELL_HEIGHT		20

struct vi_x86_time_dia_t *vi_x86_time_dia_create(struct vi_x86_core_t *core)
{
	struct vi_x86_time_dia_t *time_dia;

	/* Allocate */
	time_dia = calloc(1, sizeof(struct vi_x86_time_dia_t));
	if (!time_dia)
		fatal("%s: out of memory", __FUNCTION__);

	/* Layout */
	GtkWidget *layout;
	layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(layout, 600, 300);

	/* List store */
	GtkListStore *list_store;
	list_store = gtk_list_store_new(1, G_TYPE_STRING);
	time_dia->list_store = list_store;

	/* Tree view */
	GtkWidget *tree_view;
	GtkTreeViewColumn *tree_view_column;
	tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
	gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tree_view), GTK_TREE_VIEW_GRID_LINES_BOTH);

	GtkCellRenderer *cell_renderer;
	gint xpad, ypad;
	cell_renderer = gtk_cell_renderer_text_new();
	gtk_cell_renderer_get_padding(cell_renderer, &xpad, &ypad);
	gtk_cell_renderer_set_fixed_size(cell_renderer, VI_X86_TIME_DIA_CELL_WIDTH - xpad,
		VI_X86_TIME_DIA_CELL_HEIGHT - ypad);

	tree_view_column = gtk_tree_view_column_new_with_attributes(NULL, cell_renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), tree_view_column);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_view), FALSE);
	gtk_layout_put(GTK_LAYOUT(layout), tree_view, 0, 0);
	time_dia->tree_view = tree_view;


	/* Widget */
	time_dia->widget = layout;
	g_signal_connect(G_OBJECT(time_dia->widget), "destroy",
		G_CALLBACK(vi_x86_time_dia_widget_destroy), time_dia);
	g_signal_connect(G_OBJECT(time_dia->widget), "size_allocate",
		G_CALLBACK(vi_x86_time_dia_size_allocate), time_dia);

	/* Return */
	return time_dia;
}


void vi_x86_time_dia_free(struct vi_x86_time_dia_t *time_dia)
{
	free(time_dia);
}


GtkWidget *vi_x86_time_dia_get_widget(struct vi_x86_time_dia_t *time_dia)
{
	return time_dia->widget;
}


void vi_x86_time_dia_refresh(struct vi_x86_time_dia_t *time_dia)
{
	GtkListStore *list_store;
	GtkTreeIter iter;

	int widget_width;
	int widget_height;

	int num_rows;
	int i;

	char str[MAX_STRING_SIZE];

	/* Get new dimensions */
	widget_width = gtk_widget_get_allocated_width(time_dia->widget);
	widget_height = gtk_widget_get_allocated_height(time_dia->widget);
	time_dia->widget_width = widget_width;
	time_dia->widget_height = widget_height;

	/* Clear */
	list_store = time_dia->list_store;
	gtk_list_store_clear(list_store);

	/* Add elements */
	num_rows = widget_height / VI_X86_TIME_DIA_CELL_HEIGHT;
	for (i = 0; i < num_rows; i++)
	{
		snprintf(str, sizeof str, "%d", i);
		gtk_list_store_append(list_store, &iter);
		gtk_list_store_set(list_store, &iter, 0, str, -1);
	}

	/* Print */
	printf("num_rows=%d, tree_view->height=%d\n", num_rows,
		gtk_widget_get_allocated_height(time_dia->tree_view));
	fflush(stdout);
}


void vi_x86_time_dia_go_to_cycle(struct vi_x86_time_dia_t *time_dia, long long cycle)
{
}

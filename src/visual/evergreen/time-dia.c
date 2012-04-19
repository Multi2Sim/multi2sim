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

#include <visual-evergreen.h>


#define VI_EVG_TIME_DIA_CELL_WIDTH			153
#define VI_EVG_TIME_DIA_CELL_HEIGHT			20

#define VI_EVG_TIME_DIA_FIRST_ROW_HEIGHT		20
#define VI_EVG_TIME_DIA_FIRST_COL_WIDTH			100

#define VI_EVG_TIME_DIA_FONT_SIZE			12

#define VI_EVG_TIME_DIA_CONTENT_LAYOUT_WIDTH		300
#define VI_EVG_TIME_DIA_CONTENT_LAYOUT_HEIGHT		200

#define VI_EVG_TIME_DIA_CYCLE_LAYOUT_WIDTH		VI_EVG_TIME_DIA_CONTENT_LAYOUT_WIDTH
#define VI_EVG_TIME_DIA_CYCLE_LAYOUT_HEIGHT		VI_EVG_TIME_DIA_CELL_HEIGHT

#define VI_EVG_TIME_DIA_INST_LAYOUT_WIDTH		150
#define VI_EVG_TIME_DIA_INST_LAYOUT_HEIGHT		VI_EVG_TIME_DIA_CONTENT_LAYOUT_HEIGHT

#define VI_EVG_TIME_DIA_LEGEND_LAYOUT_WIDTH		VI_EVG_TIME_DIA_INST_LAYOUT_WIDTH
#define VI_EVG_TIME_DIA_LEGEND_LAYOUT_HEIGHT		VI_EVG_TIME_DIA_CYCLE_LAYOUT_HEIGHT


struct vi_evg_time_dia_t
{
	GtkWidget *widget;

	GtkWidget *cycle_layout;
	GtkWidget *inst_layout;
	GtkWidget *content_layout;
	GtkWidget *legend_layout;

	GtkWidget *hscrollbar;
	GtkWidget *vscrollbar;

	int width;
	int height;

	struct vi_evg_compute_unit_t *compute_unit;
};


static void vi_evg_time_dia_widget_destroy(GtkWidget *widget, struct vi_evg_time_dia_t *time_dia)
{
	vi_evg_time_dia_free(time_dia);
}


static void vi_evg_time_dia_size_allocate(GtkWidget *widget, GdkRectangle *allocation,
	struct vi_evg_time_dia_t *time_dia)
{
	if (allocation->width != time_dia->width ||
		allocation->height != time_dia->height)
		vi_evg_time_dia_refresh(time_dia);
}


static void vi_evg_time_dia_scrollbar_value_changed(GtkRange *range,
	struct vi_evg_time_dia_t *time_dia)
{
	vi_evg_time_dia_refresh(time_dia);
}


struct vi_evg_time_dia_t *vi_evg_time_dia_create(struct vi_evg_compute_unit_t *compute_unit)
{
	struct vi_evg_time_dia_t *time_dia;

	/* Allocate */
	time_dia = calloc(1, sizeof(struct vi_evg_time_dia_t));
	if (!time_dia)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	time_dia->compute_unit = compute_unit;

	/* Content layout */
	GtkWidget *content_layout;
	content_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(content_layout, VI_EVG_TIME_DIA_CONTENT_LAYOUT_WIDTH,
		VI_EVG_TIME_DIA_CONTENT_LAYOUT_HEIGHT);
	g_signal_connect(G_OBJECT(content_layout), "size_allocate",
		G_CALLBACK(vi_evg_time_dia_size_allocate), time_dia);
	/*g_signal_connect(G_OBJECT(layout), "scroll-event",
		G_CALLBACK(vi_mod_widget_scroll), mod_widget);*/
	time_dia->content_layout = content_layout;

	/* Instruction layout */
	GtkWidget *inst_layout;
	inst_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(inst_layout, VI_EVG_TIME_DIA_INST_LAYOUT_WIDTH,
		VI_EVG_TIME_DIA_INST_LAYOUT_HEIGHT);
	time_dia->inst_layout = inst_layout;

	/* Cycle layout */
	GtkWidget *cycle_layout;
	cycle_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(cycle_layout, VI_EVG_TIME_DIA_CYCLE_LAYOUT_WIDTH,
		VI_EVG_TIME_DIA_CYCLE_LAYOUT_HEIGHT);
	time_dia->cycle_layout = cycle_layout;

	/* Legend layout */
	GtkWidget *legend_layout;
	legend_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(legend_layout, VI_EVG_TIME_DIA_LEGEND_LAYOUT_WIDTH,
		VI_EVG_TIME_DIA_LEGEND_LAYOUT_HEIGHT);
	time_dia->legend_layout = legend_layout;

	/* Left vertical box */
	GtkWidget *left_vbox;
	left_vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(left_vbox), legend_layout, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(left_vbox), inst_layout, TRUE, TRUE, 0);

	/* Right vertical box */
	GtkWidget *right_vbox;
	right_vbox = gtk_vbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), cycle_layout, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), content_layout, TRUE, TRUE, 0);

	/* Horizontal panel */
	GtkWidget *hpaned;
	hpaned = gtk_hpaned_new();
	gtk_paned_add1(GTK_PANED(hpaned), left_vbox);
	gtk_paned_add2(GTK_PANED(hpaned), right_vbox);

	/* Scroll bars */
	GtkWidget *hscrollbar = gtk_hscrollbar_new(NULL);
	GtkWidget *vscrollbar = gtk_vscrollbar_new(NULL);
	time_dia->hscrollbar = hscrollbar;
	time_dia->vscrollbar = vscrollbar;
	g_signal_connect(G_OBJECT(hscrollbar), "value-changed",
		G_CALLBACK(vi_evg_time_dia_scrollbar_value_changed), time_dia);
	g_signal_connect(G_OBJECT(vscrollbar), "value-changed",
		G_CALLBACK(vi_evg_time_dia_scrollbar_value_changed), time_dia);

	/* Table */
	GtkWidget *table;
	table = gtk_table_new(2, 2, FALSE);
	gtk_table_attach(GTK_TABLE(table), hpaned, 0, 1, 0, 1,
		GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
	gtk_table_attach(GTK_TABLE(table), hscrollbar, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(table), vscrollbar, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);

	/* Widget */
	time_dia->widget = table;
	g_signal_connect(G_OBJECT(time_dia->widget), "destroy",
		G_CALLBACK(vi_evg_time_dia_widget_destroy), time_dia);

	/* Return */
	return time_dia;
}


void vi_evg_time_dia_free(struct vi_evg_time_dia_t *time_dia)
{
	free(time_dia);
}


GtkWidget *vi_evg_time_dia_get_widget(struct vi_evg_time_dia_t *time_dia)
{
	return time_dia->widget;
}


void vi_evg_time_dia_refresh(struct vi_evg_time_dia_t *time_dia)
{
	long long cycle;
	long long num_cycles;

	long long table_width;

	GList *child;

	GtkWidget *content_layout;
	GtkWidget *cycle_layout;

	int width;
	int height;

	long long left;
	long long left_cycle;

	int left_cycle_offset;

	int x;

	char str[MAX_STRING_SIZE];


	/* Go to cycle */
	cycle = vi_cycle_bar_get_cycle();
	vi_state_go_to_cycle(cycle);

	/* Remove all objects from layouts */
	content_layout = time_dia->content_layout;
	cycle_layout = time_dia->cycle_layout;
	while ((child = gtk_container_get_children(GTK_CONTAINER(content_layout))))
		gtk_container_remove(GTK_CONTAINER(content_layout), child->data);
	while ((child = gtk_container_get_children(GTK_CONTAINER(cycle_layout))))
		gtk_container_remove(GTK_CONTAINER(cycle_layout), child->data);

	/* Get allocated dimensions */
	width = gtk_widget_get_allocated_width(content_layout);
	height = gtk_widget_get_allocated_height(content_layout);
	time_dia->width = width;
	time_dia->height = height;

	/* Dimensions */
	num_cycles = vi_state_get_num_cycles();
	table_width = VI_EVG_TIME_DIA_CELL_WIDTH * num_cycles;

	/* Horizontal scroll bar */
	if (table_width > width)
	{
		gtk_range_set_range(GTK_RANGE(time_dia->hscrollbar), 0, table_width - width);
		gtk_range_set_increments(GTK_RANGE(time_dia->hscrollbar),
			VI_EVG_TIME_DIA_CELL_WIDTH / 3, width - VI_EVG_TIME_DIA_CELL_WIDTH / 3);
		gtk_widget_set_visible(time_dia->hscrollbar, TRUE);
	}
	else
		gtk_widget_set_visible(time_dia->hscrollbar, FALSE);

	/* Get starting X position */
	left = gtk_range_get_value(GTK_RANGE(time_dia->hscrollbar));
	left_cycle = left / VI_EVG_TIME_DIA_CELL_WIDTH;
	left_cycle_offset = -(left % VI_EVG_TIME_DIA_CELL_WIDTH);

	/* Cycle layout */
	cycle = left_cycle;
	x = left_cycle_offset;
	while (x < width && cycle < num_cycles)
	{
		snprintf(str, sizeof str, "%lld", cycle);
		GtkWidget *label = gtk_label_new(str);
		gtk_widget_set_size_request(label, VI_EVG_TIME_DIA_CELL_WIDTH - 1,
			VI_EVG_TIME_DIA_CYCLE_LAYOUT_HEIGHT - 1);
		gtk_widget_show(label);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(VI_EVG_TIME_DIA_FONT_SIZE << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(event_box), label);
		gtk_layout_put(GTK_LAYOUT(cycle_layout), event_box, x, 0);
		gtk_widget_show(event_box);

		/* Color */
		GdkColor color;
		gdk_color_parse("#ffffff", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

		/* Next way */
		x += VI_EVG_TIME_DIA_CELL_WIDTH;
		cycle++;
	}

	/* Repaint if necessary */
	gtk_container_check_resize(GTK_CONTAINER(time_dia->legend_layout));
	gtk_container_check_resize(GTK_CONTAINER(time_dia->content_layout));
	gtk_container_check_resize(GTK_CONTAINER(time_dia->cycle_layout));
	gtk_container_check_resize(GTK_CONTAINER(time_dia->inst_layout));
}

/*
 *  Multi2Sim Tools
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

#include <gtk/gtk.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/matrix.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <visual/common/state.h>

#include "compute-unit.h"
#include "inst.h"
#include "time-dia.h"


#define VI_SI_TIME_DIA_CELL_WIDTH			70
#define VI_SI_TIME_DIA_CELL_HEIGHT			20

#define VI_SI_TIME_DIA_FIRST_ROW_HEIGHT			20
#define VI_SI_TIME_DIA_FIRST_COL_WIDTH			100

#define VI_SI_TIME_DIA_FONT_SIZE			12

#define VI_SI_TIME_DIA_CONTENT_LAYOUT_WIDTH		300
#define VI_SI_TIME_DIA_CONTENT_LAYOUT_HEIGHT		200

#define VI_SI_TIME_DIA_CYCLE_LAYOUT_WIDTH		VI_SI_TIME_DIA_CONTENT_LAYOUT_WIDTH
#define VI_SI_TIME_DIA_CYCLE_LAYOUT_HEIGHT		VI_SI_TIME_DIA_CELL_HEIGHT

#define VI_SI_TIME_DIA_INST_LAYOUT_WIDTH		150
#define VI_SI_TIME_DIA_INST_LAYOUT_HEIGHT		VI_SI_TIME_DIA_CONTENT_LAYOUT_HEIGHT

#define VI_SI_TIME_DIA_LEGEND_LAYOUT_WIDTH		VI_SI_TIME_DIA_INST_LAYOUT_WIDTH
#define VI_SI_TIME_DIA_LEGEND_LAYOUT_HEIGHT		VI_SI_TIME_DIA_CYCLE_LAYOUT_HEIGHT


struct vi_si_time_dia_t
{
	GtkWidget *widget;

	GtkWidget *legend_layout;

	GtkWidget *hscrollbar;
	GtkWidget *vscrollbar;

	GtkWidget *content_layout;
	GtkWidget *content_table;
	struct matrix_t *content_matrix;

	GtkWidget *inst_layout;
	GtkWidget *inst_table;
	struct list_t *inst_list;

	GtkWidget *cycle_layout;
	GtkWidget *cycle_table;
	struct list_t *cycle_list;

	int content_layout_width;
	int content_layout_height;

	int inst_layout_width;
	int inst_layout_height;

	long long left_cycle;
	int left_offset;

	long long top_inst;
	int top_offset;

	struct vi_si_compute_unit_t *compute_unit;
};


static void vi_si_time_dia_widget_destroy(GtkWidget *widget, struct vi_si_time_dia_t *time_dia)
{
	vi_si_time_dia_free(time_dia);
}


static void vi_si_time_dia_size_allocate(GtkWidget *widget, GdkRectangle *allocation,
	struct vi_si_time_dia_t *time_dia)
{
	vi_si_time_dia_refresh(time_dia);
}


static gboolean vi_si_time_dia_scroll(GtkWidget *widget, GdkEventScroll *event,
	struct vi_si_time_dia_t *time_dia)
{
	long long value;

	value = gtk_range_get_value(GTK_RANGE(time_dia->vscrollbar));
	if (event->direction == GDK_SCROLL_UP)
		value -= 10;
	else
		value += 10;
	gtk_range_set_value(GTK_RANGE(time_dia->vscrollbar), value);

	return FALSE;
}


static void vi_si_time_dia_scrollbar_value_changed(GtkRange *range,
	struct vi_si_time_dia_t *time_dia)
{
	vi_si_time_dia_refresh(time_dia);
}


static void vi_si_time_dia_refresh_scrollbars(struct vi_si_time_dia_t *time_dia)
{
	long long num_cycles;
	long long num_insts;

	int table_width;
	int table_height;

	/* Dimensions */
	num_cycles = vi_state_get_num_cycles();
	num_insts = time_dia->compute_unit->num_insts;
	table_width = VI_SI_TIME_DIA_CELL_WIDTH * num_cycles;
	table_height = VI_SI_TIME_DIA_CELL_HEIGHT * num_insts;

	/* Horizontal bar */
	if (table_width > time_dia->content_layout_width)
	{
		gtk_range_set_range(GTK_RANGE(time_dia->hscrollbar), 0,
			table_width - time_dia->content_layout_width);
		gtk_range_set_increments(GTK_RANGE(time_dia->hscrollbar),
			VI_SI_TIME_DIA_CELL_WIDTH / 3, time_dia->content_layout_width
			- VI_SI_TIME_DIA_CELL_WIDTH / 3);
		gtk_widget_set_visible(time_dia->hscrollbar, TRUE);
	}
	else
	{
		gtk_widget_set_visible(time_dia->hscrollbar, FALSE);
	}

	/* Vertical bar */
	if (table_height > time_dia->content_layout_height)
	{
		gtk_range_set_range(GTK_RANGE(time_dia->vscrollbar), 0,
			table_height - time_dia->content_layout_height);
		gtk_range_set_increments(GTK_RANGE(time_dia->vscrollbar),
			VI_SI_TIME_DIA_CELL_HEIGHT, time_dia->content_layout_height
			- VI_SI_TIME_DIA_CELL_HEIGHT);
		gtk_widget_set_visible(time_dia->vscrollbar, TRUE);
	}
	else
	{
		gtk_widget_set_visible(time_dia->vscrollbar, FALSE);
	}
}


static void vi_si_time_dia_refresh_content_layout(struct vi_si_time_dia_t *time_dia)
{
	GtkWidget *content_layout;
	GtkWidget *content_table;

	int num_rows;
	int num_cols;

	int row;
	int col;

	long long num_cycles;
	long long num_insts;

	/* Get new dimensions */
	num_cycles = vi_state_get_num_cycles();
	num_insts = time_dia->compute_unit->num_insts;
	num_rows = MIN(num_insts, time_dia->content_layout_height / VI_SI_TIME_DIA_CELL_HEIGHT + 2);
	num_cols = MIN(num_cycles, time_dia->content_layout_width / VI_SI_TIME_DIA_CELL_WIDTH + 2);

	/* Create new table */
	content_layout = time_dia->content_layout;
	if (time_dia->content_table)
		gtk_widget_destroy(time_dia->content_table);
	content_table = gtk_table_new(num_rows, num_cols, TRUE);
	gtk_layout_put(GTK_LAYOUT(content_layout), content_table,
		time_dia->left_offset, time_dia->top_offset);
	time_dia->content_table = content_table;

	/* Create new matrix */
	if (time_dia->content_matrix)
		matrix_free(time_dia->content_matrix);
	time_dia->content_matrix = matrix_create(num_rows, num_cols);

	/* Create labels */
	MATRIX_FOR_EACH(time_dia->content_matrix, row, col)
	{
		/* Label */
		GtkWidget *label = gtk_label_new(NULL);
		gtk_misc_set_alignment(GTK_MISC(label), .5, .5);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(VI_SI_TIME_DIA_FONT_SIZE << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(event_box), label);
		matrix_set(time_dia->content_matrix, row, col, event_box);

		/* Frame */
		GtkWidget *frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		gtk_container_add(GTK_CONTAINER(frame), event_box);
		gtk_table_attach_defaults(GTK_TABLE(content_table), frame, col, col + 1, row, row + 1);
		gtk_widget_set_size_request(frame, VI_SI_TIME_DIA_CELL_WIDTH,
			VI_SI_TIME_DIA_CELL_HEIGHT);

		/* Color */
		GdkColor color;
		gdk_color_parse("#ffffff", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
	}

	/* Show all widgets */
	gtk_widget_show_all(content_table);
	gtk_container_check_resize(GTK_CONTAINER(content_layout));
}


static void vi_si_time_dia_refresh_inst_layout(struct vi_si_time_dia_t *time_dia)
{
	GtkWidget *inst_layout;
	GtkWidget *inst_table;

	int num_rows;
	int row;

	long long num_insts;

	/* Get new dimensions */
	num_insts = time_dia->compute_unit->num_insts;
	num_rows = MIN(num_insts, time_dia->inst_layout_height / VI_SI_TIME_DIA_CELL_HEIGHT + 2);

	/* Create new table */
	inst_layout = time_dia->inst_layout;
	if (time_dia->inst_table)
		gtk_widget_destroy(time_dia->inst_table);
	inst_table = gtk_table_new(num_rows, 1, TRUE);
	gtk_layout_put(GTK_LAYOUT(inst_layout), inst_table, 0, time_dia->top_offset);
	time_dia->inst_table = inst_table;

	/* Clear instruction label list */
	if (!time_dia->inst_list)
		time_dia->inst_list = list_create();
	list_clear(time_dia->inst_list);

	/* Create labels */
	for (row = 0; row < num_rows; row++)
	{
		/* Label */
		GtkWidget *label = gtk_label_new(NULL);
		gtk_misc_set_alignment(GTK_MISC(label), 0, .5);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(VI_SI_TIME_DIA_FONT_SIZE << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box = gtk_event_box_new();
		gtk_widget_set_size_request(event_box, time_dia->inst_layout_width,
			VI_SI_TIME_DIA_CELL_HEIGHT);
		gtk_container_add(GTK_CONTAINER(event_box), label);
		gtk_table_attach_defaults(GTK_TABLE(inst_table), event_box, 0, 1, row, row + 1);
		list_add(time_dia->inst_list, event_box);

		/* Color */
		GdkColor color;
		gdk_color_parse("#ffffff", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
	}

	/* Show all widgets */
	gtk_widget_show_all(inst_table);
	gtk_container_check_resize(GTK_CONTAINER(inst_layout));
}


static void vi_si_time_dia_refresh_cycle_layout(struct vi_si_time_dia_t *time_dia)
{
	GtkWidget *cycle_layout;
	GtkWidget *cycle_table;

	int num_cols;
	int col;

	long long num_cycles;

	/* Get new dimensions */
	num_cycles = vi_state_get_num_cycles();
	num_cols = MIN(num_cycles, time_dia->content_layout_width / VI_SI_TIME_DIA_CELL_WIDTH + 2);

	/* Create new table */
	cycle_layout = time_dia->cycle_layout;
	if (time_dia->cycle_table)
		gtk_widget_destroy(time_dia->cycle_table);
	cycle_table = gtk_table_new(1, num_cols, TRUE);
	gtk_layout_put(GTK_LAYOUT(cycle_layout), cycle_table, time_dia->left_offset, 0);
	time_dia->cycle_table = cycle_table;

	/* Clear cycle label list */
	if (!time_dia->cycle_list)
		time_dia->cycle_list = list_create();
	list_clear(time_dia->cycle_list);

	/* Create labels */
	for (col = 0; col < num_cols; col++)
	{
		/* Label */
		GtkWidget *label = gtk_label_new(NULL);
		gtk_misc_set_alignment(GTK_MISC(label), .5, .5);
		gtk_widget_set_size_request(label, VI_SI_TIME_DIA_CELL_WIDTH - 1,
			VI_SI_TIME_DIA_CELL_HEIGHT - 1);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(VI_SI_TIME_DIA_FONT_SIZE << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(event_box), label);
		gtk_table_attach_defaults(GTK_TABLE(cycle_table), event_box, col, col + 1, 0, 1);
		list_add(time_dia->cycle_list, event_box);

		/* Color */
		GdkColor color;
		gdk_color_parse("#ffffff", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
	}

	/* Show all widgets */
	gtk_widget_show_all(cycle_table);
	gtk_container_check_resize(GTK_CONTAINER(cycle_layout));
}


static void vi_si_time_dia_refresh_content(struct vi_si_time_dia_t *time_dia)
{
	struct vi_si_compute_unit_t *compute_unit;
	struct vi_si_inst_t *inst;

	long long left_cycle;
	long long top_inst;

	int row;
	int col;

	char str[MAX_LONG_STRING_SIZE];

	char *inst_name;


	/* Get compute unit */
	compute_unit = time_dia->compute_unit;

	/* Get starting positions */
	left_cycle = time_dia->left_cycle;
	top_inst = time_dia->top_inst;

	/* Cycle layout */
	LIST_FOR_EACH(time_dia->cycle_list, col)
	{
		/* Event box */
		GtkWidget *event_box;
		event_box = list_get(time_dia->cycle_list, col);

		/* Label */
		GtkWidget *label;
		label = gtk_bin_get_child(GTK_BIN(event_box));

		/* Text */
		snprintf(str, sizeof str, "%lld", left_cycle + col);
		gtk_label_set_text(GTK_LABEL(label), str);
	}

	/* Instruction layout */
	LIST_FOR_EACH(time_dia->inst_list, row)
	{
		/* Event box */
		GtkWidget *event_box;
		event_box = list_get(time_dia->inst_list, row);

		/* Label */
		GtkWidget *label;
		label = gtk_bin_get_child(GTK_BIN(event_box));
		gtk_label_set_text(GTK_LABEL(label), NULL);
	}

	/* Content layout */
	MATRIX_FOR_EACH_COLUMN(time_dia->content_matrix, col)
	{
		/* Go to cycle */
		vi_state_go_to_cycle(left_cycle + col);

		/* Add in-flight instructions */
		HASH_TABLE_FOR_EACH(compute_unit->inst_table, inst_name, inst)
		{
			int inst_row;

			/* Instruction is in range */
			inst_row = inst->id - top_inst;
			if (!IN_RANGE(inst_row, 0, list_count(time_dia->inst_list) - 1))
				continue;

			/* Event box */
			GtkWidget *event_box;
			event_box = list_get(time_dia->inst_list, inst_row);

			/* Label */
			GtkWidget *label;
			label = gtk_bin_get_child(GTK_BIN(event_box));

			/* Instruction */
			vi_si_inst_get_markup(inst, str, sizeof str);
			gtk_label_set_markup(GTK_LABEL(label), str);
		}

		/* Rows */
		MATRIX_FOR_EACH_ROW(time_dia->content_matrix, row)
		{
			/* Get instruction */
			snprintf(str, sizeof str, "i-%lld", top_inst + row);
			inst = hash_table_get(compute_unit->inst_table, str);

			/* Event box */
			GtkWidget *event_box;
			event_box = matrix_get(time_dia->content_matrix, row, col);

			/* Label */
			GtkWidget *label;
			label = gtk_bin_get_child(GTK_BIN(event_box));
			if (inst)
				gtk_label_set_markup(GTK_LABEL(label),
					str_map_value(&vi_si_inst_stage_name_map, inst->stage));
			else
				gtk_label_set_text(GTK_LABEL(label), NULL);

			/* Color */
			GdkColor color;
			char *color_str;
			color_str = inst ? str_map_value(&vi_si_inst_stage_color_map, inst->stage) : "white";
			gdk_color_parse(color_str, &color);
			gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
		}
	}
}




/*
 * Public Functions
 */

struct vi_si_time_dia_t *vi_si_time_dia_create(struct vi_si_compute_unit_t *compute_unit)
{
	struct vi_si_time_dia_t *time_dia;

	/* Initialize */
	time_dia = xcalloc(1, sizeof(struct vi_si_time_dia_t));
	time_dia->compute_unit = compute_unit;

	/* Content layout */
	GtkWidget *content_layout;
	content_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(content_layout, VI_SI_TIME_DIA_CONTENT_LAYOUT_WIDTH,
		VI_SI_TIME_DIA_CONTENT_LAYOUT_HEIGHT);
	g_signal_connect(G_OBJECT(content_layout), "size_allocate",
		G_CALLBACK(vi_si_time_dia_size_allocate), time_dia);
	g_signal_connect(G_OBJECT(content_layout), "scroll-event",
		G_CALLBACK(vi_si_time_dia_scroll), time_dia);
	time_dia->content_layout = content_layout;

	/* Instruction layout */
	GtkWidget *inst_layout;
	inst_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(inst_layout, VI_SI_TIME_DIA_INST_LAYOUT_WIDTH,
		VI_SI_TIME_DIA_INST_LAYOUT_HEIGHT);
	g_signal_connect(G_OBJECT(inst_layout), "size_allocate",
		G_CALLBACK(vi_si_time_dia_size_allocate), time_dia);
	time_dia->inst_layout = inst_layout;

	/* Cycle layout */
	GtkWidget *cycle_layout;
	cycle_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(cycle_layout, VI_SI_TIME_DIA_CYCLE_LAYOUT_WIDTH,
		VI_SI_TIME_DIA_CYCLE_LAYOUT_HEIGHT);
	time_dia->cycle_layout = cycle_layout;

	/* Legend layout */
	GtkWidget *legend_layout;
	legend_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(legend_layout, VI_SI_TIME_DIA_LEGEND_LAYOUT_WIDTH,
		VI_SI_TIME_DIA_LEGEND_LAYOUT_HEIGHT);
	time_dia->legend_layout = legend_layout;

	/* Left vertical box */
	GtkWidget *left_vbox;
	left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(left_vbox), legend_layout, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(left_vbox), inst_layout, TRUE, TRUE, 0);

	/* Right vertical box */
	GtkWidget *right_vbox;
	right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), cycle_layout, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), content_layout, TRUE, TRUE, 0);

	/* Horizontal panel */
	GtkWidget *hpaned;
	hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_pack1(GTK_PANED(hpaned), left_vbox, TRUE, FALSE);
	gtk_paned_pack2(GTK_PANED(hpaned), right_vbox, TRUE, TRUE);

	/* Scroll bars */
	GtkWidget *hscrollbar = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, NULL);
	GtkWidget *vscrollbar = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, NULL);
	time_dia->hscrollbar = hscrollbar;
	time_dia->vscrollbar = vscrollbar;
	g_signal_connect(G_OBJECT(hscrollbar), "value-changed",
		G_CALLBACK(vi_si_time_dia_scrollbar_value_changed), time_dia);
	g_signal_connect(G_OBJECT(vscrollbar), "value-changed",
		G_CALLBACK(vi_si_time_dia_scrollbar_value_changed), time_dia);

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
		G_CALLBACK(vi_si_time_dia_widget_destroy), time_dia);

	/* Return */
	return time_dia;
}


void vi_si_time_dia_free(struct vi_si_time_dia_t *time_dia)
{
	/* Content label matrix */
	if (time_dia->content_matrix)
		matrix_free(time_dia->content_matrix);

	/* Instruction label list */
	if (time_dia->inst_list)
		list_free(time_dia->inst_list);

	/* Cycle label list */
	if (time_dia->cycle_list)
		list_free(time_dia->cycle_list);

	/* Time diagram */
	free(time_dia);
}


GtkWidget *vi_si_time_dia_get_widget(struct vi_si_time_dia_t *time_dia)
{
	return time_dia->widget;
}


void vi_si_time_dia_go_to_cycle(struct vi_si_time_dia_t *time_dia, long long cycle)
{
	struct vi_si_compute_unit_t *compute_unit;
	struct vi_si_inst_t *inst;

	char *inst_name;

	long long top_inst;

	/* Bring state to cycle */
	vi_state_go_to_cycle(cycle);

	/* Get top instruction */
	compute_unit = time_dia->compute_unit;
	top_inst = MAX(0, compute_unit->num_insts - 1);
	HASH_TABLE_FOR_EACH(compute_unit->inst_table, inst_name, inst)
		top_inst = MIN(top_inst, inst->id);
	if (!hash_table_count(compute_unit->inst_table))
		top_inst = 0;

	/* Set scroll bar values */
	gtk_range_set_value(GTK_RANGE(time_dia->hscrollbar), cycle * VI_SI_TIME_DIA_CELL_WIDTH);
	gtk_range_set_value(GTK_RANGE(time_dia->vscrollbar), top_inst * VI_SI_TIME_DIA_CELL_HEIGHT);
}


void vi_si_time_dia_refresh(struct vi_si_time_dia_t *time_dia)
{
	int content_layout_width;
	int content_layout_height;
	int inst_layout_width;
	int inst_layout_height;

	long long left;
	long long left_cycle;
	int left_offset;

	long long top;
	long long top_inst_id;
	int top_offset;

	int content_layout_width_changed;
	int content_layout_height_changed;
	int inst_layout_width_changed;
	int inst_layout_height_changed;
	int left_cycle_changed;
	int left_offset_changed;
	int top_inst_changed;
	int top_offset_changed;

	/* Get new state */
	content_layout_width = gtk_widget_get_allocated_width(time_dia->content_layout);
	content_layout_height = gtk_widget_get_allocated_height(time_dia->content_layout);
	inst_layout_width = gtk_widget_get_allocated_width(time_dia->inst_layout);
	inst_layout_height = gtk_widget_get_allocated_height(time_dia->inst_layout);
	left = gtk_range_get_value(GTK_RANGE(time_dia->hscrollbar));
	left_cycle = left / VI_SI_TIME_DIA_CELL_WIDTH;
	left_offset = -(left % VI_SI_TIME_DIA_CELL_WIDTH);
	top = gtk_range_get_value(GTK_RANGE(time_dia->vscrollbar));
	top_inst_id = top / VI_SI_TIME_DIA_CELL_HEIGHT;
	top_offset = -(top % VI_SI_TIME_DIA_CELL_HEIGHT);

	/* Record changes */
	content_layout_width_changed = content_layout_width != time_dia->content_layout_width;
	content_layout_height_changed = content_layout_height != time_dia->content_layout_height;
	inst_layout_width_changed = inst_layout_width != time_dia->inst_layout_width;
	inst_layout_height_changed = inst_layout_height != time_dia->inst_layout_height;
	left_cycle_changed = left_cycle != time_dia->left_cycle;
	left_offset_changed = left_offset != time_dia->left_offset;
	top_inst_changed = top_inst_id != time_dia->top_inst;
	top_offset_changed = top_offset != time_dia->top_offset;

	/* Save new state */
	time_dia->content_layout_width = content_layout_width;
	time_dia->content_layout_height = content_layout_height;
	time_dia->inst_layout_width = inst_layout_width;
	time_dia->inst_layout_height = inst_layout_height;
	time_dia->left_cycle = left_cycle;
	time_dia->left_offset = left_offset;
	time_dia->top_inst = top_inst_id;
	time_dia->top_offset = top_offset;

	/* Refresh scroll bars */
	if (content_layout_width_changed || content_layout_height_changed)
		vi_si_time_dia_refresh_scrollbars(time_dia);

	/* Refresh content layout */
	if (content_layout_width_changed || content_layout_height_changed)
		vi_si_time_dia_refresh_content_layout(time_dia);

	/* Refresh content layout position */
	if (left_offset_changed || top_offset_changed)
		gtk_layout_move(GTK_LAYOUT(time_dia->content_layout), time_dia->content_table,
			time_dia->left_offset, time_dia->top_offset);

	/* Refresh instruction layout */
	if (inst_layout_width_changed || inst_layout_height_changed)
		vi_si_time_dia_refresh_inst_layout(time_dia);

	/* Refresh instruction layout position */
	if (top_offset_changed)
		gtk_layout_move(GTK_LAYOUT(time_dia->inst_layout), time_dia->inst_table,
			0, time_dia->top_offset);

	/* Refresh cycle layout */
	if (content_layout_width_changed)
		vi_si_time_dia_refresh_cycle_layout(time_dia);

	/* Refresh cycle layout position */
	if (left_offset_changed)
		gtk_layout_move(GTK_LAYOUT(time_dia->cycle_layout), time_dia->cycle_table,
			time_dia->left_offset, 0);

	/* Refresh content */
	if (content_layout_width_changed || content_layout_height_changed
		|| inst_layout_width_changed || inst_layout_height_changed
		|| left_cycle_changed || left_offset_changed || top_inst_changed
		|| top_offset_changed)
		vi_si_time_dia_refresh_content(time_dia);
}


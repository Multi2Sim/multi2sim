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


#define VI_EVG_TIME_DIA_CELL_WIDTH			70
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

	int content_layout_width;
	int content_layout_height;

	int inst_layout_width;
	int inst_layout_height;

	struct vi_evg_compute_unit_t *compute_unit;
};


static void vi_evg_time_dia_widget_destroy(GtkWidget *widget, struct vi_evg_time_dia_t *time_dia)
{
	vi_evg_time_dia_free(time_dia);
}


static void vi_evg_time_dia_size_allocate(GtkWidget *widget, GdkRectangle *allocation,
	struct vi_evg_time_dia_t *time_dia)
{
	int content_layout_width;
	int content_layout_height;

	int inst_layout_width;
	int inst_layout_height;

	content_layout_width = gtk_widget_get_allocated_width(time_dia->content_layout);
	content_layout_height = gtk_widget_get_allocated_height(time_dia->content_layout);

	inst_layout_width = gtk_widget_get_allocated_width(time_dia->inst_layout);
	inst_layout_height = gtk_widget_get_allocated_height(time_dia->inst_layout);

	if (content_layout_width != time_dia->content_layout_width ||
		content_layout_height != time_dia->content_layout_height ||
		inst_layout_width != time_dia->inst_layout_width ||
		inst_layout_height != time_dia->inst_layout_height)
		vi_evg_time_dia_refresh(time_dia);
}


static gboolean vi_evg_time_dia_scroll(GtkWidget *widget, GdkEventScroll *event,
	struct vi_evg_time_dia_t *time_dia)
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

	/* Colors */
	GdkColor color_gray;
	gdk_color_parse("#aaaaaa", &color_gray);

	/* Content layout */
	GtkWidget *content_layout;
	content_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(content_layout, VI_EVG_TIME_DIA_CONTENT_LAYOUT_WIDTH,
		VI_EVG_TIME_DIA_CONTENT_LAYOUT_HEIGHT);
	g_signal_connect(G_OBJECT(content_layout), "size_allocate",
		G_CALLBACK(vi_evg_time_dia_size_allocate), time_dia);
	g_signal_connect(G_OBJECT(content_layout), "scroll-event",
		G_CALLBACK(vi_evg_time_dia_scroll), time_dia);
	gtk_widget_modify_bg(content_layout, GTK_STATE_NORMAL, &color_gray);
	time_dia->content_layout = content_layout;

	/* Instruction layout */
	GtkWidget *inst_layout;
	inst_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(inst_layout, VI_EVG_TIME_DIA_INST_LAYOUT_WIDTH,
		VI_EVG_TIME_DIA_INST_LAYOUT_HEIGHT);
	g_signal_connect(G_OBJECT(inst_layout), "size_allocate",
		G_CALLBACK(vi_evg_time_dia_size_allocate), time_dia);
	gtk_widget_modify_bg(inst_layout, GTK_STATE_NORMAL, &color_gray);
	time_dia->inst_layout = inst_layout;

	/* Cycle layout */
	GtkWidget *cycle_layout;
	cycle_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(cycle_layout, VI_EVG_TIME_DIA_CYCLE_LAYOUT_WIDTH,
		VI_EVG_TIME_DIA_CYCLE_LAYOUT_HEIGHT);
	gtk_widget_modify_bg(cycle_layout, GTK_STATE_NORMAL, &color_gray);
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
	gtk_paned_pack1(GTK_PANED(hpaned), left_vbox, TRUE, FALSE);
	gtk_paned_pack2(GTK_PANED(hpaned), right_vbox, TRUE, TRUE);

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


void vi_evg_time_dia_go_to_cycle(struct vi_evg_time_dia_t *time_dia, long long cycle)
{
	struct vi_evg_compute_unit_t *compute_unit;
	struct vi_evg_inst_t *inst;

	char *inst_name;

	long long min_inst_id;

	/* Bring state to cycle */
	vi_state_go_to_cycle(cycle);

	/* Horizontal scroll bar */
	gtk_range_set_value(GTK_RANGE(time_dia->hscrollbar), cycle * VI_EVG_TIME_DIA_CELL_WIDTH);

	/* Vertical scroll bar */
	compute_unit = time_dia->compute_unit;
	min_inst_id = MAX(0, compute_unit->num_insts - 1);
	HASH_TABLE_FOR_EACH(compute_unit->inst_table, inst_name, inst)
		min_inst_id = MIN(min_inst_id, inst->id);
	gtk_range_set_value(GTK_RANGE(time_dia->vscrollbar), min_inst_id * VI_EVG_TIME_DIA_CELL_HEIGHT);
}


void vi_evg_time_dia_refresh(struct vi_evg_time_dia_t *time_dia)
{
	long long cycle;
	long long inst_id;

	long long num_cycles;
	long long num_insts;

	long long table_width;
	long long table_height;

	GList *child;

	struct vi_evg_compute_unit_t *compute_unit;
	struct vi_evg_inst_t *inst;

	GtkWidget *content_layout;
	GtkWidget *cycle_layout;
	GtkWidget *inst_layout;

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

	int x;
	int y;

	char str[MAX_LONG_STRING_SIZE];

	char *inst_name;
	char *inst_label_markup;

	struct hash_table_t *inst_table;


	/* Get compute unit */
	compute_unit = time_dia->compute_unit;

	/* Remove all objects from layouts */
	content_layout = time_dia->content_layout;
	cycle_layout = time_dia->cycle_layout;
	inst_layout = time_dia->inst_layout;
	while ((child = gtk_container_get_children(GTK_CONTAINER(content_layout))))
		gtk_container_remove(GTK_CONTAINER(content_layout), child->data);
	while ((child = gtk_container_get_children(GTK_CONTAINER(cycle_layout))))
		gtk_container_remove(GTK_CONTAINER(cycle_layout), child->data);
	while ((child = gtk_container_get_children(GTK_CONTAINER(inst_layout))))
		gtk_container_remove(GTK_CONTAINER(inst_layout), child->data);

	/* Get allocated dimensions */
	content_layout_width = gtk_widget_get_allocated_width(content_layout);
	content_layout_height = gtk_widget_get_allocated_height(content_layout);
	time_dia->content_layout_width = content_layout_width;
	time_dia->content_layout_height = content_layout_height;
	inst_layout_width = gtk_widget_get_allocated_width(inst_layout);
	inst_layout_height = gtk_widget_get_allocated_height(inst_layout);
	time_dia->inst_layout_width = inst_layout_width;
	time_dia->inst_layout_height = inst_layout_height;

	/* Dimensions */
	num_cycles = vi_state_get_num_cycles();
	num_insts = time_dia->compute_unit->num_insts;
	table_width = VI_EVG_TIME_DIA_CELL_WIDTH * num_cycles;
	table_height = VI_EVG_TIME_DIA_CELL_HEIGHT * num_insts;

	/* Horizontal scroll bar */
	if (table_width > content_layout_width)
	{
		gtk_range_set_range(GTK_RANGE(time_dia->hscrollbar), 0,
			table_width - content_layout_width);
		gtk_range_set_increments(GTK_RANGE(time_dia->hscrollbar),
			VI_EVG_TIME_DIA_CELL_WIDTH / 3, content_layout_width
			- VI_EVG_TIME_DIA_CELL_WIDTH / 3);
		gtk_widget_set_visible(time_dia->hscrollbar, TRUE);
	}
	else
		gtk_widget_set_visible(time_dia->hscrollbar, FALSE);

	/* Vertical scroll bar */
	if (table_height > content_layout_height)
	{
		gtk_range_set_range(GTK_RANGE(time_dia->vscrollbar), 0,
			table_height - content_layout_height);
		gtk_range_set_increments(GTK_RANGE(time_dia->vscrollbar),
			VI_EVG_TIME_DIA_CELL_HEIGHT, content_layout_height
			- VI_EVG_TIME_DIA_CELL_HEIGHT);
		gtk_widget_set_visible(time_dia->vscrollbar, TRUE);
	}
	else
		gtk_widget_set_visible(time_dia->vscrollbar, FALSE);

	/* Get starting X position */
	left = gtk_range_get_value(GTK_RANGE(time_dia->hscrollbar));
	left_cycle = left / VI_EVG_TIME_DIA_CELL_WIDTH;
	left_offset = -(left % VI_EVG_TIME_DIA_CELL_WIDTH);

	/* Get starting Y position */
	top = gtk_range_get_value(GTK_RANGE(time_dia->vscrollbar));
	top_inst_id = top / VI_EVG_TIME_DIA_CELL_HEIGHT;
	top_offset = -(top % VI_EVG_TIME_DIA_CELL_HEIGHT);

	/* Create hash table with in-flight instructions */
	inst_table = hash_table_create(0, FALSE);

	/* Cycle layout */
	cycle = left_cycle;
	x = left_offset;
	while (x < content_layout_width && cycle < num_cycles)
	{
		snprintf(str, sizeof str, "%lld", cycle);
		GtkWidget *label = gtk_label_new(str);
		gtk_widget_set_size_request(label, VI_EVG_TIME_DIA_CELL_WIDTH - 1,
			VI_EVG_TIME_DIA_CYCLE_LAYOUT_HEIGHT - 1);

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

		/* Color */
		GdkColor color;
		gdk_color_parse("#ffffff", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

		/* Next cycle */
		x += VI_EVG_TIME_DIA_CELL_WIDTH;
		cycle++;
	}

	/* Content layout */
	cycle = left_cycle;
	x = left_offset;
	while (x < content_layout_width && cycle < num_cycles)
	{
		/* Go to cycle */
		vi_state_go_to_cycle(cycle);

		/* Add in-flight instructions */
		HASH_TABLE_FOR_EACH(compute_unit->inst_table, inst_name, inst)
		{
			/* Instruction has already been added */
			if (hash_table_get(inst_table, inst_name))
				continue;

			/* Add instruction */
			vi_evg_inst_get_markup(inst, str, sizeof str);
			inst_label_markup = strdup(str);
			if (!inst_label_markup)
				fatal("%s: out of memory", __FUNCTION__);
			hash_table_insert(inst_table, inst_name, inst_label_markup);
		}

		/* Rows */
		inst_id = top_inst_id;
		y = top_offset;
		while (y < content_layout_height && inst_id < num_insts)
		{
			/* Get instruction */
			snprintf(str, sizeof str, "i-%lld", inst_id);
			inst = hash_table_get(compute_unit->inst_table, str);

			/* Label */
			GtkWidget *label = gtk_label_new(NULL);
			gtk_widget_set_size_request(label, VI_EVG_TIME_DIA_CELL_WIDTH - 1,
				VI_EVG_TIME_DIA_CELL_HEIGHT - 1);
			if (inst)
				gtk_label_set_markup(GTK_LABEL(label),
					map_value(&vi_evg_inst_stage_name_map, inst->stage));

			/* Set label font attributes */
			PangoAttrList *attrs;
			attrs = pango_attr_list_new();
			PangoAttribute *size_attr = pango_attr_size_new_absolute(VI_EVG_TIME_DIA_FONT_SIZE << 10);
			pango_attr_list_insert(attrs, size_attr);
			gtk_label_set_attributes(GTK_LABEL(label), attrs);

			/* Event box */
			GtkWidget *event_box = gtk_event_box_new();
			gtk_container_add(GTK_CONTAINER(event_box), label);
			gtk_layout_put(GTK_LAYOUT(content_layout), event_box, x, y);

			/* Color */
			GdkColor color;
			char *color_str;
			color_str = inst ? map_value(&vi_evg_inst_stage_color_map, inst->stage) : "white";
			gdk_color_parse(color_str, &color);
			gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

			/* Next instruction */
			y += VI_EVG_TIME_DIA_CELL_HEIGHT;
			inst_id++;
		}

		/* Next cycle */
		x += VI_EVG_TIME_DIA_CELL_WIDTH;
		cycle++;
	}

	/* Instruction layout */
	inst_id = top_inst_id;
	y = top_offset;
	while (y < content_layout_height && inst_id < num_insts)
	{
		/* Get instruction */
		snprintf(str, sizeof str, "i-%lld", inst_id);
		inst_label_markup = hash_table_get(inst_table, str);

		/* Label */
		GtkWidget *label = gtk_label_new(NULL);
		if (inst_label_markup)
			gtk_label_set_markup(GTK_LABEL(label), inst_label_markup);
		gtk_misc_set_alignment(GTK_MISC(label), 0, .5);
		gtk_widget_set_size_request(label, inst_layout_width,
			VI_EVG_TIME_DIA_CELL_HEIGHT - 1);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(VI_EVG_TIME_DIA_FONT_SIZE << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(event_box), label);
		gtk_layout_put(GTK_LAYOUT(inst_layout), event_box, 0, y);

		/* Color */
		GdkColor color;
		gdk_color_parse("#ffffff", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

		/* Next instruction */
		y += VI_EVG_TIME_DIA_CELL_HEIGHT;
		inst_id++;
	}

	/* Free hash table of instructions */
	HASH_TABLE_FOR_EACH(inst_table, inst_name, inst_label_markup)
		free(inst_label_markup);
	hash_table_free(inst_table);

	/* Repaint if necessary */
	gtk_widget_show_all(time_dia->legend_layout);
	gtk_widget_show_all(time_dia->content_layout);
	gtk_widget_show_all(time_dia->cycle_layout);
	gtk_widget_show_all(time_dia->inst_layout);
	gtk_container_check_resize(GTK_CONTAINER(time_dia->legend_layout));
	gtk_container_check_resize(GTK_CONTAINER(time_dia->content_layout));
	gtk_container_check_resize(GTK_CONTAINER(time_dia->cycle_layout));
	gtk_container_check_resize(GTK_CONTAINER(time_dia->inst_layout));
}

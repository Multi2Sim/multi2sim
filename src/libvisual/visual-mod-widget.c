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

#include <visual-private.h>


#define VISUAL_MOD_CELL_WIDTH			153
#define VISUAL_MOD_CELL_HEIGHT			20

#define VISUAL_MOD_FIRST_ROW_HEIGHT		20
#define VISUAL_MOD_FIRST_COL_WIDTH		100

#define VISUAL_MOD_FONT_SIZE			12

#define VISUAL_MOD_LABEL_BLOCK_LEFT		0
#define VISUAL_MOD_LABEL_BLOCK_WIDTH		100

#define VISUAL_MOD_LABEL_SHARERS_LEFT		101
#define VISUAL_MOD_LABEL_SHARERS_WIDTH		25

#define VISUAL_MOD_LABEL_ACCESSES_LEFT		127
#define VISUAL_MOD_LABEL_ACCESSES_WIDTH		25


static struct string_map_t visual_mod_block_state_map =
{
	6, {
		{ "I", 0 },
		{ "M", 1 },
		{ "O", 2 },
		{ "E", 3 },
		{ "S", 4 },
		{ "N", 5 }
	}
};


#define VISUAL_MOD_NUM_BLOCK_COLORS  6

static char *visual_mod_block_color[VISUAL_MOD_NUM_BLOCK_COLORS] =
{
	"#eeeeee",	/* light gray */
	"#33ccff",	/* blue */
	"#00ff33",	/* green */
	"#ffff33",	/* yellow */
	"#ff9900",	/* orange */
	"#cc99ff"	/* purple */
};


struct visual_mod_widget_t
{
	char *name;

	GtkWidget *widget;
	GtkWidget *hscrollbar;
	GtkWidget *vscrollbar;

	GtkWidget *layout;
	GtkWidget *first_row_layout;
	GtkWidget *first_col_layout;

	struct vlist_t *access_list;

	int width;
	int height;
};


static void visual_mod_widget_destroy(GtkWidget *widget,
	struct visual_mod_widget_t *visual_mod_widget)
{
	visual_mod_widget_free(visual_mod_widget);
}


static void visual_mod_widget_size_allocate(GtkWidget *widget, GdkRectangle *allocation,
	struct visual_mod_widget_t *visual_mod_widget)
{
	if (allocation->width != visual_mod_widget->width ||
		allocation->height != visual_mod_widget->height)
		visual_mod_widget_refresh(visual_mod_widget);
}


static gboolean visual_mod_widget_scroll(GtkWidget *widget, GdkEventScroll *event,
	struct visual_mod_widget_t *visual_mod_widget)
{
	int value;

	value = gtk_range_get_value(GTK_RANGE(visual_mod_widget->vscrollbar));
	if (event->direction == GDK_SCROLL_UP)
		value -= 10;
	else
		value += 10;
	gtk_range_set_value(GTK_RANGE(visual_mod_widget->vscrollbar), value);
	return FALSE;
}


static void visual_mod_widget_scroll_bar_value_changed(GtkRange *range,
	struct visual_mod_widget_t *visual_mod_widget)
{
	visual_mod_widget_refresh(visual_mod_widget);
}


struct visual_mod_widget_t *visual_mod_widget_create(char *name)
{
	struct visual_mod_widget_t *visual_mod_widget;

	/* Allocate */
	visual_mod_widget = calloc(1, sizeof(struct visual_mod_widget_t));
	if (!visual_mod_widget)
		fatal("%s: out of memory", __FUNCTION__);

	/* Name */
	visual_mod_widget->name = strdup(name);
	if (!visual_mod_widget->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Vertical box */
	GtkWidget *vbox;
	vbox = gtk_vbox_new(FALSE, 0);

	/* Access list */
	struct vlist_t *access_list;
	access_list = vlist_create("Access list", 200, 30,
		(vlist_get_elem_name_func_t) visual_mod_access_get_name_buf,
		(vlist_get_elem_desc_func_t) visual_mod_access_get_desc_buf);
	gtk_box_pack_start(GTK_BOX(vbox), vlist_get_widget(access_list), FALSE, FALSE, 0);
	visual_mod_widget->access_list = access_list;

	/* Scroll bars */
	GtkWidget *hscrollbar = gtk_hscrollbar_new(NULL);
	GtkWidget *vscrollbar = gtk_vscrollbar_new(NULL);
	visual_mod_widget->hscrollbar = hscrollbar;
	visual_mod_widget->vscrollbar = vscrollbar;
	g_signal_connect(G_OBJECT(hscrollbar), "value-changed",
		G_CALLBACK(visual_mod_widget_scroll_bar_value_changed), visual_mod_widget);
	g_signal_connect(G_OBJECT(vscrollbar), "value-changed",
		G_CALLBACK(visual_mod_widget_scroll_bar_value_changed), visual_mod_widget);

	/* Colors */
	GdkColor color_gray;
	gdk_color_parse("#aaaaaa", &color_gray);

	/* Layout */
	GtkWidget *layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(layout, 200, 100);
	g_signal_connect(G_OBJECT(layout), "size_allocate",
		G_CALLBACK(visual_mod_widget_size_allocate), visual_mod_widget);
	g_signal_connect(G_OBJECT(layout), "scroll-event",
		G_CALLBACK(visual_mod_widget_scroll), visual_mod_widget);
	gtk_widget_modify_bg(layout, GTK_STATE_NORMAL, &color_gray);
	visual_mod_widget->layout = layout;

	/* First row layout */
	GtkWidget *first_row_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(first_row_layout, -1, VISUAL_MOD_FIRST_ROW_HEIGHT);
	gtk_widget_modify_bg(first_row_layout, GTK_STATE_NORMAL, &color_gray);
	visual_mod_widget->first_row_layout = first_row_layout;

	/* First column layout */
	GtkWidget *first_col_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(first_col_layout, VISUAL_MOD_FIRST_COL_WIDTH, -1);
	gtk_widget_modify_bg(first_col_layout, GTK_STATE_NORMAL, &color_gray);
	visual_mod_widget->first_col_layout = first_col_layout;

	/* Top-left label */
	GtkWidget *top_left_label = gtk_label_new(name);
	gtk_widget_set_size_request(top_left_label, VISUAL_MOD_FIRST_COL_WIDTH - 1,
		VISUAL_MOD_FIRST_ROW_HEIGHT - 1);
	gtk_widget_show(top_left_label);

	/* Top-left label font attributes */
	PangoAttrList *attrs;
	attrs = pango_attr_list_new();
	pango_attr_list_insert(attrs, pango_attr_size_new_absolute(VISUAL_MOD_FONT_SIZE << 10));
	pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
	gtk_label_set_attributes(GTK_LABEL(top_left_label), attrs);

	/* Top-left event box */
	GtkWidget *top_left_event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(top_left_event_box), top_left_label);
	gtk_widget_show(top_left_event_box);

	/* Top-left event box background color */
	GdkColor color;
	gdk_color_parse("white", &color);
	gtk_widget_modify_bg(top_left_event_box, GTK_STATE_NORMAL, &color);

	/* Table */
	GtkWidget *table;
	table = gtk_table_new(3, 3, FALSE);
	gtk_table_attach(GTK_TABLE(table), layout, 1, 2, 1, 2,
		GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
	gtk_table_attach(GTK_TABLE(table), top_left_event_box, 0, 1, 0, 1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), first_row_layout, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), first_col_layout, 0, 1, 1, 2, 0, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
	gtk_table_attach(GTK_TABLE(table), hscrollbar, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(table), vscrollbar, 2, 3, 1, 2, GTK_FILL, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);

	/* Frame */
	GtkWidget *frame;
	frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(frame), table);
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

	/* Assign main widget */
	visual_mod_widget->widget = vbox;
	g_signal_connect(G_OBJECT(visual_mod_widget->widget), "destroy",
		G_CALLBACK(visual_mod_widget_destroy), visual_mod_widget);

	/* Return */
	return visual_mod_widget;
}


void visual_mod_widget_free(struct visual_mod_widget_t *visual_mod_widget)
{
	/* Free access list */
	vlist_free(visual_mod_widget->access_list);

	/* Free widget */
	free(visual_mod_widget->name);
	free(visual_mod_widget);
}


void visual_mod_widget_refresh(struct visual_mod_widget_t *visual_mod_widget)
{
	struct visual_mod_t *mod;

	long long cycle;

	GtkWidget *layout;

	GList *child;

	int width;
	int height;

	int table_width;
	int table_height;

	int left;
	int left_way;
	int left_way_offset;

	int top;
	int top_set;
	int top_set_offset;

	int set;
	int way;

	int x;
	int y;

	char str[MAX_STRING_SIZE];

	/* Go to cycle */
	cycle = cycle_bar_get_cycle(visual_cycle_bar);
	state_file_go_to_cycle(visual_state_file, cycle);

	/* Get associated module */
	mod = hash_table_get(visual_mem_system->mod_table, visual_mod_widget->name);
	if (!mod)
		panic("%s: invalid module", __FUNCTION__);

	/* Remove all widgets from layouts */
	layout = visual_mod_widget->layout;
	while ((child = gtk_container_get_children(GTK_CONTAINER(layout))))
		gtk_container_remove(GTK_CONTAINER(layout), child->data);
	while ((child = gtk_container_get_children(GTK_CONTAINER(visual_mod_widget->first_row_layout))))
		gtk_container_remove(GTK_CONTAINER(visual_mod_widget->first_row_layout), child->data);
	while ((child = gtk_container_get_children(GTK_CONTAINER(visual_mod_widget->first_col_layout))))
		gtk_container_remove(GTK_CONTAINER(visual_mod_widget->first_col_layout), child->data);

	/* Get allocated dimensions */
	width = gtk_widget_get_allocated_width(layout);
	height = gtk_widget_get_allocated_height(layout);
	visual_mod_widget->width = width;
	visual_mod_widget->height = height;

	/* Dimensions */
	table_width = VISUAL_MOD_CELL_WIDTH * mod->assoc;
	table_height = VISUAL_MOD_CELL_HEIGHT * mod->num_sets;

	/* Horizontal scroll bar */
	if (table_width > width)
	{
		gtk_range_set_range(GTK_RANGE(visual_mod_widget->hscrollbar), 0, table_width - width);
		gtk_range_set_increments(GTK_RANGE(visual_mod_widget->hscrollbar),
			VISUAL_MOD_CELL_WIDTH / 3, width - VISUAL_MOD_CELL_WIDTH / 3);
		gtk_widget_set_visible(visual_mod_widget->hscrollbar, TRUE);
	}
	else
		gtk_widget_set_visible(visual_mod_widget->hscrollbar, FALSE);

	/* Vertical scroll bar */
	if (table_height > height)
	{
		gtk_range_set_range(GTK_RANGE(visual_mod_widget->vscrollbar), 0, table_height - height);
		gtk_range_set_increments(GTK_RANGE(visual_mod_widget->vscrollbar),
			VISUAL_MOD_CELL_HEIGHT, height - VISUAL_MOD_CELL_HEIGHT);
		gtk_widget_set_visible(visual_mod_widget->vscrollbar, TRUE);
	}
	else
		gtk_widget_set_visible(visual_mod_widget->vscrollbar, FALSE);

	/* Get starting X position */
	left = gtk_range_get_value(GTK_RANGE(visual_mod_widget->hscrollbar));
	left_way = left / VISUAL_MOD_CELL_WIDTH;
	left_way_offset = -(left % VISUAL_MOD_CELL_WIDTH);

	/* Get starting Y position */
	top = gtk_range_get_value(GTK_RANGE(visual_mod_widget->vscrollbar));
	top_set = top / VISUAL_MOD_CELL_HEIGHT;
	top_set_offset = -(top % VISUAL_MOD_CELL_HEIGHT);

	/* First row */
	way = left_way;
	x = left_way_offset;
	while (x < width && way < mod->assoc)
	{
		snprintf(str, sizeof str, "%d", way);
		GtkWidget *label = gtk_label_new(str);
		gtk_widget_set_size_request(label, VISUAL_MOD_CELL_WIDTH - 1,
			VISUAL_MOD_FIRST_ROW_HEIGHT - 1);
		gtk_widget_show(label);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(VISUAL_MOD_FONT_SIZE << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(event_box), label);
		gtk_layout_put(GTK_LAYOUT(visual_mod_widget->first_row_layout), event_box, x, 0);
		gtk_widget_show(event_box);

		/* Color */
		GdkColor color;
		gdk_color_parse("#ffffff", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

		/* Next way */
		x += VISUAL_MOD_CELL_WIDTH;
		way++;
	}

	/* First column */
	set = top_set;
	y = top_set_offset;
	while (y < height && set < mod->num_sets)
	{
		snprintf(str, sizeof str, "%d", set);
		GtkWidget *label = gtk_label_new(str);
		gtk_widget_set_size_request(label, VISUAL_MOD_FIRST_COL_WIDTH - 1,
			VISUAL_MOD_CELL_HEIGHT - 1);
		gtk_widget_show(label);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(VISUAL_MOD_FONT_SIZE << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(event_box), label);
		gtk_layout_put(GTK_LAYOUT(visual_mod_widget->first_col_layout), event_box, 0, y);
		gtk_widget_show(event_box);

		/* Color */
		GdkColor color;
		gdk_color_parse("#ffffff", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

		/* Next set */
		y += VISUAL_MOD_CELL_HEIGHT;
		set++;
	}

	/* Blocks */
	set = top_set;
	y = top_set_offset;
	while (y < height && set < mod->num_sets)
	{
		/* Ways */
		way = left_way;
		x = left_way_offset;
		while (x < width && way < mod->assoc)
		{
			struct visual_mod_block_t *block;

			char *state_str;

			/*int num_sharers;
			int num_accesses;*/

			GtkWidget *label;
			GtkWidget *event_box;

			PangoAttrList *attrs;
			PangoAttribute *size_attr;

			/* Get block properties */
			assert(IN_RANGE(set, 0, mod->num_sets - 1));
			assert(IN_RANGE(way, 0, mod->assoc - 1));
			block = &mod->blocks[set * mod->assoc + way];
			state_str = map_value(&visual_mod_block_state_map, block->state);

			/* Tag label */
			snprintf(str, sizeof str, "0x%x (%s)", block->tag, state_str);
			label = gtk_label_new(str);
			gtk_widget_set_size_request(label, VISUAL_MOD_LABEL_BLOCK_WIDTH,
				VISUAL_MOD_CELL_HEIGHT - 1);
			gtk_widget_show(label);

			/* Set label font attributes */
			attrs = pango_attr_list_new();
			size_attr = pango_attr_size_new_absolute(VISUAL_MOD_FONT_SIZE << 10);
			pango_attr_list_insert(attrs, size_attr);
			gtk_label_set_attributes(GTK_LABEL(label), attrs);

			/* Event box */
			event_box = gtk_event_box_new();
			gtk_container_add(GTK_CONTAINER(event_box), label);
			gtk_layout_put(GTK_LAYOUT(layout), event_box, x + VISUAL_MOD_LABEL_BLOCK_LEFT, y);
			gtk_widget_show(event_box);

			/* Background color */
			GdkColor color;
			assert(IN_RANGE(block->state, 0, VISUAL_MOD_NUM_BLOCK_COLORS - 1));
			gdk_color_parse(visual_mod_block_color[block->state], &color);
			gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

#if 0
			/* Sharers text */
			num_sharers = visual_mod_block_get_num_sharers(mod, set, way);
			snprintf(str, sizeof str, "+%d", num_sharers);
			if (!num_sharers)
				strcpy(str, "-");

			/* Sharers label */
			label = gtk_label_new(str);
			gtk_widget_set_size_request(label, VISUAL_MOD_LABEL_SHARERS_WIDTH,
				VISUAL_MOD_CELL_HEIGHT - 1);
			gtk_widget_show(label);
			block->sharers_label = label;

			/* Set label font attributes */
			attrs = pango_attr_list_new();
			size_attr = pango_attr_size_new_absolute(VCACHE_FONT_SIZE << 10);
			pango_attr_list_insert(attrs, size_attr);
			gtk_label_set_attributes(GTK_LABEL(label), attrs);

			/* Event box */
			event_box = gtk_event_box_new();
			gtk_container_add(GTK_CONTAINER(event_box), label);
			gtk_layout_put(GTK_LAYOUT(layout), event_box, x + VCACHE_LABEL_SHARERS_LEFT, y);
			gtk_widget_show(event_box);
			gtk_widget_add_events(event_box, GDK_ENTER_NOTIFY_MASK |
				GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);
			if (num_sharers)
			{
				g_signal_connect(G_OBJECT(event_box), "enter-notify-event",
					G_CALLBACK(vcache_block_sharers_enter_notify_event), block);
				g_signal_connect(G_OBJECT(event_box), "leave-notify-event",
					G_CALLBACK(vcache_block_sharers_leave_notify_event), block);
				g_signal_connect(G_OBJECT(event_box), "button-press-event",
					G_CALLBACK(vcache_block_sharers_clicked_event), block);
			}
#endif

#if 0
			/* Accesses text */
			num_accesses = linked_list_count(block->vmod_access_list);
			snprintf(str, sizeof str, "+%d", num_accesses);
			if (!num_accesses)
				strcpy(str, "-");

			/* Accesses label */
			label = gtk_label_new(str);
			gtk_widget_set_size_request(label, VCACHE_LABEL_ACCESSES_WIDTH, VCACHE_CELL_HEIGHT - 1);
			gtk_widget_show(label);
			block->accesses_label = label;

			/* Set label font attributes */
			attrs = pango_attr_list_new();
			size_attr = pango_attr_size_new_absolute(VCACHE_FONT_SIZE << 10);
			pango_attr_list_insert(attrs, size_attr);
			gtk_label_set_attributes(GTK_LABEL(label), attrs);

			/* Event box */
			event_box = gtk_event_box_new();
			gtk_container_add(GTK_CONTAINER(event_box), label);
			gtk_layout_put(GTK_LAYOUT(layout), event_box, x + VCACHE_LABEL_ACCESSES_LEFT, y);
			gtk_widget_show(event_box);
			gtk_widget_add_events(event_box, GDK_ENTER_NOTIFY_MASK |
				GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);
			if (num_accesses)
			{
				g_signal_connect(G_OBJECT(event_box), "enter-notify-event",
					G_CALLBACK(vcache_block_accesses_enter_notify_event), block);
				g_signal_connect(G_OBJECT(event_box), "leave-notify-event",
					G_CALLBACK(vcache_block_accesses_leave_notify_event), block);
				g_signal_connect(G_OBJECT(event_box), "button-press-event",
					G_CALLBACK(vcache_block_accesses_clicked_event), block);
			}
#endif

			/* Next way */
			x += VISUAL_MOD_CELL_WIDTH;
			way++;
		}

		/* Next set */
		y += VISUAL_MOD_CELL_HEIGHT;
		set++;
	}

	/* Repaint if necessary */
	gtk_container_check_resize(GTK_CONTAINER(visual_mod_widget->layout));
	gtk_container_check_resize(GTK_CONTAINER(visual_mod_widget->first_row_layout));
	gtk_container_check_resize(GTK_CONTAINER(visual_mod_widget->first_col_layout));
}


GtkWidget *visual_mod_widget_get_widget(struct visual_mod_widget_t *widget)
{
	return widget->widget;
}

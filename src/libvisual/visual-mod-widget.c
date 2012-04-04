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

#define VISUAL_MOD_SHARERS_LABEL_LEFT		101
#define VISUAL_MOD_SHARERS_LABEL_WIDTH		25

#define VISUAL_MOD_ACCESSES_LABEL_LEFT		127
#define VISUAL_MOD_ACCESSES_LABEL_WIDTH		25




/*
 * Sharers label
 */

struct sharers_label_t
{
	GtkWidget *widget;
	GtkWidget *label;

	GdkColor label_color;

	char *mod_name;

	int set;
	int way;
};


/* Forwards */
static struct sharers_label_t *sharers_label_create(char *mod_name, int set, int way);
static void sharers_label_free(struct sharers_label_t *sharers_label);
static GtkWidget *sharers_label_get_widget(struct sharers_label_t *sharers_label);


static void sharers_label_destroy(GtkWidget *widget,
	struct sharers_label_t *sharers_label)
{
	sharers_label_free(sharers_label);
}


static gboolean sharers_label_enter_notify(GtkWidget *widget,
	GdkEventCrossing *event, struct sharers_label_t *sharers_label)
{
	GdkColor color;

	PangoAttrList *attrs;
	PangoAttribute *underline_attr;

	GdkWindow *window;
	GdkCursor *cursor;

	GtkStyle *style;

	style = gtk_widget_get_style(sharers_label->label);
	sharers_label->label_color = style->fg[GTK_STATE_NORMAL];

	gdk_color_parse("red", &color);
	gtk_widget_modify_fg(sharers_label->label, GTK_STATE_NORMAL, &color);

	attrs = gtk_label_get_attributes(GTK_LABEL(sharers_label->label));
	underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
	pango_attr_list_change(attrs, underline_attr);

	cursor = gdk_cursor_new(GDK_HAND1);
	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, cursor);
	gdk_cursor_unref(cursor);

	return FALSE;
}


static gboolean sharers_label_leave_notify(GtkWidget *widget,
	GdkEventCrossing *event, struct sharers_label_t *sharers_label)
{
	PangoAttrList *attrs;
	PangoAttribute *underline_attr;
	GdkWindow *window;

	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, NULL);

	attrs = gtk_label_get_attributes(GTK_LABEL(sharers_label->label));
	underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_NONE);
	pango_attr_list_change(attrs, underline_attr);
	gtk_widget_modify_fg(sharers_label->label, GTK_STATE_NORMAL, &sharers_label->label_color);

	return FALSE;
}


static void sharers_label_clicked(GtkWidget *widget,
	GdkEventButton *event, struct sharers_label_t *sharers_label)
{
	struct visual_mod_t *mod;
	struct visual_mod_t *mod_sharer;
	struct visual_net_t *net;

	char buf[MAX_LONG_STRING_SIZE];
	char *buf_ptr;
	char *comma;

	int size;
	int sub_block;
	int sharer;

	long long cycle;

	char *title_format_begin = "<span color=\"blue\"><b>";
	char *title_format_end = "</b></span>";

	/* Go to current cycle */
	cycle = cycle_bar_get_cycle(visual_cycle_bar);
	state_file_go_to_cycle(visual_state_file, cycle);

	/* Get module */
	mod = hash_table_get(visual_mem_system->mod_table, sharers_label->mod_name);
	if (!mod)
		panic("%s: %s: invalid module", __FUNCTION__, sharers_label->mod_name);

	/* Initialize */
	net = mod->high_net;
	buf_ptr = buf;
	size = sizeof buf;

	/* Title */
	str_printf(&buf_ptr, &size, "%sModule %s - Block at set %d / way %d%s\n\n",
		title_format_begin, mod->name, sharers_label->set, sharers_label->way, title_format_end);

	/* Sub-blocks */
	for (sub_block = 0; sub_block < mod->num_sub_blocks; sub_block++)
	{
		struct visual_mod_dir_entry_t *dir_entry;

		/* Start */
		str_printf(&buf_ptr, &size, "%sSub-block %d:%s\n",
			title_format_begin, sub_block, title_format_end);

		/* Owner */
		mod_sharer = NULL;
		dir_entry = visual_mod_dir_entry_get(mod, sharers_label->set,
			sharers_label->way, sub_block);
		if (dir_entry->owner >= 0)
		{
			mod_sharer = visual_net_get_mod(net, dir_entry->owner);
			if (!mod_sharer)
				panic("%s: invalid owner", __FUNCTION__);
		}
		str_printf(&buf_ptr, &size, "Owner: %s\n", mod_sharer ? mod_sharer->name : "-");

		/* Sharers */
		comma = "";
		str_printf(&buf_ptr, &size, "Sharers: ");
		for (sharer = 0; sharer < mod->num_sharers; sharer++)
		{
			/* Sharer not set */
			if (!visual_mod_dir_entry_is_sharer(mod, sharers_label->set,
				sharers_label->way, sub_block, sharer))
				continue;

			/* New sharer */
			mod_sharer = visual_net_get_mod(net, sharer);
			if (!mod_sharer)
				panic("%s: invalid sharer", __FUNCTION__);

			/* Dump */
			str_printf(&buf_ptr, &size, "%s%s", comma, mod_sharer->name);
			comma = ", ";
		}

		/* End */
		str_printf(&buf_ptr, &size, "\n\n");
	}

	/* Show pop-up */
	info_popup_show(buf);
}


static struct sharers_label_t *sharers_label_create(char *mod_name, int set, int way)
{
	struct sharers_label_t *sharers_label;
	struct visual_mod_t *mod;

	int num_sharers;

	char str[MAX_STRING_SIZE];

	/* Allocate */
	sharers_label = calloc(1, sizeof(struct sharers_label_t));
	if (!sharers_label)
		fatal("%s: out of memory", __FUNCTION__);

	/* Module name */
	sharers_label->mod_name = strdup(mod_name);
	if (!sharers_label->mod_name)
		fatal("%s: out of memroy", __FUNCTION__);

	/* Initialize */
	sharers_label->set = set;
	sharers_label->way = way;

	/* Get module */
	mod = hash_table_get(visual_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: %s: invalid module", __FUNCTION__, mod_name);

	/* Sharers text */
	num_sharers = visual_mod_block_get_num_sharers(mod, set, way);
	snprintf(str, sizeof str, "+%d", num_sharers);
	if (!num_sharers)
		strcpy(str, "-");

	/* Sharers label */
	GtkWidget *label;
	label = gtk_label_new(str);
	gtk_widget_set_size_request(label, VISUAL_MOD_SHARERS_LABEL_WIDTH,
		VISUAL_MOD_CELL_HEIGHT - 1);
	gtk_widget_show(label);
	sharers_label->label = label;

	/* Set label font attributes */
	PangoAttrList *attrs;
	PangoAttribute *size_attr;
	attrs = pango_attr_list_new();
	size_attr = pango_attr_size_new_absolute(VISUAL_MOD_FONT_SIZE << 10);
	pango_attr_list_insert(attrs, size_attr);
	gtk_label_set_attributes(GTK_LABEL(label), attrs);

	/* Event box */
	GtkWidget *event_box;
	event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(event_box), label);
	gtk_widget_show(event_box);
	gtk_widget_add_events(event_box, GDK_ENTER_NOTIFY_MASK |
		GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);
	if (num_sharers)
	{
		g_signal_connect(G_OBJECT(event_box), "enter-notify-event",
			G_CALLBACK(sharers_label_enter_notify), sharers_label);
		g_signal_connect(G_OBJECT(event_box), "leave-notify-event",
			G_CALLBACK(sharers_label_leave_notify), sharers_label);
		g_signal_connect(G_OBJECT(event_box), "button-press-event",
			G_CALLBACK(sharers_label_clicked), sharers_label);
	}

	/* Main widget */
	sharers_label->widget = event_box;
	g_signal_connect(G_OBJECT(sharers_label->widget), "destroy",
		G_CALLBACK(sharers_label_destroy), sharers_label);

	/* Return */
	return sharers_label;
}


static void sharers_label_free(struct sharers_label_t *sharers_label)
{
	free(sharers_label->mod_name);
	free(sharers_label);
}


static GtkWidget *sharers_label_get_widget(struct sharers_label_t *sharers_label)
{
	return sharers_label->widget;
}




/*
 * Accesses label
 */

struct accesses_label_t
{
	GtkWidget *widget;
	GtkWidget *label;

	GdkColor label_color;

	char *mod_name;

	int set;
	int way;
};


/* Forwards */
static struct accesses_label_t *accesses_label_create(char *mod_name, int set, int way);
static void accesses_label_free(struct accesses_label_t *accesses_label);
static GtkWidget *accesses_label_get_widget(struct accesses_label_t *accesses_label);


static void accesses_label_destroy(GtkWidget *widget,
	struct accesses_label_t *accesses_label)
{
	accesses_label_free(accesses_label);
}


static gboolean accesses_label_enter_notify(GtkWidget *widget,
	GdkEventCrossing *event, struct sharers_label_t *accesses_label)
{
	GdkColor color;

	PangoAttrList *attrs;
	PangoAttribute *underline_attr;

	GdkWindow *window;
	GdkCursor *cursor;

	GtkStyle *style;

	style = gtk_widget_get_style(accesses_label->label);
	accesses_label->label_color = style->fg[GTK_STATE_NORMAL];

	gdk_color_parse("red", &color);
	gtk_widget_modify_fg(accesses_label->label, GTK_STATE_NORMAL, &color);

	attrs = gtk_label_get_attributes(GTK_LABEL(accesses_label->label));
	underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
	pango_attr_list_change(attrs, underline_attr);

	cursor = gdk_cursor_new(GDK_HAND1);
	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, cursor);
	gdk_cursor_unref(cursor);

	return FALSE;
}


static gboolean accesses_label_leave_notify(GtkWidget *widget,
	GdkEventCrossing *event, struct accesses_label_t *accesses_label)
{
	PangoAttrList *attrs;
	PangoAttribute *underline_attr;
	GdkWindow *window;

	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, NULL);

	attrs = gtk_label_get_attributes(GTK_LABEL(accesses_label->label));
	underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_NONE);
	pango_attr_list_change(attrs, underline_attr);
	gtk_widget_modify_fg(accesses_label->label, GTK_STATE_NORMAL, &accesses_label->label_color);

	return FALSE;
}


static void accesses_label_clicked(GtkWidget *widget,
	GdkEventButton *event, struct accesses_label_t *accesses_label)
{
	long long cycle;

	struct visual_mod_t *mod;

	char buf[MAX_LONG_STRING_SIZE];
	char *buf_ptr;

	struct linked_list_t *access_list;

	int size;
	int index;

	char *title_format_begin = "<span color=\"blue\"><b>";
	char *title_format_end = "</b></span>";

	/* Go to current cycle */
	cycle = cycle_bar_get_cycle(visual_cycle_bar);
	state_file_go_to_cycle(visual_state_file, cycle);

	/* Get module */
	mod = hash_table_get(visual_mem_system->mod_table, accesses_label->mod_name);
	if (!mod)
		panic("%s: %s: invalid module", __FUNCTION__, accesses_label->mod_name);

	/* Initialize */
	buf_ptr = buf;
	size = sizeof buf;

	/* Title */
	str_printf(&buf_ptr, &size, "%sModule %s - Accesses at set %d / way %d%s\n\n",
		title_format_begin, mod->name, accesses_label->set, accesses_label->way, title_format_end);

	/* Number of accesses */
	access_list = visual_mod_get_access_list(mod, accesses_label->set, accesses_label->way);
	str_printf(&buf_ptr, &size, "%sNumber of accesses:%s %d\n",
		title_format_begin, title_format_end, access_list->count);

	/* List of accesses */
	index = 0;
	LINKED_LIST_FOR_EACH(access_list)
	{
		struct visual_mod_access_t *access;

		access = linked_list_get(access_list);
		str_printf(&buf_ptr, &size, "%sAccess[%d]%s = %s\n",
			title_format_begin, index, title_format_end, access->name);

		index++;
	}

	/* Show pop-up */
	info_popup_show(buf);
}


static struct accesses_label_t *accesses_label_create(char *mod_name, int set, int way)
{
	struct accesses_label_t *accesses_label;
	struct linked_list_t *access_list;
	struct visual_mod_t *mod;

	int num_accesses;

	char str[MAX_STRING_SIZE];

	/* Allocate */
	accesses_label = calloc(1, sizeof(struct accesses_label_t));
	if (!accesses_label)
		fatal("%s: out of memory", __FUNCTION__);

	/* Module name */
	accesses_label->mod_name = strdup(mod_name);
	if (!accesses_label->mod_name)
		fatal("%s: out of memroy", __FUNCTION__);

	/* Initialize */
	accesses_label->set = set;
	accesses_label->way = way;

	/* Get module */
	mod = hash_table_get(visual_mem_system->mod_table, mod_name);
	if (!mod)
		panic("%s: %s: invalid module", __FUNCTION__, mod_name);

	/* Accesses text */
	access_list = visual_mod_get_access_list(mod, set, way);
	num_accesses = linked_list_count(access_list);
	snprintf(str, sizeof str, "+%d", num_accesses);
	if (!num_accesses)
		strcpy(str, "-");

	/* Accesses label */
	GtkWidget *label;
	label = gtk_label_new(str);
	gtk_widget_set_size_request(label, VISUAL_MOD_ACCESSES_LABEL_WIDTH, VISUAL_MOD_CELL_HEIGHT - 1);
	gtk_widget_show(label);
	accesses_label->label = label;

	/* Set label font attributes */
	PangoAttrList *attrs;
	PangoAttribute *size_attr;
	attrs = pango_attr_list_new();
	size_attr = pango_attr_size_new_absolute(VISUAL_MOD_FONT_SIZE << 10);
	pango_attr_list_insert(attrs, size_attr);
	gtk_label_set_attributes(GTK_LABEL(label), attrs);

	/* Event box */
	GtkWidget *event_box;
	event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(event_box), label);
	gtk_widget_show(event_box);
	gtk_widget_add_events(event_box, GDK_ENTER_NOTIFY_MASK
		| GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);
	if (num_accesses)
	{
		g_signal_connect(G_OBJECT(event_box), "enter-notify-event",
				G_CALLBACK(accesses_label_enter_notify), accesses_label);
		g_signal_connect(G_OBJECT(event_box), "leave-notify-event",
				G_CALLBACK(accesses_label_leave_notify), accesses_label);
		g_signal_connect(G_OBJECT(event_box), "button-press-event",
				G_CALLBACK(accesses_label_clicked), accesses_label);
	}

	/* Main widget */
	accesses_label->widget = event_box;
	g_signal_connect(G_OBJECT(accesses_label->widget), "destroy",
		G_CALLBACK(accesses_label_destroy), accesses_label);

	/* Return */
	return accesses_label;
}


static void accesses_label_free(struct accesses_label_t *accesses_label)
{
	free(accesses_label->mod_name);
	free(accesses_label);
}


static GtkWidget *accesses_label_get_widget(struct accesses_label_t *accesses_label)
{
	return accesses_label->widget;
}




/*
 * Visual Module Widget
 */


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
		(vlist_get_elem_name_func_t) visual_mem_system_get_access_name_short,
		(vlist_get_elem_desc_func_t) visual_mem_system_get_access_desc);
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
	while (vlist_count(visual_mod_widget->access_list))
		free(vlist_remove_at(visual_mod_widget->access_list, 0));
	vlist_free(visual_mod_widget->access_list);

	/* Free widget */
	free(visual_mod_widget->name);
	free(visual_mod_widget);
}


void visual_mod_widget_refresh(struct visual_mod_widget_t *visual_mod_widget)
{
	struct visual_mod_t *mod;
	struct visual_mod_access_t *access;

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
	char *access_name;

	/* Go to cycle */
	cycle = cycle_bar_get_cycle(visual_cycle_bar);
	state_file_go_to_cycle(visual_state_file, cycle);

	/* Get associated module */
	mod = hash_table_get(visual_mem_system->mod_table, visual_mod_widget->name);
	if (!mod)
		panic("%s: invalid module", __FUNCTION__);

	/* Remove all accesses from access list */
	while (vlist_count(visual_mod_widget->access_list))
		free(vlist_remove_at(visual_mod_widget->access_list, 0));

	/* Add new accesses */
	HASH_TABLE_FOR_EACH(mod->access_table, access_name, access)
	{
		/* Access name */
		access_name = strdup(access_name);
		if (!access_name)
			fatal("%s: out of memory", __FUNCTION__);

		/* Insert new access */
		vlist_add(visual_mod_widget->access_list, access_name);
	}
	vlist_refresh(visual_mod_widget->access_list);

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

			/* Sharers label */
			struct sharers_label_t *sharers_label;
			sharers_label = sharers_label_create(mod->name, set, way);
			gtk_layout_put(GTK_LAYOUT(layout), sharers_label_get_widget(sharers_label),
				x + VISUAL_MOD_SHARERS_LABEL_LEFT, y);

			/* Accesses label */
			struct accesses_label_t *accesses_label;
			accesses_label = accesses_label_create(mod->name, set, way);
			gtk_layout_put(GTK_LAYOUT(layout), accesses_label_get_widget(accesses_label),
				x + VISUAL_MOD_ACCESSES_LABEL_LEFT, y);

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

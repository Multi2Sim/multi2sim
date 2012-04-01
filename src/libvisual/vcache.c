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


#define VCACHE_DIR_ENTRY_SHARERS_SIZE(vcache) (((vcache)->num_sharers + 7) / 8)
#define VCACHE_DIR_ENTRY_SIZE(vcache) (sizeof(struct vcache_dir_entry_t) + \
	VCACHE_DIR_ENTRY_SHARERS_SIZE((vcache)))

#define VCACHE_CELL_WIDTH		153
#define VCACHE_CELL_HEIGHT		20

#define VCACHE_FIRST_ROW_HEIGHT		20
#define VCACHE_FIRST_COL_WIDTH		100

#define VCACHE_FONT_SIZE		12

#define VCACHE_LABEL_BLOCK_LEFT		0
#define VCACHE_LABEL_BLOCK_WIDTH	100

#define VCACHE_LABEL_SHARERS_LEFT	101
#define VCACHE_LABEL_SHARERS_WIDTH	25

#define VCACHE_LABEL_ACCESSES_LEFT	127
#define VCACHE_LABEL_ACCESSES_WIDTH	25


static struct string_map_t vcache_block_state_map =
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


#define VCACHE_NUM_BLOCK_STATE_COLORS  6
static char *vcache_block_state_color[VCACHE_NUM_BLOCK_STATE_COLORS] =
{
	"#eeeeee",	/* light gray */
	"#33ccff",	/* blue */
	"#00ff33",	/* green */
	"#ffff33",	/* yellow */
	"#ff9900",	/* orange */
	"#cc99ff"	/* purple */
};


static void vcache_size_allocate_event(GtkWidget *widget, GdkRectangle *allocation, struct vcache_t *vcache)
{
	if (allocation->width != vcache->width || allocation->height != vcache->height)
		vcache_refresh(vcache);
}


static gboolean vcache_scroll_event(GtkWidget *widget, GdkEventScroll *event, struct vcache_t *vcache)
{
	int value;

	value = gtk_range_get_value(GTK_RANGE(vcache->vscrollbar));
	if (event->direction == GDK_SCROLL_UP)
		value -= 10;
	else
		value += 10;
	gtk_range_set_value(GTK_RANGE(vcache->vscrollbar), value);
	return FALSE;
}


static void vcache_scroll_bar_value_changed_event(GtkRange *range, struct vcache_t *vcache)
{
	vcache_refresh(vcache);
}


static struct vcache_dir_entry_t *vcache_get_dir_entry(struct vcache_t *vcache, int set, int way, int sub_block)
{
	struct vcache_block_t *block;
	struct vcache_dir_entry_t *dir_entry;

	assert(IN_RANGE(set, 0, vcache->num_sets - 1));
	assert(IN_RANGE(way, 0, vcache->assoc - 1));
	assert(IN_RANGE(sub_block, 0, vcache->num_sub_blocks - 1));

	block = &vcache->blocks[set * vcache->assoc + way];
	dir_entry = (struct vcache_dir_entry_t *) (((void *) block->dir_entries) + VCACHE_DIR_ENTRY_SIZE(vcache) * sub_block);

	return dir_entry;
}


static void vcache_dir_entry_read_checkpoint(struct vcache_t *vcache,
	int x, int y, int z, FILE *f)
{
	struct vcache_dir_entry_t *dir_entry;

	int count = 0;

	dir_entry = vcache_get_dir_entry(vcache, x, y, z);
	count += fread(&dir_entry->num_sharers, 1, 4, f);
	count += fread(&dir_entry->owner, 1, 4, f);
	count += fread(dir_entry->sharers, 1, VCACHE_DIR_ENTRY_SHARERS_SIZE(vcache), f);
	if (count != 8 + VCACHE_DIR_ENTRY_SHARERS_SIZE(vcache))
		panic("%s: corrupted checkpoint", __FUNCTION__);
}


static void vcache_dir_entry_write_checkpoint(struct vcache_t *vcache,
	int x, int y, int z, FILE *f)
{
	struct vcache_dir_entry_t *dir_entry;

	dir_entry = vcache_get_dir_entry(vcache, x, y, z);
	fwrite(&dir_entry->num_sharers, 1, 4, f);
	fwrite(&dir_entry->owner, 1, 4, f);
	fwrite(dir_entry->sharers, 1, VCACHE_DIR_ENTRY_SHARERS_SIZE(vcache), f);
}


/* Get total number of sharers of a block, adding up sharers of each sub-block */
static int vcache_block_get_num_sharers(struct vcache_t *vcache, int set, int way)
{
	struct vcache_dir_entry_t *dir_entry;

	int num_sharers;
	int i;

	num_sharers = 0;
	for (i = 0; i < vcache->num_sub_blocks; i++)
	{
		dir_entry = vcache_get_dir_entry(vcache, set, way, i);
		num_sharers += dir_entry->num_sharers;
	}

	return num_sharers;
}


static gboolean vcache_block_sharers_enter_notify_event(GtkWidget *widget,
	GdkEventCrossing *event, struct vcache_block_t *block)
{
	GdkColor color;

	PangoAttrList *attrs;
	PangoAttribute *underline_attr;

	GdkWindow *window;
	GdkCursor *cursor;

	GtkStyle *style;

	style = gtk_widget_get_style(block->sharers_label);
	block->sharers_label_color = style->fg[GTK_STATE_NORMAL];

	gdk_color_parse("red", &color);
	gtk_widget_modify_fg(block->sharers_label, GTK_STATE_NORMAL, &color);

	attrs = gtk_label_get_attributes(GTK_LABEL(block->sharers_label));
	underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
	pango_attr_list_change(attrs, underline_attr);

	cursor = gdk_cursor_new(GDK_HAND1);
	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, cursor);
	gdk_cursor_unref(cursor);

	return FALSE;
}


static gboolean vcache_block_sharers_leave_notify_event(GtkWidget *widget,
	GdkEventCrossing *event, struct vcache_block_t *block)
{
	PangoAttrList *attrs;
	PangoAttribute *underline_attr;
	GdkWindow *window;

	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, NULL);

	attrs = gtk_label_get_attributes(GTK_LABEL(block->sharers_label));
	underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_NONE);
	pango_attr_list_change(attrs, underline_attr);
	gtk_widget_modify_fg(block->sharers_label, GTK_STATE_NORMAL, &block->sharers_label_color);

	return FALSE;
}


static void vcache_block_sharers_clicked_event(GtkWidget *widget,
	GdkEventButton *event, struct vcache_block_t *block)
{
	struct vcache_t *vcache = block->vcache;
	struct vmod_t *vmod = vcache->vmod;
	struct vmod_t *vmod_sharer;
	struct vnet_t *vnet = vmod->high_vnet;

	char buf[MAX_LONG_STRING_SIZE];
	char *buf_ptr;
	char *comma;

	int size;
	int sub_block;
	int sharer;

	/* Initialize */
	buf_ptr = buf;
	size = sizeof buf;

	/* Title */
	str_printf(&buf_ptr, &size, "<b>Module %s - Block at set %d - way %d</b>\n\n",
		vmod->name, block->set, block->way);

	/* Sub-blocks */
	for (sub_block = 0; sub_block < vcache->num_sub_blocks; sub_block++)
	{
		struct vcache_dir_entry_t *dir_entry;

		/* Start */
		str_printf(&buf_ptr, &size, "<b>Sub-block %d:</b>\n", sub_block);

		/* Owner */
		vmod_sharer = NULL;
		dir_entry = vcache_get_dir_entry(vcache, block->set, block->way, sub_block);
		if (dir_entry->owner >= 0)
		{
			vmod_sharer = vnet_get_vmod(vnet, dir_entry->owner);
			if (!vmod_sharer)
				panic("%s: invalid owner", __FUNCTION__);
		}
		str_printf(&buf_ptr, &size, "Owner: %s\n", vmod_sharer ? vmod_sharer->name : "-");

		/* Sharers */
		comma = "";
		str_printf(&buf_ptr, &size, "Sharers: ");
		for (sharer = 0; sharer < vcache->num_sharers; sharer++)
		{
			/* Sharer not set */
			if (!vcache_dir_entry_is_sharer(vcache, block->set, block->way, sub_block, sharer))
				continue;

			/* New sharer */
			vmod_sharer = vnet_get_vmod(vnet, sharer);
			if (!vmod_sharer)
				panic("%s: invalid sharer", __FUNCTION__);

			/* Dump */
			str_printf(&buf_ptr, &size, "%s%s", comma, vmod_sharer->name);
			comma = ", ";
		}

		/* End */
		str_printf(&buf_ptr, &size, "\n\n");
	}

	/* Show pop-up */
	info_popup_show(buf);
}


static gboolean vcache_block_accesses_enter_notify_event(GtkWidget *widget,
	GdkEventCrossing *event, struct vcache_block_t *block)
{
	GdkColor color;

	PangoAttrList *attrs;
	PangoAttribute *underline_attr;

	GdkWindow *window;
	GdkCursor *cursor;

	GtkStyle *style;

	style = gtk_widget_get_style(block->accesses_label);
	block->accesses_label_color = style->fg[GTK_STATE_NORMAL];

	gdk_color_parse("red", &color);
	gtk_widget_modify_fg(block->accesses_label, GTK_STATE_NORMAL, &color);

	attrs = gtk_label_get_attributes(GTK_LABEL(block->accesses_label));
	underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
	pango_attr_list_change(attrs, underline_attr);

	cursor = gdk_cursor_new(GDK_HAND1);
	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, cursor);
	gdk_cursor_unref(cursor);

	return FALSE;
}


static gboolean vcache_block_accesses_leave_notify_event(GtkWidget *widget,
	GdkEventCrossing *event, struct vcache_block_t *block)
{
	PangoAttrList *attrs;
	PangoAttribute *underline_attr;
	GdkWindow *window;

	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, NULL);

	attrs = gtk_label_get_attributes(GTK_LABEL(block->accesses_label));
	underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_NONE);
	pango_attr_list_change(attrs, underline_attr);
	gtk_widget_modify_fg(block->accesses_label, GTK_STATE_NORMAL, &block->accesses_label_color);

	return FALSE;
}


static void vcache_block_accesses_clicked_event(GtkWidget *widget,
	GdkEventButton *event, struct vcache_block_t *block)
{
#if 0
	struct vcache_t *vcache = block->vcache;
	struct vmod_t *vmod = vcache->vmod;
	struct vmod_t *vmod_sharer;
	struct vnet_t *vnet = vmod->high_vnet;

	char buf[MAX_LONG_STRING_SIZE];
	char *buf_ptr;
	char *comma;

	int size;
	int sub_block;
	int sharer;

	/* Initialize */
	buf_ptr = buf;
	size = sizeof buf;

	/* Title */
	str_printf(&buf_ptr, &size, "<b>Module %s - Block at set %d - way %d</b>\n\n",
		vmod->name, block->set, block->way);

	/* Sub-blocks */
	for (sub_block = 0; sub_block < vcache->num_sub_blocks; sub_block++)
	{
		struct vcache_dir_entry_t *dir_entry;

		/* Start */
		str_printf(&buf_ptr, &size, "<b>Sub-block %d:</b>\n", sub_block);

		/* Owner */
		vmod_sharer = NULL;
		dir_entry = vcache_get_dir_entry(vcache, block->set, block->way, sub_block);
		if (dir_entry->owner >= 0)
		{
			vmod_sharer = vnet_get_vmod(vnet, dir_entry->owner);
			if (!vmod_sharer)
				panic("%s: invalid owner", __FUNCTION__);
		}
		str_printf(&buf_ptr, &size, "Owner: %s\n", vmod_sharer ? vmod_sharer->name : "-");

		/* Sharers */
		comma = "";
		str_printf(&buf_ptr, &size, "Sharers: ");
		for (sharer = 0; sharer < vcache->num_sharers; sharer++)
		{
			/* Sharer not set */
			if (!vcache_dir_entry_is_sharer(vcache, block->set, block->way, sub_block, sharer))
				continue;

			/* New sharer */
			vmod_sharer = vnet_get_vmod(vnet, sharer);
			if (!vmod_sharer)
				panic("%s: invalid sharer", __FUNCTION__);

			/* Dump */
			str_printf(&buf_ptr, &size, "%s%s", comma, vmod_sharer->name);
			comma = ", ";
		}

		/* End */
		str_printf(&buf_ptr, &size, "\n\n");
	}

	/* Show pop-up */
	info_popup_show(buf);
#endif
}




/*
 * Public Functions
 */

struct vcache_t *vcache_create(struct vmod_t *vmod, char *name, int num_sets, int assoc,
	int block_size, int sub_block_size, int num_sharers)
{
	int set;
	int way;

	struct vcache_t *vcache;
	struct vcache_block_t *block;

	/* Allocate */
	vcache = calloc(1, sizeof(struct vcache_t));
	if (!vcache)
		fatal("%s: out of memory", __FUNCTION__);

	/* Name */
	vcache->name = strdup(name);
	if (!vcache->name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	vcache->vmod = vmod;
	vcache->num_sets = num_sets;
	vcache->assoc = assoc;
	vcache->block_size = block_size;
	vcache->sub_block_size = sub_block_size;
	vcache->num_sub_blocks = block_size / sub_block_size;
	vcache->num_sharers = num_sharers;
	assert(block_size % sub_block_size == 0);

	/* Create block array */
	vcache->blocks = calloc(num_sets * assoc, sizeof(struct vcache_block_t));
	for (set = 0; set < num_sets; set++)
	{
		for (way = 0; way < assoc; way++)
		{
			int sub_block;

			block = &vcache->blocks[set * assoc + way];
			block->vcache = vcache;
			block->set = set;
			block->way = way;
			block->vmod_access_list = linked_list_create();
			block->dir_entries = calloc(vcache->num_sub_blocks, VCACHE_DIR_ENTRY_SIZE(vcache));

			for (sub_block = 0; sub_block < vcache->num_sub_blocks; sub_block++)
			{
				struct vcache_dir_entry_t *dir_entry;

				dir_entry = vcache_get_dir_entry(vcache, set, way, sub_block);
				dir_entry->owner = -1;
			}
		}
	}

	/* Scroll bars */
	GtkWidget *hscrollbar = gtk_hscrollbar_new(NULL);
	GtkWidget *vscrollbar = gtk_vscrollbar_new(NULL);
	vcache->hscrollbar = hscrollbar;
	vcache->vscrollbar = vscrollbar;
	g_signal_connect(G_OBJECT(hscrollbar), "value-changed", G_CALLBACK(vcache_scroll_bar_value_changed_event), vcache);
	g_signal_connect(G_OBJECT(vscrollbar), "value-changed", G_CALLBACK(vcache_scroll_bar_value_changed_event), vcache);

	/* Colors */
	GdkColor color_gray;
	gdk_color_parse("#aaaaaa", &color_gray);

	/* Layout */
	GtkWidget *layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(layout, 200, 100);
	g_signal_connect(G_OBJECT(layout), "size_allocate", G_CALLBACK(vcache_size_allocate_event), vcache);
	g_signal_connect(G_OBJECT(layout), "scroll-event", G_CALLBACK(vcache_scroll_event), vcache);
	gtk_widget_modify_bg(layout, GTK_STATE_NORMAL, &color_gray);
	vcache->layout = layout;

	/* First row layout */
	GtkWidget *first_row_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(first_row_layout, -1, VCACHE_FIRST_ROW_HEIGHT);
	gtk_widget_modify_bg(first_row_layout, GTK_STATE_NORMAL, &color_gray);
	vcache->first_row_layout = first_row_layout;

	/* First column layout */
	GtkWidget *first_col_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(first_col_layout, VCACHE_FIRST_COL_WIDTH, -1);
	gtk_widget_modify_bg(first_col_layout, GTK_STATE_NORMAL, &color_gray);
	vcache->first_col_layout = first_col_layout;

	/* Top-left label */
	GtkWidget *top_left_label = gtk_label_new(vcache->name);
	gtk_widget_set_size_request(top_left_label, VCACHE_FIRST_COL_WIDTH - 1, VCACHE_FIRST_ROW_HEIGHT - 1);
	gtk_widget_show(top_left_label);

	/* Top-left label font attributes */
	PangoAttrList *attrs;
	attrs = pango_attr_list_new();
	pango_attr_list_insert(attrs, pango_attr_size_new_absolute(VCACHE_FONT_SIZE << 10));
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
	vcache->widget = frame;

	/* Return */
	return vcache;
}


void vcache_free(struct vcache_t *vcache)
{
	struct vcache_block_t *block;
	struct linked_list_t *access_list;

	int i;

	/* Free blocks */
	for (i = 0; i < vcache->num_sets * vcache->assoc; i++)
	{
		/* Get block */
		block = &vcache->blocks[i];
		access_list = block->vmod_access_list;

		/* Free accesses */
		while (linked_list_count(access_list))
		{
			linked_list_head(access_list);
			vmod_access_free(linked_list_get(access_list));
			linked_list_remove(access_list);
		}
		linked_list_free(access_list);

		/* Directory entries */
		free(vcache->blocks[i].dir_entries);
	}
	free(vcache->blocks);

	/* Free object */
	free(vcache->name);
	free(vcache);
}


void vcache_add_access(struct vcache_t *vcache, int set, int way, struct vmod_access_t *access)
{
	struct vcache_block_t *block;
	struct linked_list_t *access_list;

	assert(IN_RANGE(set, 0, vcache->num_sets - 1));
	assert(IN_RANGE(way, 0, vcache->assoc - 1));

	/* Get block */
	block = &vcache->blocks[set * vcache->assoc + way];
	access_list = block->vmod_access_list;

	/* Add access */
	linked_list_add(access_list, access);
}


struct vmod_access_t *vcache_find_access(struct vcache_t *vcache, int set, int way, char *access_name)
{
	struct vcache_block_t *block;
	struct vmod_access_t *access;
	struct linked_list_t *access_list;

	assert(IN_RANGE(set, 0, vcache->num_sets - 1));
	assert(IN_RANGE(way, 0, vcache->assoc - 1));

	/* Get block */
	block = &vcache->blocks[set * vcache->assoc + way];
	access_list = block->vmod_access_list;

	/* Find access */
	LINKED_LIST_FOR_EACH(access_list)
	{
		access = linked_list_get(access_list);
		if (!strcmp(vmod_access_get_name(access), access_name))
			break;
	}

	/* Not found */
	if (linked_list_is_end(access_list))
		return NULL;

	/* Return access */
	return access;
}


struct vmod_access_t *vcache_remove_access(struct vcache_t *vcache, int set, int way, char *access_name)
{
	struct vcache_block_t *block;
	struct vmod_access_t *access;
	struct linked_list_t *access_list;

	assert(IN_RANGE(set, 0, vcache->num_sets - 1));
	assert(IN_RANGE(way, 0, vcache->assoc - 1));

	/* Get block */
	block = &vcache->blocks[set * vcache->assoc + way];
	access_list = block->vmod_access_list;

	/* Find access */
	LINKED_LIST_FOR_EACH(access_list)
	{
		access = linked_list_get(access_list);
		if (!strcmp(vmod_access_get_name(access), access_name))
			break;
	}

	/* Not found */
	if (linked_list_is_end(access_list))
		return NULL;

	/* Remove access and return */
	linked_list_remove(access_list);
	return access;
}


void vcache_set_block(struct vcache_t *vcache, int set, int way,
	unsigned int tag, char *state)
{
	struct vcache_block_t *block;

	if (!IN_RANGE(set, 0, vcache->num_sets - 1))
		fatal("%s: invalid set", __FUNCTION__);
	if (!IN_RANGE(way, 0, vcache->assoc - 1))
		fatal("%s: invalid way", __FUNCTION__);

	block = &vcache->blocks[set * vcache->assoc + way];
	block->tag = tag;
	block->state = map_string(&vcache_block_state_map, state);
}


void vcache_dir_entry_set_sharer(struct vcache_t *vcache,
	int x, int y, int z, int sharer)
{
	struct vcache_dir_entry_t *dir_entry;

	/* Get directory entry */
	assert(IN_RANGE(sharer, 0, vcache->num_sharers - 1));
	dir_entry = vcache_get_dir_entry(vcache, x, y, z);

	/* In a correct trace, sharer should not be set */
	if ((dir_entry->sharers[sharer / 8] & (1 << sharer % 8)))
		panic("%s: sharer already set", __FUNCTION__);

	/* Add sharer */
	assert(dir_entry->num_sharers < vcache->num_sharers);
	dir_entry->sharers[sharer / 8] |= 1 << (sharer % 8);
	dir_entry->num_sharers++;
}


void vcache_dir_entry_clear_sharer(struct vcache_t *vcache,
	int x, int y, int z, int sharer)
{
	struct vcache_dir_entry_t *dir_entry;

	/* Get directory entry */
	assert(IN_RANGE(sharer, 0, vcache->num_sharers - 1));
	dir_entry = vcache_get_dir_entry(vcache, x, y, z);

	/* In a correct trace, sharer should not be set */
	if (!(dir_entry->sharers[sharer / 8] & (1 << sharer % 8)))
		panic("%s: sharer already clear", __FUNCTION__);

	/* Add sharer */
	assert(dir_entry->num_sharers > 0);
	dir_entry->sharers[sharer / 8] &= ~(1 << (sharer % 8));
	dir_entry->num_sharers--;
}


void vcache_dir_entry_clear_all_sharers(struct vcache_t *vcache,
	int x, int y, int z)
{
	struct vcache_dir_entry_t *dir_entry;

	int i;

	/* Clear sharers */
	dir_entry = vcache_get_dir_entry(vcache, x, y, z);
	dir_entry->num_sharers = 0;
	for (i = 0; i < VCACHE_DIR_ENTRY_SHARERS_SIZE(vcache); i++)
		dir_entry->sharers[i] = 0;
}


int vcache_dir_entry_is_sharer(struct vcache_t *vcache,
	int x, int y, int z, int sharer)
{
	struct vcache_dir_entry_t *dir_entry;

	/* Get directory entry */
	assert(IN_RANGE(sharer, 0, vcache->num_sharers - 1));
	dir_entry = vcache_get_dir_entry(vcache, x, y, z);

	/* Return whether sharer is set */
	return (dir_entry->sharers[sharer / 8] & (1 << sharer % 8)) > 0;
}


void vcache_dir_entry_set_owner(struct vcache_t *vcache,
	int x, int y, int z, int owner)
{
	struct vcache_dir_entry_t *dir_entry;

	/* Get directory entry */
	assert(owner == -1 || IN_RANGE(owner, 0, vcache->num_sharers - 1));
	dir_entry = vcache_get_dir_entry(vcache, x, y, z);

	/* Set new owner */
	dir_entry->owner = owner;
}


void vcache_read_checkpoint(struct vcache_t *vcache, FILE *f)
{
	int set;
	int way;

	for (set = 0; set < vcache->num_sets; set++)
	{
		for (way = 0; way < vcache->assoc; way++)
		{
			struct vcache_block_t *block;
			struct vmod_access_t *access;

			unsigned char state;

			int num_accesses;
			int count;
			int i;

			/* Get block */
			block = &vcache->blocks[set * vcache->assoc + way];

			/* Read tag */
			fread(&block->tag, 1, 4, f);

			/* Read state */
			fread(&state, 1, 1, f);
			block->state = state;

			/* Read directory entry */
			for (i = 0; i < vcache->num_sub_blocks; i++)
				vcache_dir_entry_read_checkpoint(vcache, set, way, i, f);

			/* Free previous accesses */
			while (linked_list_count(block->vmod_access_list))
			{
				linked_list_head(block->vmod_access_list);
				vmod_access_free(linked_list_get(block->vmod_access_list));
				linked_list_remove(block->vmod_access_list);
			}

			/* Read number of accesses */
			count = fread(&num_accesses, 1, 4, f);
			if (count != 4)
				fatal("%s: error reading from checkpoint", __FUNCTION__);

			/* Read accesses */
			for (i = 0; i < num_accesses; i++)
			{
				access = vmod_access_create(NULL);
				vmod_access_read_checkpoint(access, f);
				linked_list_add(block->vmod_access_list, access);
			}
		}
	}
}


void vcache_write_checkpoint(struct vcache_t *vcache, FILE *f)
{
	int set;
	int way;

	for (set = 0; set < vcache->num_sets; set++)
	{
		for (way = 0; way < vcache->assoc; way++)
		{
			struct vcache_block_t *block;
			struct vmod_access_t *access;

			unsigned char state;

			int num_accesses;
			int count;
			int i;

			/* Get block */
			block = &vcache->blocks[set * vcache->assoc + way];

			/* Dump tag */
			fwrite(&block->tag, 1, 4, f);

			/* Dump state */
			state = block->state;
			fwrite(&state, 1, 1, f);

			/* Dump directory entry */
			for (i = 0; i < vcache->num_sub_blocks; i++)
				vcache_dir_entry_write_checkpoint(vcache, set, way, i, f);

			/* Write number of accesses */
			num_accesses = linked_list_count(block->vmod_access_list);
			count = fwrite(&num_accesses, 1, 4, f);
			if (count != 4)
				fatal("%s: cannot write to checkpoint file", __FUNCTION__);

			/* Write accesses */
			LINKED_LIST_FOR_EACH(block->vmod_access_list)
			{
				access = linked_list_get(block->vmod_access_list);
				vmod_access_write_checkpoint(access, f);
			}
		}
	}
}


void vcache_refresh(struct vcache_t *vcache)
{
	GtkWidget *layout = vcache->layout;

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

	/* Remove all widgets from layouts */
	while ((child = gtk_container_get_children(GTK_CONTAINER(layout))))
		gtk_container_remove(GTK_CONTAINER(layout), child->data);
	while ((child = gtk_container_get_children(GTK_CONTAINER(vcache->first_row_layout))))
		gtk_container_remove(GTK_CONTAINER(vcache->first_row_layout), child->data);
	while ((child = gtk_container_get_children(GTK_CONTAINER(vcache->first_col_layout))))
		gtk_container_remove(GTK_CONTAINER(vcache->first_col_layout), child->data);

	/* Get allocated dimensions */
	width = gtk_widget_get_allocated_width(layout);
	height = gtk_widget_get_allocated_height(layout);
	vcache->width = width;
	vcache->height = height;

	/* Dimensions */
	table_width = VCACHE_CELL_WIDTH * vcache->assoc;
	table_height = VCACHE_CELL_HEIGHT * vcache->num_sets;

	/* Horizontal scroll bar */
	if (table_width > width)
	{
		gtk_range_set_range(GTK_RANGE(vcache->hscrollbar), 0, table_width - width);
		gtk_range_set_increments(GTK_RANGE(vcache->hscrollbar),
			VCACHE_CELL_WIDTH / 3, width - VCACHE_CELL_WIDTH / 3);
		gtk_widget_set_visible(vcache->hscrollbar, TRUE);
	}
	else
		gtk_widget_set_visible(vcache->hscrollbar, FALSE);

	/* Vertical scroll bar */
	if (table_height > height)
	{
		gtk_range_set_range(GTK_RANGE(vcache->vscrollbar), 0, table_height - height);
		gtk_range_set_increments(GTK_RANGE(vcache->vscrollbar),
			VCACHE_CELL_HEIGHT, height - VCACHE_CELL_HEIGHT);
		gtk_widget_set_visible(vcache->vscrollbar, TRUE);
	}
	else
		gtk_widget_set_visible(vcache->vscrollbar, FALSE);

	/* Get starting X position */
	left = gtk_range_get_value(GTK_RANGE(vcache->hscrollbar));
	left_way = left / VCACHE_CELL_WIDTH;
	left_way_offset = -(left % VCACHE_CELL_WIDTH);

	/* Get starting Y position */
	top = gtk_range_get_value(GTK_RANGE(vcache->vscrollbar));
	top_set = top / VCACHE_CELL_HEIGHT;
	top_set_offset = -(top % VCACHE_CELL_HEIGHT);

	/* First row */
	way = left_way;
	x = left_way_offset;
	while (x < width && way < vcache->assoc)
	{
		snprintf(str, sizeof str, "%d", way);
		GtkWidget *label = gtk_label_new(str);
		gtk_widget_set_size_request(label, VCACHE_CELL_WIDTH - 1, VCACHE_FIRST_ROW_HEIGHT - 1);
		gtk_widget_show(label);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(VCACHE_FONT_SIZE << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(event_box), label);
		gtk_layout_put(GTK_LAYOUT(vcache->first_row_layout), event_box, x, 0);
		gtk_widget_show(event_box);

		/* Color */
		GdkColor color;
		gdk_color_parse("#ffffff", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

		/* Next way */
		x += VCACHE_CELL_WIDTH;
		way++;
	}

	/* First column */
	set = top_set;
	y = top_set_offset;
	while (y < height && set < vcache->num_sets)
	{
		snprintf(str, sizeof str, "%d", set);
		GtkWidget *label = gtk_label_new(str);
		gtk_widget_set_size_request(label, VCACHE_FIRST_COL_WIDTH - 1, VCACHE_CELL_HEIGHT - 1);
		gtk_widget_show(label);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(VCACHE_FONT_SIZE << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(event_box), label);
		gtk_layout_put(GTK_LAYOUT(vcache->first_col_layout), event_box, 0, y);
		gtk_widget_show(event_box);

		/* Color */
		GdkColor color;
		gdk_color_parse("#ffffff", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

		/* Next set */
		y += VCACHE_CELL_HEIGHT;
		set++;
	}

	/* Blocks */
	set = top_set;
	y = top_set_offset;
	while (y < height && set < vcache->num_sets)
	{
		/* Ways */
		way = left_way;
		x = left_way_offset;
		while (x < width && way < vcache->assoc)
		{
			struct vcache_block_t *block;

			char *state_str;

			int num_sharers;
			int num_accesses;

			GtkWidget *label;
			GtkWidget *event_box;

			PangoAttrList *attrs;
			PangoAttribute *size_attr;

			/* Get block properties */
			assert(IN_RANGE(set, 0, vcache->num_sets - 1));
			assert(IN_RANGE(way, 0, vcache->assoc - 1));
			block = &vcache->blocks[set * vcache->assoc + way];
			state_str = map_value(&vcache_block_state_map, block->state);

			/* Tag label */
			snprintf(str, sizeof str, "0x%x (%s)", block->tag, state_str);
			label = gtk_label_new(str);
			gtk_widget_set_size_request(label, VCACHE_LABEL_BLOCK_WIDTH, VCACHE_CELL_HEIGHT - 1);
			gtk_widget_show(label);

			/* Set label font attributes */
			attrs = pango_attr_list_new();
			size_attr = pango_attr_size_new_absolute(VCACHE_FONT_SIZE << 10);
			pango_attr_list_insert(attrs, size_attr);
			gtk_label_set_attributes(GTK_LABEL(label), attrs);

			/* Event box */
			event_box = gtk_event_box_new();
			gtk_container_add(GTK_CONTAINER(event_box), label);
			gtk_layout_put(GTK_LAYOUT(layout), event_box, x + VCACHE_LABEL_BLOCK_LEFT, y);
			gtk_widget_show(event_box);

			/* Background color */
			GdkColor color;
			assert(IN_RANGE(block->state, 0, VCACHE_NUM_BLOCK_STATE_COLORS - 1));
			gdk_color_parse(vcache_block_state_color[block->state], &color);
			gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

			/* Sharers text */
			num_sharers = vcache_block_get_num_sharers(vcache, set, way);
			snprintf(str, sizeof str, "+%d", num_sharers);
			if (!num_sharers)
				strcpy(str, "-");

			/* Sharers label */
			label = gtk_label_new(str);
			gtk_widget_set_size_request(label, VCACHE_LABEL_SHARERS_WIDTH, VCACHE_CELL_HEIGHT - 1);
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

			/* Next way */
			x += VCACHE_CELL_WIDTH;
			way++;
		}

		/* Next set */
		y += VCACHE_CELL_HEIGHT;
		set++;
	}

	/* Repaint if necessary */
	gtk_container_check_resize(GTK_CONTAINER(vcache->layout));
	gtk_container_check_resize(GTK_CONTAINER(vcache->first_row_layout));
	gtk_container_check_resize(GTK_CONTAINER(vcache->first_col_layout));
}


GtkWidget *vcache_get_widget(struct vcache_t *vcache)
{
	return vcache->widget;
}

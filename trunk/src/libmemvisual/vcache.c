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

#include <memvisual-private.h>

#define SIZE_OF_VCACHE_DIR_ENTRY(vcache) (sizeof(struct vcache_dir_entry_t) + \
	((vcache)->num_sharers + 7) / 8)

#define VCACHE_CELL_WIDTH		150
#define VCACHE_CELL_HEIGHT		20

#define VCACHE_FIRST_ROW_HEIGHT		20
#define VCACHE_FIRST_COL_WIDTH		100

#define VCACHE_FONT_SIZE		12


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


static void vcache_refresh(struct vcache_t *vcache)
{
	GtkWidget *layout = vcache->layout;

	GList *child;

	int width;
	int height;

	int cell_width;
	int cell_height;
	int cell_font_size;

	int table_width;
	int table_height;

	int block_width;

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
	cell_width = VCACHE_CELL_WIDTH;
	cell_height = VCACHE_CELL_HEIGHT;
	cell_font_size = VCACHE_FONT_SIZE;

	table_width = cell_width * vcache->assoc;
	table_height = cell_height * vcache->num_sets;

	block_width = 100;

	/* Horizontal scroll bar */
	if (table_width > width)
	{
		gtk_range_set_range(GTK_RANGE(vcache->hscrollbar), 0, table_width - width);
		gtk_range_set_increments(GTK_RANGE(vcache->hscrollbar), cell_width / 3, width - cell_width / 3);
		gtk_widget_set_visible(vcache->hscrollbar, TRUE);
	}
	else
		gtk_widget_set_visible(vcache->hscrollbar, FALSE);

	/* Vertical scroll bar */
	if (table_height > height)
	{
		gtk_range_set_range(GTK_RANGE(vcache->vscrollbar), 0, table_height - height);
		gtk_range_set_increments(GTK_RANGE(vcache->vscrollbar), cell_height, height - cell_height);
		gtk_widget_set_visible(vcache->vscrollbar, TRUE);
	}
	else
		gtk_widget_set_visible(vcache->vscrollbar, FALSE);

	/* Get starting X position */
	left = gtk_range_get_value(GTK_RANGE(vcache->hscrollbar));
	left_way = left / cell_width;
	left_way_offset = -(left % cell_width);

	/* Get starting Y position */
	top = gtk_range_get_value(GTK_RANGE(vcache->vscrollbar));
	top_set = top / cell_height;
	top_set_offset = -(top % cell_height);

	/* First row */
	way = left_way;
	x = left_way_offset;
	while (x < width && way < vcache->assoc)
	{
		snprintf(str, sizeof str, "%d", way);
		GtkWidget *label = gtk_label_new(str);
		gtk_widget_set_size_request(label, cell_width - 1, VCACHE_FIRST_ROW_HEIGHT - 1);
		gtk_widget_show(label);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(cell_font_size << 10);
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
		x += cell_width;
		way++;
	}

	/* First column */
	set = top_set;
	y = top_set_offset;
	while (y < height && set < vcache->num_sets)
	{
		snprintf(str, sizeof str, "%d", set);
		GtkWidget *label = gtk_label_new(str);
		gtk_widget_set_size_request(label, VCACHE_FIRST_COL_WIDTH - 1, cell_height - 1);
		gtk_widget_show(label);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(cell_font_size << 10);
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
		y += cell_height;
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

			/* Get block properties */
			assert(IN_RANGE(set, 0, vcache->num_sets - 1));
			assert(IN_RANGE(way, 0, vcache->assoc - 1));
			block = &vcache->blocks[set * vcache->assoc + way];
			state_str = map_value(&vcache_block_state_map, block->state);

			/* Label */
			snprintf(str, sizeof str, "0x%x (%s)", block->tag, state_str);
			GtkWidget *label = gtk_label_new(str);
			gtk_widget_set_size_request(label, block_width, cell_height - 1);
			gtk_widget_show(label);

			/* Set label font attributes */
			PangoAttrList *attrs;
			attrs = pango_attr_list_new();
			PangoAttribute *size_attr = pango_attr_size_new_absolute(cell_font_size << 10);
			pango_attr_list_insert(attrs, size_attr);
			gtk_label_set_attributes(GTK_LABEL(label), attrs);

			/* Event box */
			GtkWidget *event_box = gtk_event_box_new();
			gtk_container_add(GTK_CONTAINER(event_box), label);
			gtk_layout_put(GTK_LAYOUT(layout), event_box, x, y);
			gtk_widget_show(event_box);

			GdkColor color;
			assert(IN_RANGE(block->state, 0, VCACHE_NUM_BLOCK_STATE_COLORS - 1));
			gdk_color_parse(vcache_block_state_color[block->state], &color);
			gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

			/* Next way */
			x += cell_width;
			way++;
		}

		/* Next set */
		y += cell_height;
		set++;
	}

	/* Repaint if necessary */
	gtk_container_check_resize(GTK_CONTAINER(vcache->layout));
	gtk_container_check_resize(GTK_CONTAINER(vcache->first_row_layout));
	gtk_container_check_resize(GTK_CONTAINER(vcache->first_col_layout));
}


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


struct vcache_t *vcache_create(char *name, int num_sets, int assoc, int block_size,
	int sub_block_size, int num_sharers)
{
	int i;

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
	vcache->num_sets = num_sets;
	vcache->assoc = assoc;
	vcache->block_size = block_size;
	vcache->sub_block_size = sub_block_size;
	vcache->num_sub_blocks = block_size / sub_block_size;
	vcache->num_sharers = num_sharers;
	assert(block_size % sub_block_size == 0);

	/* Create block array */
	vcache->blocks = calloc(num_sets * assoc, sizeof(struct vcache_block_t));
	for (i = 0; i < num_sets * assoc; i++)
	{
		block = &vcache->blocks[i];
		block->dir_entries = calloc(vcache->num_sub_blocks, SIZE_OF_VCACHE_DIR_ENTRY(vcache));
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
	gtk_widget_set_size_request(layout, 200, 200);
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
	int i;

	/* Free blocks */
	for (i = 0; i < vcache->num_sets * vcache->assoc; i++)
		free(vcache->blocks[i].dir_entries);
	free(vcache->blocks);

	/* Free object */
	free(vcache->name);
	free(vcache);
}


struct vcache_dir_entry_t *vcache_get_dir_entry(struct vcache_t *vcache, int set, int way, int sub_block)
{
	struct vcache_block_t *block;
	struct vcache_dir_entry_t *dir_entry;

	assert(IN_RANGE(set, 0, vcache->num_sets - 1));
	assert(IN_RANGE(way, 0, vcache->assoc - 1));
	assert(IN_RANGE(sub_block, 0, vcache->num_sub_blocks - 1));

	block = &vcache->blocks[set * vcache->assoc + way];
	dir_entry = (struct vcache_dir_entry_t *) (((void *) block->dir_entries) + SIZE_OF_VCACHE_DIR_ENTRY(vcache) * sub_block);

	return dir_entry;
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

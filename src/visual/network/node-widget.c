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

#include <math.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/matrix.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <visual/common/cycle-bar.h>

#include "buffer.h"
#include "link.h"
#include "node.h"
#include "net-packet.h"
#include "node-widget.h"

#define VI_NODE_CONTENT_CELL_WIDTH              100
#define VI_NODE_CONTENT_CELL_HEIGHT             20

#define VI_NODE_CONTENT_LAYOUT_WIDTH            300
#define VI_NODE_CONTENT_LAYOUT_HEIGHT           200

#define VI_NODE_BUFFER_TABLE_LAYOUT_WIDTH       150
#define VI_NODE_BUFFER_TABLE_LAYOUT_HEIGHT      VI_NODE_CONTENT_LAYOUT_HEIGHT

#define VI_NODE_BUFFER_QUEUE_LAYOUT_WIDTH       VI_NODE_CONTENT_LAYOUT_WIDTH
#define VI_NODE_BUFFER_QUEUE_LAYOUT_HEIGHT      VI_NODE_CONTENT_CELL_HEIGHT

#define VI_NODE_CONTENT_FONT_SIZE               12

static void vi_node_widget_destroy(GtkWidget *widget, struct vi_node_widget_t *node_widget);
static int vi_max_buffer_queue_size (struct vi_net_node_t *node);

GtkWidget *vi_node_widget_get_widget(struct vi_node_widget_t *node_widget)
{
	return node_widget->widget;
}

static void vi_node_widget_size_allocate(GtkWidget *widget, GdkRectangle *allocation,
		struct vi_node_widget_t *node_widget)
{

	vi_node_widget_refresh(node_widget);

}

static void vi_node_widget_scroll_bar_value_changed(GtkRange *range,
		struct vi_node_widget_t *node_widget)
{
	vi_node_widget_refresh(node_widget);
}


static gboolean vi_node_widget_scroll(GtkWidget *widget, GdkEventScroll *event,
		struct vi_node_widget_t *node_widget)
{
	long long value;

	value = gtk_range_get_value(GTK_RANGE(node_widget->vscrollbar));
	if (event->direction == GDK_SCROLL_UP)
		value -= 10;
	else
		value += 10;
	gtk_range_set_value(GTK_RANGE(node_widget->vscrollbar), value);

	return FALSE;
}

static void vi_node_widget_destroy(GtkWidget *widget, struct vi_node_widget_t *node_widget)
{
	vi_node_widget_free(node_widget);
}


void vi_node_widget_free(struct vi_node_widget_t *node_widget)
{
	/* Content label matrix */
	if (node_widget->content_matrix)
		matrix_free(node_widget->content_matrix);

	/* buffer list */
	if (node_widget->buffer_list)
		list_free(node_widget->buffer_list);

	/* buffer queue list */
	if (node_widget->buffer_queue_list)
		list_free(node_widget->buffer_queue_list);

	if (node_widget->buffer_title_list)
		list_free(node_widget->buffer_title_list);

	/* Free widget */
	free(node_widget);
}


static void vi_node_widget_refresh_scrollbars(struct vi_node_widget_t *node_widget)
{
	struct vi_net_node_t *node = node_widget->node ;

	long long num_packets;
	long long num_buffers;
	int table_width;
	int table_height;

	/* Dimensions */
	num_packets = vi_max_buffer_queue_size(node);
	num_buffers = hash_table_count(node->input_buffer_list) + hash_table_count(node->output_buffer_list);
	table_width = VI_NODE_CONTENT_CELL_WIDTH * num_packets;
	table_height = VI_NODE_CONTENT_CELL_HEIGHT * num_buffers;

	/* Horizontal bar */
	if (table_width > node_widget->content_layout_width)
	{
		gtk_range_set_range(GTK_RANGE(node_widget->hscrollbar), 0,
				table_width - node_widget->content_layout_width);
		gtk_range_set_increments(GTK_RANGE(node_widget->hscrollbar),
				VI_NODE_CONTENT_CELL_WIDTH / 3, node_widget->content_layout_width
				- VI_NODE_CONTENT_CELL_WIDTH / 3);
		gtk_widget_set_visible(node_widget->hscrollbar, TRUE);
	}
	else
	{
		gtk_widget_set_visible(node_widget->hscrollbar, FALSE);
	}

	/* Vertical bar */
	if (table_height > node_widget->content_layout_height)
	{
		gtk_range_set_range(GTK_RANGE(node_widget->vscrollbar), 0,
				table_height - node_widget->content_layout_height);
		gtk_range_set_increments(GTK_RANGE(node_widget->vscrollbar),
				VI_NODE_CONTENT_CELL_HEIGHT, node_widget->content_layout_height
				- VI_NODE_CONTENT_CELL_HEIGHT);
		gtk_widget_set_visible(node_widget->vscrollbar, TRUE);
	}
	else
	{
		gtk_widget_set_visible(node_widget->vscrollbar, FALSE);
	}
}


static void vi_node_widget_refresh_content_layout(struct vi_node_widget_t *node_widget)
{
	GtkWidget *content_layout;
	GtkWidget *content_table;

	struct vi_net_node_t *node = node_widget->node;

	int num_rows;
	int num_cols;

	int row;
	int col;

	long long num_packets;
	long long num_buffers;

	/* Get new dimensions */
	num_packets = vi_max_buffer_queue_size(node);
	num_buffers = hash_table_count(node->input_buffer_list) + hash_table_count(node->output_buffer_list);
	num_rows = MIN(num_buffers, node_widget->content_layout_height / VI_NODE_CONTENT_CELL_HEIGHT + 2);
	num_cols = MIN(num_packets, node_widget->content_layout_width / VI_NODE_CONTENT_CELL_WIDTH + 2);

	/* Create new matrix */
	if (node_widget->content_matrix)
		matrix_free(node_widget->content_matrix);
	node_widget->content_matrix = matrix_create(num_rows, num_cols);

	/* Create new table */
	content_layout = node_widget->content_layout;
	if (node_widget->content_table)
		gtk_widget_destroy(node_widget->content_table);
	content_table = gtk_table_new(num_rows, num_cols, TRUE);
	gtk_layout_put(GTK_LAYOUT(content_layout), content_table,
			node_widget->left_offset, node_widget->top_offset);
	node_widget->content_table = content_table;

	/* Create labels */
	MATRIX_FOR_EACH(node_widget->content_matrix, row, col)
	{
		/* Label */
		GtkWidget *label = gtk_label_new(NULL);
		gtk_misc_set_alignment(GTK_MISC(label), .5, .5);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(VI_NODE_CONTENT_FONT_SIZE << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(event_box), label);
		matrix_set(node_widget->content_matrix, row, col, event_box);

		/* Frame */
		GtkWidget *frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		gtk_container_add(GTK_CONTAINER(frame), event_box);
		gtk_table_attach_defaults(GTK_TABLE(content_table), frame, col, col + 1, row, row + 1);
		gtk_widget_set_size_request(frame, VI_NODE_CONTENT_CELL_WIDTH,
				VI_NODE_CONTENT_CELL_HEIGHT);

		/* Color */
		GdkColor color;
		gdk_color_parse("#ffffff", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
	}

	/* Show all widgets */
	gtk_widget_show_all(content_table);
	gtk_container_check_resize(GTK_CONTAINER(content_layout));
}

static void vi_node_widget_refresh_buffer_title_layout(struct vi_node_widget_t *node_widget)
{
	GtkWidget *buffer_table_title_layout;
	GtkWidget *buffer_title;

	int col;
	int num_cols = MIN(6, node_widget->buffer_table_layout_width /  VI_NODE_CONTENT_CELL_WIDTH + 2);

	/* Clear title list */
	if (!node_widget->buffer_title_list)
		node_widget->buffer_title_list = list_create();
	list_clear(node_widget->buffer_title_list);

	/* Create New Table */
	buffer_table_title_layout = node_widget->buffer_table_title_layout;
	if (node_widget->buffer_title)
		gtk_widget_destroy(node_widget->buffer_title);
	buffer_title = gtk_table_new(1, num_cols, FALSE);
	gtk_layout_put(GTK_LAYOUT(buffer_table_title_layout), buffer_title, 0, node_widget->top_offset);
	node_widget->buffer_title = buffer_title;

	for (col = 0; col < num_cols ; col++)
	{
		/* Label */

		float width;
		if (col == 1)
			width = 2 * (float) VI_NODE_CONTENT_CELL_WIDTH;
		else
			width = VI_NODE_CONTENT_CELL_WIDTH;
		GtkWidget *label = gtk_label_new(NULL);
		gtk_misc_set_alignment(GTK_MISC(label), .5, .5);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(VI_NODE_CONTENT_FONT_SIZE << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box = gtk_event_box_new();
		//                        gtk_widget_set_size_request(event_box, width,
		//                        VI_NODE_CONTENT_CELL_HEIGHT - 1);
		gtk_container_add(GTK_CONTAINER(event_box), label);
		list_add(node_widget->buffer_title_list, event_box);

		/* Frame */
		GtkWidget *frame = gtk_frame_new(NULL);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
		gtk_container_add(GTK_CONTAINER(frame), event_box);
		gtk_table_attach_defaults(GTK_TABLE(buffer_title), frame, col ,  col + 1, 0, 1);
		gtk_widget_set_size_request(frame,  width,
				VI_NODE_CONTENT_CELL_HEIGHT);
		/* Color */
		GdkColor color;
		gdk_color_parse("#eeeeee", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
	}
	/* Show all widgets */
	gtk_widget_show_all(buffer_title);
	gtk_container_check_resize(GTK_CONTAINER(buffer_table_title_layout));

}
static void vi_node_widget_refresh_buffer_table_layout(struct vi_node_widget_t *node_widget)
{
	GtkWidget *buffer_table_layout;
	GtkWidget *buffer_table;

	struct vi_net_node_t *node = node_widget->node;

	int num_rows;
	int row;

	long long num_buffers;

	/* Get new dimensions */
	num_buffers = hash_table_count(node->input_buffer_list) + hash_table_count(node->output_buffer_list);
	num_rows = MIN(num_buffers, node_widget->buffer_table_layout_height / VI_NODE_CONTENT_CELL_HEIGHT + 2);

	/* Clear instruction label list */
	if (!node_widget->buffer_list)
		node_widget->buffer_list = list_create();
	list_clear(node_widget->buffer_list);

	/* Create new table */
	buffer_table_layout = node_widget->buffer_table_layout;
	if (node_widget->buffer_table)
		gtk_widget_destroy(node_widget->buffer_table);
	buffer_table = gtk_table_new(num_rows, 6, FALSE);
	gtk_layout_put(GTK_LAYOUT(buffer_table_layout), buffer_table, 0, node_widget->top_offset);
	node_widget->buffer_table = buffer_table;

	/* Create labels */
	for (row = 0; row < num_rows; row++)
	{
		for (int col = 0; col < 6; col++ )
		{
			/* Label */

			float width;
			if (col == 1)
				width = 2 * (float) VI_NODE_CONTENT_CELL_WIDTH;
			else
				width = VI_NODE_CONTENT_CELL_WIDTH;
			GtkWidget *label = gtk_label_new(NULL);
			gtk_misc_set_alignment(GTK_MISC(label), .5, .5);

			/* Set label font attributes */
			PangoAttrList *attrs;
			attrs = pango_attr_list_new();
			PangoAttribute *size_attr = pango_attr_size_new_absolute(VI_NODE_CONTENT_FONT_SIZE << 10);
			pango_attr_list_insert(attrs, size_attr);
			gtk_label_set_attributes(GTK_LABEL(label), attrs);

			/* Event box */
			GtkWidget *event_box = gtk_event_box_new();
			//                        gtk_widget_set_size_request(event_box, width,
			//                        VI_NODE_CONTENT_CELL_HEIGHT - 1);
			gtk_container_add(GTK_CONTAINER(event_box), label);
			list_add(node_widget->buffer_list, event_box);

			/* Frame */
			GtkWidget *frame = gtk_frame_new(NULL);
			gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
			gtk_container_add(GTK_CONTAINER(frame), event_box);
			gtk_table_attach_defaults(GTK_TABLE(buffer_table), frame, col ,  col + 1, row, row + 1);
			gtk_widget_set_size_request(frame,  width,
					VI_NODE_CONTENT_CELL_HEIGHT);
			/* Color */
			GdkColor color;
			gdk_color_parse("#c6f729", &color);
			gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
		}
	}

	/* Show all widgets */
	gtk_widget_show_all(buffer_table);
	gtk_container_check_resize(GTK_CONTAINER(buffer_table_layout));
}

static int vi_max_buffer_queue_size (struct vi_net_node_t *node)
{
	int max_count = 1;
	char *buffer_name;
	struct vi_net_buffer_t *buffer;
	HASH_TABLE_FOR_EACH(node->input_buffer_list, buffer_name, buffer)
	{
		if (list_count(buffer->packet_list) > max_count)
			max_count = list_count(buffer->packet_list);
	}
	HASH_TABLE_FOR_EACH(node->output_buffer_list, buffer_name, buffer)
	{
		if (list_count(buffer->packet_list) > max_count)
			max_count = list_count(buffer->packet_list);
	}
	return max_count;
}

static void vi_node_widget_refresh_buffer_queue_layout(struct vi_node_widget_t *node_widget)
{
	GtkWidget *buffer_queue_layout;
	GtkWidget *buffer_queue;

	struct vi_net_node_t *node = node_widget->node;
	int num_cols;
	int col;

	int num_packets;

	/* Get new dimensions */
	num_packets = vi_max_buffer_queue_size(node);
	num_cols = MIN(num_packets, node_widget->content_layout_width / VI_NODE_CONTENT_CELL_WIDTH + 2);

	/* Clear cycle label list */
	if (!node_widget->buffer_queue_list)
		node_widget->buffer_queue_list = list_create();
	list_clear(node_widget->buffer_queue_list);

	/* Create new table */
	buffer_queue_layout = node_widget->buffer_queue_layout;
	if (node_widget->buffer_queue)
		gtk_widget_destroy(node_widget->buffer_queue);
	buffer_queue = gtk_table_new(1, num_cols, TRUE);
	gtk_layout_put(GTK_LAYOUT(buffer_queue_layout), buffer_queue, node_widget->left_offset, 0);
	node_widget->buffer_queue = buffer_queue;

	/* Create labels */
	for (col = 0; col < num_cols; col++)
	{
		/* Label */
		GtkWidget *label = gtk_label_new(NULL);
		gtk_misc_set_alignment(GTK_MISC(label), .5, .5);
		gtk_widget_set_size_request(label, VI_NODE_CONTENT_CELL_WIDTH - 1,
				VI_NODE_CONTENT_CELL_HEIGHT - 1);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(VI_NODE_CONTENT_FONT_SIZE << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(event_box), label);
		gtk_table_attach_defaults(GTK_TABLE(buffer_queue), event_box, col, col + 1, 0, 1);
		list_add(node_widget->buffer_queue_list, event_box);

		/* Color */
		GdkColor color;
		gdk_color_parse("#ffffff", &color);
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
	}

	/* Show all widgets */
	gtk_widget_show_all(buffer_queue);
	gtk_container_check_resize(GTK_CONTAINER(buffer_queue_layout));
}


void vi_node_widget_refresh_content(struct vi_node_widget_t *node_widget)
{
	struct vi_net_node_t *node;
	struct vi_net_buffer_t *buffer;

	int left_packet;
	int top_buffer;

	char *buffer_name;

	int col;
	int row = 0;

	char str[MAX_LONG_STRING_SIZE];

	/* Get variables */
	node = node_widget->node;
	left_packet = node_widget->left_packet;
	top_buffer  = node_widget->top_buffer;

	/* Cycle layout */
	LIST_FOR_EACH(node_widget->buffer_queue_list, col)
	{
		/* Event box */
		GtkWidget *event_box;
		event_box = list_get(node_widget->buffer_queue_list, col);
		if (!event_box)
			continue;

		/* Label */
		GtkWidget *label;
		label = gtk_bin_get_child(GTK_BIN(event_box));

		/* Text */
		snprintf(str, sizeof str, "%d", left_packet + col);
		gtk_label_set_text(GTK_LABEL(label), str);
	}

	/* Title Layout */
	LIST_FOR_EACH(node_widget->buffer_title_list, col)
	{
		/* Event Box */
		GtkWidget *event_box;
		event_box = list_get(node_widget->buffer_title_list, col);
		if (!event_box)
			continue;

		/* Label */
		GtkWidget *label;
		label = gtk_bin_get_child(GTK_BIN(event_box));

		/* Text */
		snprintf(str, sizeof str, "%s", str_map_value(&vi_net_buffer_attrib_map, col));
		gtk_label_set_text(GTK_LABEL(label), str);
	}

	/* Clear Content Matrix */
	MATRIX_FOR_EACH(node_widget->content_matrix, row, col)
	{
		/* Event box */
		GtkWidget *event_box;
		event_box = matrix_get(node_widget->content_matrix,row, col);
		if (!event_box)
			continue;

		/* Label */
		GtkWidget *label;
		label = gtk_bin_get_child(GTK_BIN(event_box));
		gtk_label_set_text(GTK_LABEL(label), NULL);
	}
	/* Buffer List */
	HASH_TABLE_FOR_EACH(node->input_buffer_list, buffer_name, buffer)
	{
		int buffer_row;

		/* Buffer is in range */
		buffer_row = buffer->id - top_buffer;
		if (!IN_RANGE(buffer_row, 0, list_count(node_widget->buffer_list) - 1))
			continue;

		/* Event box */
		GtkWidget *event_box;
		/* Label */
		GtkWidget *label;

		/* Buffer Direction */
		event_box = list_get(node_widget->buffer_list, buffer_row * 6);
		label = gtk_bin_get_child(GTK_BIN(event_box));
		snprintf(str, sizeof str, "%s", str_map_value(&vi_net_buffer_direction_map,
				buffer->direction));
		gtk_label_set_label(GTK_LABEL(label), str);

		/* Buffer Connection */
		event_box = list_get(node_widget->buffer_list, buffer_row * 6 + 1);
		label = gtk_bin_get_child(GTK_BIN(event_box));
		snprintf(str, sizeof str, "%s", buffer->link->name);
		gtk_label_set_label(GTK_LABEL(label), str);

		/* Connection Utilization */
		float utilization = (float) buffer->link->transferred_bytes/ (vi_cycle_bar_get_cycle() * buffer->link->bandwidth);
		if (utilization > 1)
			utilization = 1;
		if (utilization < 0)
			utilization = 0;

		event_box = list_get(node_widget->buffer_list, buffer_row * 6 + 2);
		label = gtk_bin_get_child(GTK_BIN(event_box));
		snprintf(str, sizeof str, "%.2f", utilization);
		gtk_label_set_label(GTK_LABEL(label), str);

		/* Buffer Name */
		event_box = list_get(node_widget->buffer_list, buffer_row * 6 + 3);
		label = gtk_bin_get_child(GTK_BIN(event_box));
		snprintf(str, sizeof str, "%s", buffer->name);
		gtk_label_set_label(GTK_LABEL(label), str);

		/* Buffer Size */
		event_box = list_get(node_widget->buffer_list, buffer_row * 6 + 4);
		label = gtk_bin_get_child(GTK_BIN(event_box));
		snprintf(str, sizeof str, "%d", buffer->size);
		gtk_label_set_label(GTK_LABEL(label), str);

		/* Buffer Utilization */
		event_box = list_get(node_widget->buffer_list, buffer_row * 6 + 5);
		label = gtk_bin_get_child(GTK_BIN(event_box));
		snprintf(str, sizeof str, "%.2f", (float) buffer->occupancy / buffer->size);
		gtk_label_set_label(GTK_LABEL(label), str);

		MATRIX_FOR_EACH_COLUMN(node_widget->content_matrix, col)
		{
			/* Event Box */
			GtkWidget *event_box;
			event_box = matrix_get(node_widget->content_matrix, buffer_row, col);
			if (!event_box)
				continue;

			struct vi_net_packet_t *packet;
			packet = list_get(buffer->packet_list, col + left_packet);
			if (packet)
			{
				label = gtk_bin_get_child(GTK_BIN(event_box));
				if (packet->stage != vi_packet_dont_care)
					snprintf(str, sizeof str, "%s(%s)", packet->name,
							str_map_value(&vi_packet_stage_map, packet->stage));
				else
					snprintf(str, sizeof str, "%s", packet->name);
				gtk_label_set_label(GTK_LABEL(label), str);
			}
			/* Color */
			GdkColor color;
			char *color_str;
			color_str = packet ? str_map_value(&vi_packet_stage_color_map, packet->stage) : "#EEEEEE";
			gdk_color_parse(color_str, &color);
			gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

		}
		/* Next Buffer */
		row ++;
	}
	HASH_TABLE_FOR_EACH(node->output_buffer_list, buffer_name, buffer)
	{
		int buffer_row;

		/* Buffer is in range */
		buffer_row = buffer->id + hash_table_count(node->input_buffer_list)- top_buffer;
		if (!IN_RANGE(buffer_row, 0, list_count(node_widget->buffer_list) - 1))
			continue;

		/* Event box */
		GtkWidget *event_box;
		/* Label */
		GtkWidget *label;

		/* Buffer Direction */
		/* Buffer Direction */
		event_box = list_get(node_widget->buffer_list, buffer_row * 6);
		label = gtk_bin_get_child(GTK_BIN(event_box));
		snprintf(str, sizeof str, "%s", str_map_value(&vi_net_buffer_direction_map,
				buffer->direction));
		gtk_label_set_label(GTK_LABEL(label), str);

		/* Buffer Connection */
		event_box = list_get(node_widget->buffer_list, buffer_row * 6 + 1);
		label = gtk_bin_get_child(GTK_BIN(event_box));
		snprintf(str, sizeof str, "%s", buffer->link->name);
		gtk_label_set_label(GTK_LABEL(label), str);

		/* Connection Utilization */
		float utilization = 0;
		utilization = (float) buffer->link->transferred_bytes/ (vi_cycle_bar_get_cycle() * buffer->link->bandwidth);
		if (utilization > 1)
			utilization = 1;
		if (utilization < 0)
			utilization = 0;
		event_box = list_get(node_widget->buffer_list, buffer_row * 6 + 2);
		label = gtk_bin_get_child(GTK_BIN(event_box));
		snprintf(str, sizeof str, "%.2f", utilization);
		gtk_label_set_label(GTK_LABEL(label), str);

		/* Buffer Name */
		event_box = list_get(node_widget->buffer_list, buffer_row * 6 + 3);
		label = gtk_bin_get_child(GTK_BIN(event_box));
		snprintf(str, sizeof str, "%s", buffer->name);
		gtk_label_set_label(GTK_LABEL(label), str);

		/* Buffer Size */
		event_box = list_get(node_widget->buffer_list, buffer_row * 6 + 4);
		label = gtk_bin_get_child(GTK_BIN(event_box));
		snprintf(str, sizeof str, "%d", buffer->size);
		gtk_label_set_label(GTK_LABEL(label), str);

		/* Buffer Utilization */
		event_box = list_get(node_widget->buffer_list, buffer_row * 6 + 5);
		label = gtk_bin_get_child(GTK_BIN(event_box));
		snprintf(str, sizeof str, "%.2f", (float) buffer->occupancy / buffer->size);
		gtk_label_set_label(GTK_LABEL(label), str);

		MATRIX_FOR_EACH_COLUMN(node_widget->content_matrix, col)
		{
			/* Event Box */
			GtkWidget *event_box;
			event_box = matrix_get(node_widget->content_matrix, buffer_row, col);
			if (!event_box)
				continue;

			struct vi_net_packet_t *packet;
			packet = list_get(buffer->packet_list, col + left_packet);
			if (packet)
			{
				label = gtk_bin_get_child(GTK_BIN(event_box));
				if (packet->stage != vi_packet_dont_care)
					snprintf(str, sizeof str, "%s(%s)", packet->name,
							str_map_value(&vi_packet_stage_map, packet->stage));
				else
					snprintf(str, sizeof str, "%s", packet->name);
				gtk_label_set_label(GTK_LABEL(label), str);
			}
			/* Color */
			GdkColor color;
			char *color_str;
			color_str = packet ? str_map_value(&vi_packet_stage_color_map, packet->stage) : "#EEEEEE";
			gdk_color_parse(color_str, &color);
			gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);

		}
		/* Next Buffer */
		row ++;
	}
/*
	HASH_TABLE_FOR_EACH(node->input_buffer_list, buffer_name, buffer)
	{
		int i;
		struct vi_net_packet_t *packet;

		fprintf(stderr,"buffer %s:%s content:", node->name, buffer->name);
		LIST_FOR_EACH(buffer->packet_list, i)
		{
			packet = list_get(buffer->packet_list, i);
			fprintf(stderr, " | %s |", packet->name);
		}
		fprintf(stderr, "\n");
	}

	HASH_TABLE_FOR_EACH(node->output_buffer_list, buffer_name, buffer)
	{
		int i;
		struct vi_net_packet_t *packet;

		fprintf(stderr,"buffer %s:%s content:", node->name, buffer->name);
		LIST_FOR_EACH(buffer->packet_list, i)
		{
			packet = list_get(buffer->packet_list, i);
			fprintf(stderr, " | %s |", packet->name);
		}
		fprintf(stderr, "\n");
	}
*/

}


struct vi_node_widget_t *vi_node_widget_create(struct vi_net_node_t *node)
{
	struct vi_node_widget_t *node_widget;

	/* Initialize */
	node_widget = xcalloc(1, sizeof(struct vi_node_widget_t));
	node_widget->node = node;

	/* Content Layout */
	GtkWidget *content_layout;
	content_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(content_layout, VI_NODE_CONTENT_LAYOUT_WIDTH,
			VI_NODE_CONTENT_LAYOUT_HEIGHT);
	g_signal_connect(G_OBJECT(content_layout), "size_allocate",
			G_CALLBACK(vi_node_widget_size_allocate), node_widget);
	g_signal_connect(G_OBJECT(content_layout), "scroll-event",
			G_CALLBACK(vi_node_widget_scroll), node_widget);
	node_widget->content_layout = content_layout;

	/* Table Right - Vertical */
	GtkWidget *buffer_queue_layout;
	buffer_queue_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(buffer_queue_layout , VI_NODE_BUFFER_QUEUE_LAYOUT_WIDTH,
			VI_NODE_BUFFER_QUEUE_LAYOUT_HEIGHT);
	node_widget->buffer_queue_layout = buffer_queue_layout;

	/* Right vertical box */
	GtkWidget *right_vbox;
	right_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), buffer_queue_layout, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(right_vbox), content_layout, TRUE, TRUE, 0);

	/* Buffer Queue Layout */
	GtkWidget *buffer_table_layout;
	buffer_table_layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(buffer_table_layout, VI_NODE_BUFFER_TABLE_LAYOUT_WIDTH,
			VI_NODE_BUFFER_TABLE_LAYOUT_HEIGHT);
	g_signal_connect(G_OBJECT(buffer_table_layout), "size_allocate",
			G_CALLBACK(vi_node_widget_size_allocate), node_widget);
	g_signal_connect(G_OBJECT(buffer_table_layout), "scroll-event",
			G_CALLBACK(vi_node_widget_scroll), node_widget);
	node_widget->buffer_table_layout = buffer_table_layout;

	/* Buffer Table Title Layout */
	GtkWidget *buffer_table_title_layout;
	buffer_table_title_layout= gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(buffer_table_title_layout, VI_NODE_BUFFER_TABLE_LAYOUT_WIDTH,
			VI_NODE_CONTENT_CELL_HEIGHT);
	node_widget->buffer_table_title_layout = buffer_table_title_layout;

	/* Left vertical box */
	GtkWidget *left_vbox;
	left_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start(GTK_BOX(left_vbox), buffer_table_title_layout, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(left_vbox), buffer_table_layout, TRUE, TRUE, 0);

	/* Inner horizontal panel */
	GtkWidget *inner_hpaned;
	inner_hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_pack1(GTK_PANED(inner_hpaned), left_vbox, TRUE, FALSE);
	gtk_paned_pack2(GTK_PANED(inner_hpaned), right_vbox, TRUE, TRUE);

	/* Scroll bars */
	GtkWidget *hscrollbar = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, NULL);
	GtkWidget *vscrollbar = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, NULL);
	node_widget->hscrollbar = hscrollbar;
	node_widget->vscrollbar = vscrollbar;
	g_signal_connect(G_OBJECT(hscrollbar), "value-changed",
			G_CALLBACK(vi_node_widget_scroll_bar_value_changed), node_widget);
	g_signal_connect(G_OBJECT(vscrollbar), "value-changed",
			G_CALLBACK(vi_node_widget_scroll_bar_value_changed), node_widget);

	/* Table */
	GtkWidget *table;
	table = gtk_table_new(2, 2, FALSE);
	gtk_table_attach(GTK_TABLE(table), inner_hpaned, 0, 1, 0, 1,
			GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
	gtk_table_attach(GTK_TABLE(table), hscrollbar, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(table), vscrollbar, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);

	/* Widget */
	node_widget->widget = table;
	g_signal_connect(G_OBJECT(node_widget->widget), "destroy",
			G_CALLBACK(vi_node_widget_destroy), node_widget);
	g_signal_connect(G_OBJECT(node_widget->widget), "size_allocate",
			G_CALLBACK(vi_node_widget_size_allocate), node_widget);

	/* Return */
	return node_widget;
}


void vi_node_widget_refresh(struct vi_node_widget_t *node_widget)
{
	int content_layout_width;
	int content_layout_height;

	int buffer_table_layout_width;
	int buffer_table_layout_height;

	int left;
	int left_packet;
	int left_offset;

	int top;
	int top_buffer;
	int top_offset;

	int content_layout_width_changed;
	int content_layout_height_changed;

	int buffer_table_layout_width_changed;
	int buffer_table_layout_height_changed;
	int left_packet_changed;
	int left_offset_changed;
	int top_buffer_changed;
	int top_offset_changed;

	/* Get new state */
	content_layout_width = gtk_widget_get_allocated_width(node_widget->content_layout);
	content_layout_height = gtk_widget_get_allocated_height(node_widget->content_layout);

	buffer_table_layout_width = gtk_widget_get_allocated_width(node_widget->buffer_table_layout);
	buffer_table_layout_height = gtk_widget_get_allocated_height(node_widget->buffer_table_layout);
	left = gtk_range_get_value(GTK_RANGE(node_widget->hscrollbar));
	left_packet = left / VI_NODE_CONTENT_CELL_WIDTH;
	left_offset = -(left % VI_NODE_CONTENT_CELL_WIDTH);
	top = gtk_range_get_value(GTK_RANGE(node_widget->vscrollbar));
	top_buffer = top / VI_NODE_CONTENT_CELL_HEIGHT;
	top_offset = -(top % VI_NODE_CONTENT_CELL_HEIGHT);

	/* Record changes */
	content_layout_width_changed = content_layout_width != node_widget->content_layout_width;
	content_layout_height_changed = content_layout_height != node_widget->content_layout_height;

	buffer_table_layout_width_changed = buffer_table_layout_width != node_widget->buffer_table_layout_width;
	buffer_table_layout_height_changed = buffer_table_layout_height != node_widget->buffer_table_layout_height;
	left_packet_changed = left_packet != node_widget->left_packet;
	left_offset_changed = left_offset != node_widget->left_offset;
	top_buffer_changed = top_buffer != node_widget->top_buffer;
	top_offset_changed = top_offset != node_widget->top_offset;

	/* Save new state */
	node_widget->content_layout_width = content_layout_width;
	node_widget->content_layout_height = content_layout_height;
	node_widget->buffer_table_layout_width = buffer_table_layout_width;
	node_widget->buffer_table_layout_height = buffer_table_layout_height;
	node_widget->left_packet = left_packet;
	node_widget->left_offset = left_offset;
	node_widget->top_buffer = top_buffer;
	node_widget->top_offset = top_offset;

	/* Refresh scroll bars */
	if (content_layout_width_changed || content_layout_height_changed)
		vi_node_widget_refresh_scrollbars(node_widget);

	/* Refresh content layout */
	if (content_layout_width_changed || content_layout_height_changed)
		vi_node_widget_refresh_content_layout(node_widget);

	/* Refresh content layout position */
	if (left_offset_changed || top_offset_changed)
		gtk_layout_move(GTK_LAYOUT(node_widget->content_layout), node_widget->content_table,
				node_widget->left_offset, node_widget->top_offset);


	/* Refresh buffer table layout */
	if (buffer_table_layout_width_changed || buffer_table_layout_height_changed)
		vi_node_widget_refresh_buffer_table_layout(node_widget);

	/* Refresh buffer table layout position */
	if (top_offset_changed)
		gtk_layout_move(GTK_LAYOUT(node_widget->buffer_table_layout), node_widget->buffer_table,
				0, node_widget->top_offset);

	if (buffer_table_layout_width_changed)
		vi_node_widget_refresh_buffer_title_layout(node_widget);

	/* Refresh cycle layout */
	if (content_layout_width_changed)
		vi_node_widget_refresh_buffer_queue_layout(node_widget);

	/* Refresh cycle layout position */
	if (left_offset_changed)
		gtk_layout_move(GTK_LAYOUT(node_widget->buffer_queue_layout), node_widget->buffer_queue,
				node_widget->left_offset, 0);

	/* Refresh content */
	if (content_layout_width_changed || content_layout_height_changed
			|| buffer_table_layout_width_changed || buffer_table_layout_height_changed
			|| left_packet_changed || left_offset_changed || top_buffer_changed
			|| top_offset_changed)
		vi_node_widget_refresh_content(node_widget);
}



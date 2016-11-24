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


#ifndef VISUAL_NETWORK_NODE_WIDGET_H
#define VISUAL_NETWORK_NODE_WIDGET_H

#include <gtk/gtk.h>

struct vi_node_widget_t
{
	struct vi_net_node_t *node;

	struct vi_list_t *message_list;
	GtkWidget *widget;

        GtkWidget *buffer_table_title_layout;
        GtkWidget *buffer_title;
        struct list_t *buffer_title_list;

	GtkWidget *hscrollbar;
	GtkWidget *vscrollbar;

        GtkWidget *content_layout;
        GtkWidget *content_table;
        struct matrix_t *content_matrix;

        GtkWidget *buffer_queue_layout;
        GtkWidget *buffer_queue;
        struct list_t *buffer_queue_list;

        GtkWidget *buffer_table_layout;
        GtkWidget *buffer_table;
        struct list_t *buffer_list;

        GtkWidget *msg_bar_layout;

        int content_layout_width;
        int content_layout_height;

        int buffer_table_layout_width;
        int buffer_table_layout_height;

        int left_packet;
        int left_offset;

        int top_buffer;
        int top_offset;
};


struct vi_node_widget_t *vi_node_widget_create     (struct vi_net_node_t *node);
void                     vi_node_widget_free       (struct vi_node_widget_t *node_widget);
void 			 vi_node_widget_refresh    (struct vi_node_widget_t *node_widget);
GtkWidget               *vi_node_widget_get_widget (struct vi_node_widget_t *widget);
void vi_node_widget_refresh_content(struct vi_node_widget_t *node_widget);

#endif

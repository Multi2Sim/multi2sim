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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "node-widget.h"

#define VI_NODE_CONTENT_LAYOUT_WIDTH    300
#define VI_NODE_CONTENT_LAYOUT_HEIGHT   200

#define VI_NODE_MSG_BAR_LAYOUT_WIDTH    300
#define VI_NODE_MSG_BAR_LAYOUT_HEIGHT    50

#define VI_NODE_FIRST_ROW_HEIGHT         20
#define VI_NODE_FIRST_COL_WIDTH          100


static void vi_node_widget_destroy(GtkWidget *widget, struct vi_node_widget_t *node_widget);

GtkWidget *vi_node_widget_get_widget(struct vi_node_widget_t *node_widget)
{
	return node_widget->widget;
}

static void vi_node_widget_size_allocate(GtkWidget *widget, GdkRectangle *allocation,
        struct vi_node_widget_t *node_widget)
{
        /* NOT_IMPLEMENTED : place holder
         * vi_node_widget_refresh(node_widget);
          */
}

static void vi_node_widget_scroll_bar_value_changed(GtkRange *range,
        struct vi_node_widget_t *mod_widget)
{
        /* NOT_IMPLEMENTED : place holder
         * vi_node_widget_refresh(node_widget);
          */
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

struct vi_node_widget_t *vi_node_widget_create(char *node_name)
{
	struct vi_node_widget_t *node_widget;

	/* Initialize */
	node_widget = xcalloc(1, sizeof(struct vi_node_widget_t));
	node_widget->name = xstrdup(node_name);

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

	/* Horizontal Message list layout */
        GtkWidget *msg_bar_layout;
        msg_bar_layout = gtk_layout_new(NULL, NULL);
        gtk_widget_set_size_request(msg_bar_layout, VI_NODE_MSG_BAR_LAYOUT_WIDTH,
                        VI_NODE_MSG_BAR_LAYOUT_HEIGHT);
        g_signal_connect(G_OBJECT(content_layout), "size_allocate",
                G_CALLBACK(vi_node_widget_size_allocate), node_widget);
        g_signal_connect(G_OBJECT(content_layout), "scroll-event",
                G_CALLBACK(vi_node_widget_scroll), node_widget);
        node_widget->msg_bar_layout = msg_bar_layout;
        /* Connection Name Table
         * -------------------------------------------------------
         * Direction - Connection Name - Buffer Name - Buffer Size
         * -------------------------------------------------------
         *
         * 1. Set Colors.
         * 2. Set First Row.
         * 3. Set Table
         */
        /* Colors */
        GdkColor color_gray;
        gdk_color_parse("#aaaaaa", &color_gray);

        /* First Row Layout */
        GtkWidget *first_row_layout = gtk_layout_new(NULL, NULL);
        gtk_widget_set_size_request(first_row_layout, -1, VI_NODE_FIRST_ROW_HEIGHT);
        gtk_widget_modify_bg(first_row_layout, GTK_STATE_NORMAL, &color_gray);
        node_widget->first_row_layout = first_row_layout;

        /* First column layout */
        GtkWidget *first_col_layout = gtk_layout_new(NULL, NULL);
        gtk_widget_set_size_request(first_col_layout, VI_NODE_FIRST_COL_WIDTH, -1);
        gtk_widget_modify_bg(first_col_layout, GTK_STATE_NORMAL, &color_gray);
        node_widget->first_col_layout = first_col_layout;

        /* Top-left label */
        GtkWidget *top_left_label = gtk_label_new("Direction");
        gtk_widget_set_size_request(top_left_label, VI_NODE_FIRST_COL_WIDTH - 1,
                VI_NODE_FIRST_ROW_HEIGHT - 1);
        gtk_widget_show(top_left_label);

        /* Middle label */
        GtkWidget *mid_label = gtk_label_new("Connection-Name");
        gtk_widget_set_size_request(mid_label, VI_NODE_FIRST_COL_WIDTH - 1,
                VI_NODE_FIRST_ROW_HEIGHT - 1);
        gtk_widget_show(mid_label);

        /* Top-left label */
        GtkWidget *top_right_label = gtk_label_new("BufferName[Size]");
        gtk_widget_set_size_request(top_right_label, VI_NODE_FIRST_COL_WIDTH - 1,
                VI_NODE_FIRST_ROW_HEIGHT - 1);
        gtk_widget_show(top_right_label);

        /* Top-left event box */
        GtkWidget *top_left_event_box = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(top_left_event_box), top_left_label);
        gtk_widget_show(top_left_event_box);

        /* Middle event box */
        GtkWidget *middle_event_box = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(middle_event_box), mid_label);
        gtk_widget_show(middle_event_box);

        /* Top-Right event box */
        GtkWidget *top_right_event_box = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(top_right_event_box), top_right_label);
        gtk_widget_show(top_right_event_box);

        /* Scroll bars */
        GtkWidget *hscrollbar = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, NULL);
        GtkWidget *vscrollbar = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, NULL);
        node_widget->hscrollbar = hscrollbar;
        node_widget->vscrollbar = vscrollbar;
        g_signal_connect(G_OBJECT(hscrollbar), "value-changed",
                G_CALLBACK(vi_node_widget_scroll_bar_value_changed), node_widget);
        g_signal_connect(G_OBJECT(vscrollbar), "value-changed",
                G_CALLBACK(vi_node_widget_scroll_bar_value_changed), node_widget);

        /* event box background color */
        GdkColor color;
        gdk_color_parse("white", &color);
        gtk_widget_modify_bg(top_left_event_box, GTK_STATE_NORMAL, &color);
        gtk_widget_modify_bg(middle_event_box, GTK_STATE_NORMAL, &color);
        gtk_widget_modify_bg(top_right_event_box, GTK_STATE_NORMAL, &color);

        /* Vertical box */
        GtkWidget *vbox;
        vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

        /* Table_left */
        GtkWidget *table_left;
        table_left = gtk_table_new(3, 3, FALSE);
        gtk_table_attach(GTK_TABLE(table_left), content_layout, 1, 2, 1, 2,
                GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
        gtk_table_attach(GTK_TABLE(table_left), top_left_event_box, 0, 1, 0, 1, 0, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(table_left), middle_event_box, 0, 1, 0, 1, 0, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(table_left), top_right_event_box, 0, 1, 0, 1, 0, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(table_left), first_row_layout, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(table_left), first_col_layout, 0, 1, 1, 2, 0, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
        gtk_table_attach(GTK_TABLE(table_left), hscrollbar, 1, 2, 2, 3, GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_FILL, 0, 0);
        gtk_table_attach(GTK_TABLE(table_left), vscrollbar, 2, 3, 1, 2, GTK_FILL, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);

        /* Frame */
        GtkWidget *frame;
        frame = gtk_frame_new(NULL);
        gtk_container_add(GTK_CONTAINER(frame), table_left);
        gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

        /* Outer horizontal panel */
        GtkWidget *outer_hpaned;
        outer_hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
        gtk_paned_pack1(GTK_PANED(outer_hpaned), vbox, TRUE, FALSE);


        /* Table */
        GtkWidget *table;
        table = gtk_table_new(2, 2, FALSE);
        gtk_table_attach(GTK_TABLE(table), outer_hpaned, 0, 1, 0, 1,
                GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);
        gtk_table_attach(GTK_TABLE(table), hscrollbar, 0, 1, 1, 2, GTK_EXPAND | GTK_FILL | GTK_SHRINK, GTK_FILL, 0, 0);
        gtk_table_attach(GTK_TABLE(table), vscrollbar, 1, 2, 0, 1, GTK_FILL, GTK_EXPAND | GTK_FILL | GTK_SHRINK, 0, 0);

	/* Assign main widget */
	node_widget->widget = table;
	g_signal_connect(G_OBJECT(node_widget->widget), "destroy",
		G_CALLBACK(vi_node_widget_destroy), node_widget);

	/* Return */
	return node_widget;
}

static void vi_node_widget_destroy(GtkWidget *widget, struct vi_node_widget_t *node_widget)
{
	vi_node_widget_free(node_widget);
}


void vi_node_widget_free(struct vi_node_widget_t *node_widget)
{

	/* Free widget */
	free(node_widget->name);
	free(node_widget);
}

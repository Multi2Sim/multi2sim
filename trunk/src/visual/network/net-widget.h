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


#ifndef VISUAL_NETWORK_NET_WIDGET_H
#define VISUAL_NETWORK_NET_WIDGET_H

#include <gtk/gtk.h>

struct vi_net_widget_t
{
	GtkWidget *widget;

	/* List of Node Boards */
	struct list_t *node_board_list;

	/* List of all subLinks */
	struct list_t *link_board_list;

};

/* Node Detailed View */
struct vi_node_window_t
{
	/* Main detail Window */
	GtkWidget *widget;

	/* Network that window show */
	struct vi_net_node_t *node;

	/* Toggle button that causes activation of window */
	GtkWidget *parent_toggle_button;

	/* Node widget */
	struct vi_node_widget_t *node_widget;

};

struct vi_node_board_t
{
	/* Main Widget */
	GtkWidget *widget;

	/* Pop-up window for node detail */
	struct vi_node_window_t *node_window;

	/* Toggle button to activate pop-up window */
	GtkWidget *toggle_button;

	/* Associated Node */
	struct vi_net_node_t *node;
};

struct vi_link_board_t
{
        GtkWidget *widget;

        float src_x;
        float src_y;
        float dst_x;
        float dst_y;

        struct vi_net_link_t *link;
};



struct vi_net_widget_t *vi_net_widget_create     (struct vi_net_t *net);
void                    vi_net_widget_free       (struct vi_net_widget_t *net_widget);
GtkWidget              *vi_net_widget_get_widget (struct vi_net_widget_t *widget);
void 			vi_net_widget_refresh    (struct vi_net_widget_t *net_widget);


#endif

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

#ifndef VISUAL_NETWORK_PANEL_H
#define VISUAL_NETWORK_PANEL_H

#include <gtk/gtk.h>

/* Network Detailed View */
struct vi_net_window_t
{
	/* Main detail Window */
	GtkWidget *widget;

	/* Network that window show */
	struct vi_net_t *net;

	/* Toggle button that causes activation of window */
	GtkWidget *parent_toggle_button;

	/* Network widget */
	struct vi_net_widget_t *net_widget;

};

struct vi_net_board_t
{
	/* Main widget */
	GtkWidget *widget;

	/* Pop-up window showing detail */
	struct vi_net_window_t *net_window;

	/* Toggle button to activate pop-up window */
	GtkWidget *toggle_button;

	/* Associated Network */
	struct vi_net_t *net;

	/* Message list */
	struct vi_list_t *message_list;
};


struct vi_net_board_t *vi_net_board_create(struct vi_net_t *net);
void 		       vi_net_board_refresh(struct vi_net_board_t *board);

#endif

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
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "net.h"
#include "panel.h"

/*
 * Network Module Board
 */

#define VI_MOD_BOARD_PADDING		10
#define VI_MOD_BOARD_WIDTH		100
#define VI_MOD_BOARD_HEIGHT		100


static gboolean vi_net_board_toggle_button_toggled(GtkWidget *widget, struct vi_net_board_t *board);
static void vi_net_board_destroy(GtkWidget *widget, struct vi_net_board_t *board);


struct vi_net_board_t *vi_net_board_create(struct vi_net_t *net)
{
	struct vi_net_board_t *board;

	char str[MAX_STRING_SIZE];

	/* Initialize */
	board = xcalloc(1, sizeof(struct vi_net_board_t));
	board->net = net;

	/* Color */

	GdkRGBA frame_color;
	frame_color.red = .682;
	frame_color.green = .917;
	frame_color.blue = 1;
	frame_color.alpha = .8;

	/* Frame */
	GtkWidget *frame = gtk_frame_new(NULL);
	GtkWidget *event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(event_box), frame);
	gtk_widget_override_background_color(event_box, GTK_STATE_FLAG_NORMAL, &frame_color);
	gtk_widget_set_size_request(frame, VI_MOD_BOARD_WIDTH, VI_MOD_BOARD_HEIGHT);

	/* Vertical box */
	GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	gtk_container_add(GTK_CONTAINER(frame), vbox);

	/* Name */
	snprintf(str, sizeof str, "<b>%s</b>", net->name);

	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label), str);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);

	/* Horizontal box */
	GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

	/* Toggle button */
	GtkWidget *toggle_button = gtk_toggle_button_new_with_label("Detail");
	gtk_box_pack_start(GTK_BOX(hbox), toggle_button, TRUE, TRUE, 0);
	g_signal_connect(G_OBJECT(toggle_button), "toggled",
			G_CALLBACK(vi_net_board_toggle_button_toggled), board);
	board->toggle_button = toggle_button;

	/* Main widget */
	board->widget = event_box;
	g_signal_connect(G_OBJECT(board->widget), "destroy", G_CALLBACK(vi_net_board_destroy), board);

	/* Return */
	return board;
}

static gboolean vi_net_board_toggle_button_toggled(GtkWidget *widget, struct vi_net_board_t *board)
{

	int active;

	/* Get button state */
	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(board->toggle_button));

	fprintf(stderr, "active = %d Window will be created \n", active);

	/* Show
	if (active && !board->net_window)
	{

		net_window = vi_net_window_create(board->net, board->toggle_button);
		board->net_window = net_window;
	}

	 Hide
	if (!active && board->net_window)
	{
		gtk_widget_destroy(vi_net_window_get_widget(board->net_window));
		board->net_window = NULL;
	}
	 */
	return FALSE;
}

static void vi_net_board_free(struct vi_net_board_t *board)
{
	/* Destroy pop-up window */
	/*	if (board->net_window)
		gtk_widget_destroy(vi_net_window_get_widget(board->net_window));
	 */

	/* Free */
	free(board);
}


static void vi_net_board_destroy(GtkWidget *widget, struct vi_net_board_t *board)
{
	vi_net_board_free(board);
}

/*
static GtkWidget *vi_net_window_get_widget(struct vi_net_window_t *net_window)
{
	return net_window->widget;
}

 */

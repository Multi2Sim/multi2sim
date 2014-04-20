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
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <visual/common/list.h>

#include "net.h"
#include "panel.h"
#include "net-message.h"
#include "net-widget.h"

#define VI_NET_BOARD_PADDING		10
#define VI_NET_BOARD_WIDTH		100
#define VI_NET_BOARD_HEIGHT		100

#define VI_NETWORK_LAYOUT_WIDTH         800
#define VI_NETWORK_LAYOUT_HEIGHT        600


static gboolean 		 vi_net_board_toggle_button_toggled (GtkWidget *widget, struct vi_net_board_t *board);
static void 			 vi_net_board_destroy               (GtkWidget *widget, struct vi_net_board_t *board);
static struct vi_net_window_t   *vi_net_window_create              (struct vi_net_t * net, GtkWidget *parent_toggle_button);
static void                      vi_net_window_free                 (struct vi_net_window_t *net_window);
static void                      vi_net_window_destroy              (GtkWidget *widget, struct vi_net_window_t *net_window);
static gboolean                  vi_net_window_delete               (GtkWidget *widget, GdkEvent *event, struct vi_net_window_t *net_window);
static GtkWidget                *vi_net_window_get_widget           (struct vi_net_window_t *net_window);


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
	gtk_widget_set_size_request(frame, VI_NET_BOARD_WIDTH, VI_NET_BOARD_HEIGHT);

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

	/* Access list */
	struct vi_list_t *message_list = vi_list_create("Message list", 10, 10, (vi_list_get_elem_name_func_t)
			vi_net_message_get_name_short, (vi_list_get_elem_desc_func_t) vi_net_message_get_desc);
	gtk_box_pack_start(GTK_BOX(vbox), vi_list_get_widget(message_list), TRUE, TRUE, 0);
	gtk_widget_override_background_color(vi_list_get_widget(message_list), GTK_STATE_FLAG_NORMAL, &frame_color);
	board->message_list = message_list;

	/* Main widget */
	board->widget = event_box;
	g_signal_connect(G_OBJECT(board->widget), "destroy", G_CALLBACK(vi_net_board_destroy), board);

	/* Return */
	return board;
}

static gboolean vi_net_board_toggle_button_toggled(GtkWidget *widget, struct vi_net_board_t *board)
{
	struct vi_net_window_t *net_window;

	int active;

	/* Get button state */
	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(board->toggle_button));

	/* Show */
	if (active && !board->net_window)
	{

		net_window = vi_net_window_create(board->net, board->toggle_button);
		board->net_window = net_window;
	}

	/* Hide */
	if (!active && board->net_window)
	{
		gtk_widget_destroy(vi_net_window_get_widget(board->net_window));
		board->net_window = NULL;
	}

	return FALSE;
}

static void vi_net_board_free(struct vi_net_board_t *board)
{
	/* Destroy pop-up window */
	if (board->net_window)
		gtk_widget_destroy(vi_net_window_get_widget(board->net_window));

	/* Free message list */
	while (vi_list_count(board->message_list))
		vi_list_remove_at(board->message_list, 0);
	vi_list_free(board->message_list);

	/* Free */
	free(board);
}


static void vi_net_board_destroy(GtkWidget *widget, struct vi_net_board_t *board)
{
	vi_net_board_free(board);
}


static struct vi_net_window_t * vi_net_window_create(struct vi_net_t * net, GtkWidget *parent_toggle_button)
{
	struct vi_net_window_t *net_window;

	char str[MAX_STRING_SIZE];

	/* Initialize */
	net_window = xcalloc(1, sizeof(struct vi_net_window_t));
	net_window->net = net;
	net_window->parent_toggle_button = parent_toggle_button;

	/* Main window */
	snprintf(str, sizeof str, "Network %s", net->name);
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), str);
        gtk_window_set_default_size(GTK_WINDOW(window), VI_NETWORK_LAYOUT_WIDTH,
                        VI_NETWORK_LAYOUT_HEIGHT);
	/* Module widget */
	struct vi_net_widget_t *net_widget;
	net_widget = vi_net_widget_create(net);
	gtk_container_add(GTK_CONTAINER(window), vi_net_widget_get_widget(net_widget));
	net_window->net_widget = net_widget;

	/* Associated widget */
	net_window->widget = window;
	gtk_widget_show_all(net_window->widget);
	g_signal_connect(G_OBJECT(net_window->widget), "destroy", G_CALLBACK(vi_net_window_destroy), net_window);
	g_signal_connect(G_OBJECT(net_window->widget), "delete_event", G_CALLBACK(vi_net_window_delete), net_window);

	/* Return */
	return net_window;
}

static void vi_net_window_free(struct vi_net_window_t *net_window)
{
	free(net_window);
}

static void vi_net_window_destroy(GtkWidget *widget, struct vi_net_window_t *net_window)
{
	vi_net_window_free(net_window);
}

static gboolean vi_net_window_delete(GtkWidget *widget, GdkEvent *event, struct vi_net_window_t *net_window)
{
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(net_window->parent_toggle_button), FALSE);
	return TRUE;
}

static GtkWidget *vi_net_window_get_widget(struct vi_net_window_t *net_window)
{
	return net_window->widget;
}

static void vi_net_window_refresh(struct vi_net_window_t *net_window)
{
	vi_net_widget_refresh(net_window->net_widget);
}

void vi_net_board_refresh(struct vi_net_board_t *board)
{
	struct vi_net_message_t *message;

	char *message_name;

	/* Empty access list */
	while (vi_list_count(board->message_list))
		vi_list_remove_at(board->message_list, 0);

	/* Refresh access list */
	HASH_TABLE_FOR_EACH(board->net->message_table, message_name, message)
	{
		/* Add to list */
		vi_list_add(board->message_list, message);
	}
	vi_list_refresh(board->message_list);

	/* Refresh pop-up window */
	if (board->net_window)
		vi_net_window_refresh(board->net_window);
}

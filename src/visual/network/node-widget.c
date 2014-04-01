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

static void vi_node_widget_destroy(GtkWidget *widget, struct vi_node_widget_t *node_widget);

GtkWidget *vi_node_widget_get_widget(struct vi_node_widget_t *node_widget)
{
	return node_widget->widget;
}

struct vi_node_widget_t *vi_node_widget_create(char *node_name)
{
	struct vi_node_widget_t *node_widget;

	/* Initialize */
	node_widget = xcalloc(1, sizeof(struct vi_node_widget_t));
	node_widget->name = xstrdup(node_name);

	/* You should create different Widgets based on the type. Return vi_node_widget_t */
	/* Vertical box */
	GtkWidget *vbox;
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	/* Assign main widget */
	node_widget->widget = vbox;
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

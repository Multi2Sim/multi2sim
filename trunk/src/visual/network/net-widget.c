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

#include "net-widget.h"

static void vi_net_widget_destroy(GtkWidget *widget, struct vi_net_widget_t *net_widget);

GtkWidget *vi_net_widget_get_widget(struct vi_net_widget_t *net_widget)
{
	return net_widget->widget;
}

struct vi_net_widget_t *vi_net_widget_create(char *net_name)
{
	struct vi_net_widget_t *net_widget;

	/* Initialize */
	net_widget = xcalloc(1, sizeof(struct vi_net_widget_t));
	net_widget->name = xstrdup(net_name);

	/* Vertical box */
	GtkWidget *vbox;
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	/* Assign main widget */
	net_widget->widget = vbox;
	g_signal_connect(G_OBJECT(net_widget->widget), "destroy",
		G_CALLBACK(vi_net_widget_destroy), net_widget);

	/* Return */
	return net_widget;
}

static void vi_net_widget_destroy(GtkWidget *widget, struct vi_net_widget_t *net_widget)
{
	vi_net_widget_free(net_widget);
}


void vi_net_widget_free(struct vi_net_widget_t *net_widget)
{

	/* Free widget */
	free(net_widget->name);
	free(net_widget);
}

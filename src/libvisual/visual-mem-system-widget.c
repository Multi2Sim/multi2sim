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

#include <visual-private.h>


struct visual_mem_system_widget_t
{
	GtkWidget *widget;
};


static void visual_mem_system_widget_destroy_event(GtkWidget *widget,
	struct visual_mem_system_widget_t *visual_mem_system_widget)
{
	visual_mem_system_widget_free(visual_mem_system_widget);
}


struct visual_mem_system_widget_t *visual_mem_system_widget_create(void)
{
	struct visual_mem_system_widget_t *widget;

	/* Allocate */
	widget = calloc(1, sizeof(struct visual_mem_system_widget_t));
	if (!widget)
		fatal("%s: out of memory", __FUNCTION__);

	/* Layout */
	GtkWidget *layout;
	layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(layout, 100, 100);
	g_signal_connect(G_OBJECT(layout), "destroy",
		G_CALLBACK(visual_mem_system_widget_destroy_event), widget);
	widget->widget = layout;

	/* Return */
	return widget;
}


void visual_mem_system_widget_free(struct visual_mem_system_widget_t *widget)
{
	free(widget);
}


GtkWidget *visual_mem_system_widget_get_widget(struct visual_mem_system_widget_t *widget)
{
	return widget->widget;
}

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

#include <memvisual-private.h>



struct vmod_t *vmod_create(char *name, int level)
{
	struct vmod_t *vmod;

	/* Allocate */
	vmod = calloc(1, sizeof(struct vmod_t));
	if (!vmod)
		fatal("%s: out of memory", __FUNCTION__);

	/* Name */
	vmod->name = strdup(name);
	if (!name)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	vmod->low_vmod_list = list_create();
	vmod->high_vmod_list = list_create();
	vmod->low_vmod_conn_list = list_create();
	vmod->high_vmod_conn_list = list_create();
	vmod->level = level;

	/* Create layout */
	vmod->layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(vmod->layout, VMOD_DEFAULT_WIDTH, VMOD_DEFAULT_HEIGHT);
	g_signal_connect(G_OBJECT(vmod->layout), "draw", G_CALLBACK(vmod_draw_event), vmod);

	/* Return */
	return vmod;
}


void vmod_free(struct vmod_t *vmod)
{
	list_free(vmod->low_vmod_list);
	list_free(vmod->high_vmod_list);
	list_free(vmod->low_vmod_conn_list);
	list_free(vmod->high_vmod_conn_list);
	free(vmod->name);
	free(vmod);
}


gboolean vmod_draw_event(GtkWidget *widget, GdkEventConfigure *event, struct vmod_t *vmod)
{
	GdkWindow *window;
	cairo_t *cr;

	/* Create cairo */
	window = gtk_layout_get_bin_window(GTK_LAYOUT(widget));
	cr = gdk_cairo_create(window);

	/* Draw connections */
	cairo_rectangle(cr, 0, 0, VMOD_DEFAULT_WIDTH, VMOD_DEFAULT_HEIGHT);
	cairo_stroke(cr);

	/* Destroy cairo */
	cairo_destroy(cr);
	return FALSE;
}

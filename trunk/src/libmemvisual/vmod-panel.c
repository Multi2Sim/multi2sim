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


/*
 * Private Functions
 */

static struct vmod_conn_t *vmod_conn_create(int x, int y, int m1, int m2, int m3,
	struct vmod_t *source, struct vmod_t *dest)
{
	struct vmod_conn_t *conn;

	/* Create */
	conn = calloc(1, sizeof(struct vmod_conn_t));
	if (!conn)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	conn->x = x;
	conn->y = y;
	conn->m1 = m1;
	conn->m2 = m2;
	conn->m3 = m3;
	conn->source = source;
	conn->dest = dest;

	/* Return */
	return conn;
}


static void vmod_conn_free(struct vmod_conn_t *conn)
{
	free(conn);
}


#define VMOD_CONN_RADIUS  8
#define VMOD_CONN_LINE_WIDTH  4


static void vmod_conn_draw(struct vmod_conn_t *conn, cairo_t *cr)
{
	const int r = VMOD_CONN_RADIUS;

	int x[4];
	int y[4];

	/* Calculate points */
	x[0] = conn->x;
	y[0] = conn->y;
	x[1] = conn->x;
	y[1] = conn->y + conn->m1;
	x[2] = conn->x + conn->m2;
	y[2] = conn->y + conn->m1;
	x[3] = conn->x + conn->m2;
	y[3] = conn->y + conn->m1 + conn->m3;

	cairo_set_line_width(cr, VMOD_CONN_LINE_WIDTH);
	cairo_set_source_rgb(cr, 0, 0, 0);

	cairo_move_to(cr, x[0], y[0]);
	cairo_line_to(cr, x[1], y[1] - r);
	cairo_stroke(cr);

	cairo_arc(cr, x[1] + r, y[1] - r, r, M_PI / 2, M_PI);
	cairo_stroke(cr);

	cairo_move_to(cr, x[1] + r, y[1]);
	cairo_line_to(cr, x[2] - r, y[2]);
	cairo_stroke(cr);

	cairo_arc(cr, x[2] - r, y[2] + r, r, M_PI * 1.5, M_PI * 2);
	cairo_stroke(cr);

	cairo_move_to(cr, x[2], y[2] + r);
	cairo_line_to(cr, x[3], y[3]);
	cairo_stroke(cr);

	/*
	cairo_line_to(cr, x[2], y[2]);
	cairo_line_to(cr, x[3], y[3]);
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_stroke(cr);*/
}


static gboolean vmod_panel_draw_event(GtkWidget *widget, GdkEventConfigure *event,
	struct vmod_panel_t *panel)
{
	GdkWindow *window;
	cairo_t *cr;
	int i;

	/* Create cairo */
	window = gtk_layout_get_bin_window(GTK_LAYOUT(widget));
	cr = gdk_cairo_create(window);

	/* Draw connections */
	for (i = 0; i < list_count(panel->vmod_conn_list); i++)
		vmod_conn_draw(list_get(panel->vmod_conn_list, i), cr);

	/* Destroy cairo */
	cairo_destroy(cr);
	return FALSE;
}



/*
 * Public Functions
 */


struct vmod_panel_t *vmod_panel_create(void)
{
	struct vmod_panel_t *panel;

	/* Allocate */
	panel = calloc(1, sizeof(struct vmod_panel_t));
	if (!panel)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	panel->vmod_list = list_create();
	panel->vmod_conn_list = list_create();

	/* Create layout */
	panel->layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(panel->layout, 500, 300);
	g_signal_connect(G_OBJECT(panel->layout), "draw",
		G_CALLBACK(vmod_panel_draw_event), panel);

	/* Draw */
	vmod_panel_draw(panel);

	/* Return */
	return panel;
}


void vmod_panel_free(struct vmod_panel_t *panel)
{
	int i;

	/* Free modules */
	for (i = 0; i < list_count(panel->vmod_list); i++)
		vmod_free(list_get(panel->vmod_list, i));
	list_free(panel->vmod_list);

	/* Free connections */
	for (i = 0; i < list_count(panel->vmod_conn_list); i++)
		vmod_conn_free(list_get(panel->vmod_conn_list, i));
	list_free(panel->vmod_conn_list);

	/* Free panel */
	free(panel);
}


void vmod_panel_draw(struct vmod_panel_t *panel)
{
	struct vmod_t *vmod1, *vmod2, *vmod3;
	struct vmod_conn_t *conn;

	vmod1 = vmod_create("l1-0", 1);
	vmod2 = vmod_create("l1-1", 1);
	vmod3 = vmod_create("l2", 2);

	list_add(vmod1->low_vmod_list, vmod3);
	list_add(vmod2->low_vmod_list, vmod3);
	list_add(vmod3->high_vmod_list, vmod1);
	list_add(vmod3->high_vmod_list, vmod2);

	list_add(panel->vmod_list, vmod1);
	list_add(panel->vmod_list, vmod2);
	list_add(panel->vmod_list, vmod3);

	gtk_layout_put(GTK_LAYOUT(panel->layout), vmod1->layout, 10, 10);

	conn = vmod_conn_create(50, 50, 100, 100, 100, vmod1, vmod2);
	list_add(panel->vmod_conn_list, conn);
}

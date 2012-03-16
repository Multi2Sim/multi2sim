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
 * Module Level
 */

static struct vmod_level_t *vmod_level_create(void)
{
	struct vmod_level_t *level;

	/* Create */
	level = calloc(1, sizeof(struct vmod_level_t));
	if (!level)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	level->vmod_list = list_create();

	/* Return */
	return level;
}


static void vmod_level_free(struct vmod_level_t *level)
{
	list_free(level->vmod_list);
	free(level);
}




/*
 * Module Connection
 */

struct vmod_conn_t *vmod_conn_create(int x0, int y0, int y1, int x2, int y3,
	struct vmod_t *source, struct vmod_t *dest)
{
	struct vmod_conn_t *conn;

	/* Create */
	conn = calloc(1, sizeof(struct vmod_conn_t));
	if (!conn)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	conn->x0 = x0;
	conn->y0 = y0;
	conn->y1 = y1;
	conn->x2 = x2;
	conn->y3 = y3;
	conn->source = source;
	conn->dest = dest;

	/* Return */
	return conn;
}


void vmod_conn_free(struct vmod_conn_t *conn)
{
	free(conn);
}


void vmod_conn_draw(struct vmod_conn_t *conn, cairo_t *cr)
{
	const int r = VMOD_CONN_RADIUS;

	int x[4];
	int y[4];

	/* Calculate points */
	x[0] = conn->x0;
	y[0] = conn->y0;
	x[1] = conn->x0;
	y[1] = conn->y1;
	x[2] = conn->x2;
	y[2] = conn->y1;
	x[3] = conn->x2;
	y[3] = conn->y3;

	cairo_set_line_width(cr, VMOD_CONN_LINE_WIDTH);
	cairo_set_source_rgb(cr, 0, 0, 0);

	if (x[1] < x[2])
	{
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
	}
	else
	{
		cairo_move_to(cr, x[0], y[0]);
		cairo_line_to(cr, x[1], y[1] - r);
		cairo_stroke(cr);

		cairo_arc(cr, x[1] - r, y[1] - r, r, 0, M_PI * 0.5);
		cairo_stroke(cr);

		cairo_move_to(cr, x[1] - r, y[1]);
		cairo_line_to(cr, x[2] + r, y[2]);
		cairo_stroke(cr);

		cairo_arc(cr, x[2] + r, y[2] + r, r, M_PI, M_PI * 1.5);
		cairo_stroke(cr);

		cairo_move_to(cr, x[2], y[2] + r);
		cairo_line_to(cr, x[3], y[3]);
		cairo_stroke(cr);
	}
}




/*
 * Visual Module Panel
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
	panel->vmod_level_list = list_create();

	/* Create layout */
	panel->layout = gtk_layout_new(NULL, NULL);
	g_signal_connect(G_OBJECT(panel->layout), "draw",
		G_CALLBACK(vmod_panel_draw_event), panel);

	/* Draw */
	vmod_panel_refresh(panel);

	/* Return */
	return panel;
}


void vmod_panel_free(struct vmod_panel_t *panel)
{
	int i;

	/* Free modules */
	LIST_FOR_EACH(panel->vmod_list, i)
		vmod_free(list_get(panel->vmod_list, i));
	list_free(panel->vmod_list);

	/* Free connections */
	LIST_FOR_EACH(panel->vmod_conn_list, i)
		vmod_conn_free(list_get(panel->vmod_conn_list, i));
	list_free(panel->vmod_conn_list);

	/* Free levels */
	LIST_FOR_EACH(panel->vmod_level_list, i)
		vmod_level_free(list_get(panel->vmod_level_list, i));
	list_free(panel->vmod_level_list);

	/* Free panel */
	free(panel);
}


void vmod_panel_populate_vmod_levels(struct vmod_panel_t *panel)
{
	struct vmod_t *vmod;
	struct vmod_level_t *level;

	int num_levels = 0;
	int i;

	/* Get number of levels */
	for (i = 0; i < list_count(panel->vmod_list); i++)
	{
		vmod = list_get(panel->vmod_list, i);
		if (vmod->level < 0)
			fatal("%s: invalid level", __FUNCTION__);
		num_levels = MAX(num_levels, vmod->level + 1);
	}

	/* Create levels */
	for (i = 0; i < num_levels; i++)
	{
		level = vmod_level_create();
		list_add(panel->vmod_level_list, level);
	}

	/* Add modules to levels */
	for (i = 0; i < list_count(panel->vmod_list); i++)
	{
		vmod = list_get(panel->vmod_list, i);
		level = list_get(panel->vmod_level_list, vmod->level);
		list_add(level->vmod_list, vmod);
	}
}


void vmod_panel_calculate_vmod_positions(struct vmod_panel_t *panel)
{
	struct vmod_t *vmod;
	struct vmod_level_t *level;

	int vmod_id;
	int level_id;
	int y;

	/* Get y coordinates */
	y = VMOD_PADDING;
	panel->max_modules_in_level = 0;
	LIST_FOR_EACH(panel->vmod_level_list, level_id)
	{
		/* Current level */
		level = list_get(panel->vmod_level_list, level_id);

		/* Record maximum number of modules in level */
		panel->max_modules_in_level = MAX(panel->max_modules_in_level, level->vmod_list->count);

		/* Calculate level width */
		level->width = level->vmod_list->count * VMOD_DEFAULT_WIDTH +
			(level->vmod_list->count - 1) * VMOD_PADDING;

		/* For each module */
		level->num_low_connections = 0;
		LIST_FOR_EACH(level->vmod_list, vmod_id)
		{
			/* Get module */
			vmod = list_get(level->vmod_list, vmod_id);

			/* Assign y */
			vmod->y = y;

			/* Accumulate number of connections in level */
			level->num_low_connections += vmod->low_vmod_list->count;
		}

		/* Go lower */
		y += VMOD_DEFAULT_HEIGHT;
		if (level_id < panel->vmod_level_list->count - 1)
		{
			y += VMOD_LEVEL_PADDING * 2;
			y += (level->num_low_connections * 2 - 1) * VMOD_CONN_LINE_WIDTH;
		}
	}
	y += VMOD_PADDING;

	/* Panel dimensions */
	panel->height = y;
	panel->width = panel->max_modules_in_level * VMOD_DEFAULT_WIDTH +
		(panel->max_modules_in_level + 1) * VMOD_PADDING;

	/* Get x coordinates */
	LIST_FOR_EACH(panel->vmod_level_list, level_id)
	{
		/* Current level */
		level = list_get(panel->vmod_level_list, level_id);

		/* For each module */
		LIST_FOR_EACH(level->vmod_list, vmod_id)
		{
			/* Get module */
			vmod = list_get(level->vmod_list, vmod_id);

			/* Assign x */
			vmod->x = panel->width / 2 - level->width / 2
				+ vmod_id * (VMOD_DEFAULT_WIDTH + VMOD_PADDING);
		}
	}
}


void vmod_panel_create_connections(struct vmod_panel_t *panel)
{
	struct vmod_t *vmod;
	struct vmod_t *low_vmod;
	struct vmod_conn_t *conn;
	struct vmod_level_t *level;

	int vmod_id;
	int low_vmod_id;
	int level_id;

	int conn_id_in_level;

	int i;

	/* Erase previous connections */
	LIST_FOR_EACH(panel->vmod_conn_list, i)
		vmod_conn_free(list_get(panel->vmod_conn_list, i));
	list_clear(panel->vmod_conn_list);

	/* Clear connection lists */
	LIST_FOR_EACH(panel->vmod_list, i)
	{
		vmod = list_get(panel->vmod_list, i);
		list_clear(vmod->high_vmod_conn_list);
		list_clear(vmod->low_vmod_conn_list);
	}

	/* Create connections */
	LIST_FOR_EACH(panel->vmod_level_list, level_id)
	{
		conn_id_in_level = 0;

		level = list_get(panel->vmod_level_list, level_id);
		LIST_FOR_EACH(level->vmod_list, vmod_id)
		{
			vmod = list_get(level->vmod_list, vmod_id);
			LIST_FOR_EACH(vmod->low_vmod_list, low_vmod_id)
			{
				int x0;
				int y0;
				int y1;
				int x2;
				int y3;

				/* Get lower module */
				low_vmod = list_get(vmod->low_vmod_list, low_vmod_id);

				/* Calculate connection location */
				x0 = vmod->x + VMOD_DEFAULT_WIDTH * (vmod->low_vmod_conn_list->count + 1)
					/ (vmod->low_vmod_list->count + 1);
				y0 = vmod->y + VMOD_DEFAULT_HEIGHT;
				y1 = y0 + VMOD_LEVEL_PADDING + conn_id_in_level * 2 * VMOD_CONN_LINE_WIDTH;
				x2 = low_vmod->x + VMOD_DEFAULT_WIDTH * (low_vmod->high_vmod_conn_list->count + 1)
					/ (low_vmod->high_vmod_list->count + 1);
				y3 = low_vmod->y;

				/* Create connection between 'vmod' and 'low_vmod' */
				conn = vmod_conn_create(x0, y0, y1, x2, y3, vmod, low_vmod);
				list_add(vmod->low_vmod_conn_list, conn);
				list_add(low_vmod->high_vmod_conn_list, conn);
				list_add(panel->vmod_conn_list, conn);
				conn_id_in_level++;
			}
		}
	}
}


void vmod_panel_insert_vmod_layouts(struct vmod_panel_t *panel)
{
	struct vmod_t *vmod;
	int i;

	/* Remove all layouts from container */
	/* FIXME */

	/* Insert all layouts */
	LIST_FOR_EACH(panel->vmod_list, i)
	{
		vmod = list_get(panel->vmod_list, i);
		gtk_layout_put(GTK_LAYOUT(panel->layout), vmod->layout, vmod->x, vmod->y);
	}
}


void vmod_panel_refresh(struct vmod_panel_t *panel)
{
	struct vmod_t *vmod1, *vmod2, *vmod3, *vmod4, *vmod5, *vmod6;

	vmod1 = vmod_create("l1-0", 0);
	vmod2 = vmod_create("l1-1", 0);
	vmod3 = vmod_create("l1-2", 0);
	vmod4 = vmod_create("l1-3", 0);
	vmod5 = vmod_create("l2-0", 1);
	vmod6 = vmod_create("l2-1", 1);

	list_add(vmod1->low_vmod_list, vmod5);
	list_add(vmod2->low_vmod_list, vmod5);
	list_add(vmod3->low_vmod_list, vmod5);
	list_add(vmod4->low_vmod_list, vmod5);
	list_add(vmod5->high_vmod_list, vmod1);
	list_add(vmod5->high_vmod_list, vmod2);
	list_add(vmod5->high_vmod_list, vmod3);
	list_add(vmod5->high_vmod_list, vmod4);

	list_add(vmod1->low_vmod_list, vmod6);
	list_add(vmod2->low_vmod_list, vmod6);
	list_add(vmod3->low_vmod_list, vmod6);
	list_add(vmod4->low_vmod_list, vmod6);
	list_add(vmod6->high_vmod_list, vmod1);
	list_add(vmod6->high_vmod_list, vmod2);
	list_add(vmod6->high_vmod_list, vmod3);
	list_add(vmod6->high_vmod_list, vmod4);

	list_add(panel->vmod_list, vmod1);
	list_add(panel->vmod_list, vmod2);
	list_add(panel->vmod_list, vmod3);
	list_add(panel->vmod_list, vmod4);
	list_add(panel->vmod_list, vmod5);
	list_add(panel->vmod_list, vmod6);

	vmod_panel_populate_vmod_levels(panel);
	vmod_panel_calculate_vmod_positions(panel);
	vmod_panel_insert_vmod_layouts(panel);
	vmod_panel_create_connections(panel);

	gtk_widget_set_size_request(panel->layout, panel->width, panel->height);
}


gboolean vmod_panel_draw_event(GtkWidget *widget, GdkEventConfigure *event,
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

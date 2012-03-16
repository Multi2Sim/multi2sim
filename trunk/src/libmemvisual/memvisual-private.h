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


#ifndef MEMVISUAL_PRIVATE_H
#define MEMVISUAL_PRIVATE_H

#include <math.h>
#include <gtk/gtk.h>

#include <memvisual.h>
#include <stdlib.h>
#include <list.h>



/*
 * Visual Memory Module
 */

struct vmod_t
{
	char *name;
	int level;

	/* GTK layout to draw */
	GtkWidget *layout;

	/* Position in panel */
	int x;
	int y;

	/* List of low and high modules */
	struct list_t *low_vmod_list;
	struct list_t *high_vmod_list;

	/* List of low and high connections */
	struct list_t *low_vmod_conn_list;
	struct list_t *high_vmod_conn_list;
};


struct vmod_t *vmod_create(char *name, int level);
void vmod_free(struct vmod_t *vmod);

gboolean vmod_draw_event(GtkWidget *widget, GdkEventConfigure *event, struct vmod_t *vmod);




/*
 * Panel with memory modules
 */

#define VMOD_PADDING  20

#define VMOD_LEVEL_PADDING  15

#define VMOD_CONN_RADIUS  8
#define VMOD_CONN_LINE_WIDTH  4

#define VMOD_DEFAULT_WIDTH  100
#define VMOD_DEFAULT_HEIGHT 100


/* One level of the memory hierarchy */
struct vmod_level_t
{
	/* List of modules in this level */
	struct list_t *vmod_list;

	/* Number of low connections */
	int num_low_connections;

	/* Width of level */
	int width;
};

/* Connection between to modules */
struct vmod_conn_t
{
	int x0;
	int y0;
	int y1;
	int x2;
	int y3;

	struct vmod_t *source;
	struct vmod_t *dest;
};

/* Panel representing memory hierarchy */
struct vmod_panel_t
{
	GtkWidget *layout;

	/* Dimensions */
	int width;
	int height;

	/* List of modules (vmod_t) */
	struct list_t *vmod_list;

	/* List of memory levels (vmod_level_t) */
	struct list_t *vmod_level_list;

	/* List of module connections (vmod_conn_t) */
	struct list_t *vmod_conn_list;

	/* Maximum number of modules in a level */
	int max_modules_in_level;

};

struct vmod_panel_t *vmod_panel_create(void);
void vmod_panel_free(struct vmod_panel_t *panel);

void vmod_panel_refresh(struct vmod_panel_t *panel);

gboolean vmod_panel_draw_event(GtkWidget *widget, GdkEventConfigure *event,
	struct vmod_panel_t *panel);

#endif


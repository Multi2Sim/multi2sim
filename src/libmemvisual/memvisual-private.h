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
#include <misc.h>



/*
 * Visual List
 */

struct vlist_item_t
{
	/* Position */
	int x;
	int y;

	/* Associated GTK widgets */
	GtkWidget *event_box;
	GtkWidget *label;

	/* Visual list where it belongs */
	struct vlist_t *vlist;

	/* Associated data element from 'vlist->elem_list' */
	void *elem;
};


struct vlist_t
{
	/* Widget showing list */
	GtkWidget *widget;

	/* Dimensions after last refresh */
	int width;
	int height;

	/* List of elements in the list. These elements can have any external type.
	 * They are controlled with 'vlist_add', 'vlist_remove', etc. macros. */
	struct list_t *elem_list;

	/* List of elements 'vlist_item_t' currently displayed in the list. This
	 * list will synchronize with 'elem_list' upon a call to 'vlist_refresh'. */
	struct list_t *item_list;

	/* Properties */
	int text_size;
};


struct vlist_t *vlist_create(void);
void vlist_free(struct vlist_t *vlist);

void vlist_refresh(struct vlist_t *vlist);




/*
 * Access to a memory module
 */

struct vmod_access_t
{
	long long id;
};

struct vmod_access_t *vmod_access_create(long long id);
void vmod_access_free(struct vmod_access_t *access);




/*
 * Visual Memory Module
 */

struct vmod_t
{
	char *name;
	int level;

	/* Widget representing module */
	GtkWidget *widget;

	/* List of low and high modules */
	struct list_t *low_vmod_list;
	struct list_t *high_vmod_list;

	/* Visual list of accesses */
	struct vlist_t *access_list;
};


struct vmod_t *vmod_create(char *name, int level);
void vmod_free(struct vmod_t *vmod);

gboolean vmod_draw_event(GtkWidget *widget, GdkEventConfigure *event, struct vmod_t *vmod);




/*
 * Panel with memory modules
 */

#define VMOD_PADDING  5

#define VMOD_DEFAULT_WIDTH  100
#define VMOD_DEFAULT_HEIGHT 100


/* One level of the memory hierarchy */
struct vmod_level_t
{
	/* List of modules in this level */
	struct list_t *vmod_list;
};


/* Panel representing memory hierarchy */
struct vmod_panel_t
{
	/* Widget representing the panel */
	GtkWidget *widget;

	/* List of modules (vmod_t) */
	struct list_t *vmod_list;

	/* List of memory levels (vmod_level_t) */
	struct list_t *vmod_level_list;

	/* Maximum number of modules in a level */
	int max_modules_in_level;

};


struct vmod_panel_t *vmod_panel_create(void);
void vmod_panel_free(struct vmod_panel_t *panel);

void vmod_panel_refresh(struct vmod_panel_t *panel);

gboolean vmod_panel_draw_event(GtkWidget *widget, GdkEventConfigure *event,
	struct vmod_panel_t *panel);

#endif


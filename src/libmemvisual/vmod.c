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
 * Module access
 */

struct vmod_access_t *vmod_access_create(long long id)
{
	struct vmod_access_t *access;

	/* Allocate */
	access = calloc(1, sizeof(struct vmod_access_t));
	if (!access)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	access->id = id;

	/* Return */
	return access;
}


void vmod_access_free(struct vmod_access_t *access)
{
	free(access);
}




/*
 * Module
 */

static void vmod_size_allocate_event(GtkWidget *widget, GdkRectangle *allocation, struct vmod_t *vmod)
{
	vlist_refresh(vmod->access_list);
}


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
	vmod->level = level;

	/* Create layout */
	vmod->widget = gtk_vbox_new(0, 0);
	g_signal_connect(G_OBJECT(vmod->widget), "size_allocate", G_CALLBACK(vmod_size_allocate_event), vmod);

	/* List of accesses */
	vmod->access_list = vlist_create();
	list_add(vmod->access_list->elem_list, vmod_access_create(random() % 100));
	list_add(vmod->access_list->elem_list, vmod_access_create(random() % 100));
	list_add(vmod->access_list->elem_list, vmod_access_create(random() % 100));
	list_add(vmod->access_list->elem_list, vmod_access_create(random() % 100));
	list_add(vmod->access_list->elem_list, vmod_access_create(random() % 100));
	gtk_container_add(GTK_CONTAINER(vmod->widget), vmod->access_list->widget);

	/* Return */
	return vmod;
}


void vmod_free(struct vmod_t *vmod)
{
	list_free(vmod->low_vmod_list);
	list_free(vmod->high_vmod_list);
	vlist_free(vmod->access_list);
	free(vmod->name);
	free(vmod);
}

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


void vmod_access_get_name(void *elem, char *buf, int size)
{
	struct vmod_access_t *access = elem;

	snprintf(buf, size, "access-%lld", access->id);
}


void vmod_access_get_desc(void *elem, char *buf, int size)
{
	struct vmod_access_t *access = elem;

	snprintf(buf, size, "Description for access-%lld", access->id);
}




/*
 * Module
 */

struct vmod_t *vmod_create(char *name, int num_sets, int assoc, int block_size,
		int sub_block_size, int num_sharers, int level)
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
	vmod->level = level;

	/* Create layout */
	vmod->widget = gtk_vbox_new(0, 0);

	/* List of accesses */
	vmod->access_list = vlist_create("Access list", 200, 30,
		vmod_access_get_name, vmod_access_get_desc);
	gtk_box_pack_start(GTK_BOX(vmod->widget), vmod->access_list->widget, FALSE, FALSE, 0);

	/* Cache */
	struct vcache_t *vcache;
	vcache = vcache_create(name, num_sets, assoc, block_size, sub_block_size, num_sharers);
	gtk_box_pack_start(GTK_BOX(vmod->widget), vcache_get_widget(vcache), TRUE, TRUE, 0);
	vmod->vcache = vcache;

	/* Return */
	return vmod;
}


void vmod_free(struct vmod_t *vmod)
{
	vlist_free(vmod->access_list);
	vcache_free(vmod->vcache);
	free(vmod->name);
	free(vmod);
}


void vmod_read_checkpoint(struct vmod_t *vmod, FILE *f)
{
	vcache_read_checkpoint(vmod->vcache, f);
}


void vmod_write_checkpoint(struct vmod_t *vmod, FILE *f)
{
	vcache_write_checkpoint(vmod->vcache, f);
}


void vmod_refresh(struct vmod_t *vmod)
{
	vcache_refresh(vmod->vcache);
}


GtkWidget *vmod_get_widget(struct vmod_t *vmod)
{
	return vmod->widget;
}

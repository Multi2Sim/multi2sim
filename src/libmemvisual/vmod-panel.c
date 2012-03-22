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
	panel->vmod_level_list = list_create();

	/* Create widget */
	panel->widget = gtk_vbox_new(0, 0);

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


void vmod_panel_insert_vmod_layouts(struct vmod_panel_t *panel)
{
	struct vmod_level_t *level;
	struct vmod_t *vmod;

	int level_id;
	int vmod_id;

	GtkWidget *hbox;

	/* Remove all layouts from container */
	/* FIXME */

	/* Insert levels */
	LIST_FOR_EACH(panel->vmod_level_list, level_id)
	{
		level = list_get(panel->vmod_level_list, level_id);

		/* Horizontal box for a new level */
		hbox = gtk_hbox_new(0, VMOD_PADDING);
		gtk_container_add(GTK_CONTAINER(panel->widget), hbox);

		/* Modules */
		LIST_FOR_EACH(level->vmod_list, vmod_id)
		{
			vmod = list_get(level->vmod_list, vmod_id);
			gtk_container_add(GTK_CONTAINER(hbox), vmod->widget);

			if (vmod_id < level->vmod_list->count - 1)
				gtk_container_add(GTK_CONTAINER(hbox), gtk_vseparator_new());
		}

		/* Horizontal bar */
		if (level_id < panel->vmod_level_list->count - 1)
			gtk_container_add(GTK_CONTAINER(panel->widget), gtk_hseparator_new());
	}
}


void vmod_panel_refresh(struct vmod_panel_t *panel)
{
	/*struct vmod_t *vmod1;

	vmod1 = vmod_create("l1-0", 0);
	list_add(panel->vmod_list, vmod1);*/

	/*struct vmod_t *vmod1, *vmod2, *vmod3, *vmod4, *vmod5, *vmod6;

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
	list_add(panel->vmod_list, vmod6);*/

	vmod_panel_populate_vmod_levels(panel);
	vmod_panel_insert_vmod_layouts(panel);
}

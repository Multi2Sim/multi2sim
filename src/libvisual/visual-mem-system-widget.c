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


#define VISUAL_MOD_PADDING		5

#define VISUAL_MOD_DEFAULT_WIDTH	100
#define VISUAL_MOD_DEFAULT_HEIGHT	100


struct visual_mem_system_widget_t
{
	GtkWidget *widget;

	struct vlist_t *access_list;
};


static void visual_mem_system_widget_destroy(GtkWidget *widget,
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

	struct visual_mod_t *mod;
	struct list_t *mod_list;

	int level_id;
	int mod_id;

	/* Vertical box */
	GtkWidget *vbox;
	vbox = gtk_vbox_new(FALSE, 0);

	/* Access list */
	struct vlist_t *access_list;
	access_list = vlist_create("Access list", 0, 0,
		(vlist_get_elem_name_func_t) visual_mod_access_get_name_buf,
		(vlist_get_elem_desc_func_t) visual_mod_access_get_desc_buf);
	gtk_box_pack_start(GTK_BOX(vbox), vlist_get_widget(access_list), FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
	widget->access_list = access_list;

	/* Insert levels */
	LIST_FOR_EACH(visual_mem_system->mod_level_list, level_id)
	{
		mod_list = list_get(visual_mem_system->mod_level_list, level_id);

		/* Empty level */
		if (!list_count(mod_list))
			continue;

		/* Horizontal box for a new level */
		GtkWidget *hbox = gtk_hbox_new(0, VISUAL_MOD_PADDING);
		gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);

		/* Modules */
		LIST_FOR_EACH(mod_list, mod_id)
		{
			/* Get module */
			mod = list_get(mod_list, mod_id);

			/* Create module widget */
			struct visual_mod_widget_t *visual_mod_widget;
			visual_mod_widget = visual_mod_widget_create(mod->name);
			gtk_box_pack_start(GTK_BOX(hbox), visual_mod_widget_get_widget(visual_mod_widget),
				TRUE, TRUE, 0);

			/* Separator */
			if (mod_id < mod_list->count - 1)
				gtk_box_pack_start(GTK_BOX(hbox), gtk_vseparator_new(), FALSE, FALSE, 0);
		}

		/* Horizontal bar */
		if (level_id < visual_mem_system->mod_level_list->count - 1)
			gtk_box_pack_start(GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
	}

	/* Assign panel widget */
	widget->widget = vbox;
	g_signal_connect(G_OBJECT(widget->widget), "destroy",
		G_CALLBACK(visual_mem_system_widget_destroy), widget);

	/* Return */
	return widget;
}


void visual_mem_system_widget_free(struct visual_mem_system_widget_t *widget)
{
	vlist_free(widget->access_list);
	free(widget);
}


GtkWidget *visual_mem_system_widget_get_widget(struct visual_mem_system_widget_t *widget)
{
	return widget->widget;
}

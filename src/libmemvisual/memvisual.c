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



struct vmem_t
{
	GtkWidget *window;

	struct vmod_panel_t *vmod_panel;
};


static gboolean vmem_window_destroy_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	//struct vmem_t *vmem = data;
	gtk_main_quit();

	return TRUE;
}


struct vmem_t *vmem_create(void)
{
	struct vmem_t *vmem;

	/* Create */
	vmem = calloc(1, sizeof(struct vmem_t));
	if (!vmem)
		fatal("%s: out of memory", __FUNCTION__);

	/* Main window */
	vmem->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(vmem->window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(vmem->window), "Multi2Sim Memory Hierarchy Visualization Tool");
	gtk_container_set_border_width(GTK_CONTAINER(vmem->window), 10);
	g_signal_connect(G_OBJECT(vmem->window), "destroy", G_CALLBACK(vmem_window_destroy_event), G_OBJECT(vmem->window));

	/* Panel */
	vmem->vmod_panel = vmod_panel_create();
	gtk_container_add(GTK_CONTAINER(vmem->window), vmem->vmod_panel->layout);

	/* Show window */
	gtk_widget_show_all(vmem->window);

	/* Return */
	return vmem;
}


void vmem_free(struct vmem_t *vmem)
{
	vmod_panel_free(vmem->vmod_panel);
	free(vmem);
}


void vmem_run(char *file_name)
{
	struct vmem_t *vmem;

	/* Initialize GTK */
	gtk_init(NULL, NULL);

	/* Create main window */
	vmem = vmem_create();

	/* Run GTK */
	gtk_main();

	/* Free main window */
	vmem_free(vmem);
}


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
	struct vcache_t *vcache;
};


static void vmem_destroy_event(GtkWidget *widget, struct vmem_t *vmem)
{
	vcache_free(vmem->vcache);
	gtk_main_quit();
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
	gtk_container_set_border_width(GTK_CONTAINER(vmem->window), 0);
	g_signal_connect(G_OBJECT(vmem->window), "destroy", G_CALLBACK(vmem_destroy_event), vmem);

	/* Panel */
	vmem->vmod_panel = vmod_panel_create();
	//gtk_container_add(GTK_CONTAINER(vmem->window), vmem->vmod_panel->widget);

	{ /////////
		struct vcache_t *vcache;
		vcache = vcache_create("Test cache", 32, 8, 64, 32, 5);
		gtk_container_add(GTK_CONTAINER(vmem->window), vcache->widget);
		vmem->vcache = vcache;
	}

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
	char *m2s_images_path = "images";

	struct vmem_t *vmem;
	struct trace_file_t *trace_file;

	/* Initialization */
	m2s_dist_file("close.png", m2s_images_path, m2s_images_path,
		vlist_image_close_path, sizeof vlist_image_close_path);
	m2s_dist_file("close-sel.png", m2s_images_path, m2s_images_path,
		vlist_image_close_sel_path, sizeof vlist_image_close_sel_path);

	/* Trace file */
	trace_file = trace_file_open(file_name);

	/* Initialize GTK */
	gtk_init(NULL, NULL);

	/* Create main window */
	vmem = vmem_create();

	/* Run GTK */
	gtk_main();

	/* Free main window */
	vmem_free(vmem);
	trace_file_close(trace_file);
}


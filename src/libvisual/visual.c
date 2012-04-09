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



/*
 * Main Program
 */

struct state_file_t *visual_state_file;
struct cycle_bar_t *visual_cycle_bar;
struct visual_mem_system_widget_t *visual_mem_system_widget;


static void visual_xxx_destroy_event(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}


static void visual_cycle_bar_refresh(void *user_data, long long cycle)
{
	visual_mem_system_widget_refresh(visual_mem_system_widget);
}


void vmem_run(char *file_name)
{
	char *m2s_images_path = "images";

	/* Initialization */
	m2s_dist_file("close.png", m2s_images_path, m2s_images_path,
		vlist_image_close_path, sizeof vlist_image_close_path);
	m2s_dist_file("close-sel.png", m2s_images_path, m2s_images_path,
		vlist_image_close_sel_path, sizeof vlist_image_close_sel_path);
	m2s_dist_file("back-single.png", m2s_images_path, m2s_images_path,
		cycle_bar_back_single_path, sizeof cycle_bar_back_single_path);
	m2s_dist_file("back-double.png", m2s_images_path, m2s_images_path,
		cycle_bar_back_double_path, sizeof cycle_bar_back_double_path);
	m2s_dist_file("back-triple.png", m2s_images_path, m2s_images_path,
		cycle_bar_back_triple_path, sizeof cycle_bar_back_triple_path);
	m2s_dist_file("forward-single.png", m2s_images_path, m2s_images_path,
		cycle_bar_forward_single_path, sizeof cycle_bar_forward_single_path);
	m2s_dist_file("forward-double.png", m2s_images_path, m2s_images_path,
		cycle_bar_forward_double_path, sizeof cycle_bar_forward_double_path);
	m2s_dist_file("forward-triple.png", m2s_images_path, m2s_images_path,
		cycle_bar_forward_triple_path, sizeof cycle_bar_forward_triple_path);
	m2s_dist_file("go.png", m2s_images_path, m2s_images_path,
		cycle_bar_go_path, sizeof cycle_bar_go_path);

	/* Create state file */
	visual_state_file = state_file_create(file_name);

	/* Initialize visual systems */
	visual_mem_system_init();

	/* Create checkpoints */
	state_file_create_checkpoints(visual_state_file);

	/* Initialize GTK */
	gtk_init(NULL, NULL);

	/* Create main window */
	GtkWidget *window;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(visual_xxx_destroy_event), NULL);

	/* Vertical box */
	GtkWidget *vbox;
	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	/* Cycle bar */
	visual_cycle_bar = cycle_bar_create();
	gtk_box_pack_start(GTK_BOX(vbox), cycle_bar_get_widget(visual_cycle_bar), FALSE, FALSE, 0);
	cycle_bar_set_refresh_func(visual_cycle_bar, visual_cycle_bar_refresh, NULL);

	/* Memory system widget */
	visual_mem_system_widget = visual_mem_system_widget_create();
	gtk_box_pack_start(GTK_BOX(vbox), visual_mem_system_widget_get_widget(visual_mem_system_widget),
		TRUE, TRUE, 0);

	/* Show */
	gtk_widget_show_all(window);

	/* Run GTK */
	gtk_main();

	/* Free */
	visual_mem_system_done();
	state_file_free(visual_state_file);
}

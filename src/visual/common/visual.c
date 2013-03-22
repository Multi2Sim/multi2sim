/*
 *  Multi2Sim Tools
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
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

#include <lib/util/misc.h>
#include <visual/evergreen/gpu.h>
#include <visual/evergreen/panel.h>
#include <visual/memory/mem-system.h>
#include <visual/memory/panel.h>
#include <visual/southern-islands/gpu.h>
#include <visual/southern-islands/panel.h>
#include <visual/x86/cpu.h>
#include <visual/x86/panel.h>

#include "cycle-bar.h"
#include "list.h"
#include "state.h"
#include "visual.h"



/*
 * Main Program
 */

struct vi_cycle_bar_t *vi_cycle_bar;
struct vi_mem_panel_t *vi_mem_panel;
struct vi_evg_panel_t *vi_evg_panel;
struct vi_si_panel_t  *vi_si_panel;
struct vi_x86_panel_t *vi_x86_panel;


static void visual_destroy_event(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}


static void visual_cycle_bar_refresh(void *user_data, long long cycle)
{
	/* Memory */
	vi_mem_panel_refresh(vi_mem_panel);

	/* Evergreen */
	if (vi_evg_gpu->active)
		vi_evg_panel_refresh(vi_evg_panel);

	/* Southern Islands */
	if (vi_si_gpu->active)
		vi_si_panel_refresh(vi_si_panel);

	/* x86 */
	if (vi_x86_cpu->active)
		vi_x86_panel_refresh(vi_x86_panel);
}


void visual_run(char *file_name)
{
	char *m2s_images_path = "images";
	char vi_icon_path[MAX_STRING_SIZE];

	/* Initialization */
	m2s_dist_file("close.png", m2s_images_path, m2s_images_path,
		vi_list_image_close_path, sizeof vi_list_image_close_path);
	m2s_dist_file("close-sel.png", m2s_images_path, m2s_images_path,
		vi_list_image_close_sel_path, sizeof vi_list_image_close_sel_path);
	m2s_dist_file("m2s-icon.png", m2s_images_path, m2s_images_path,
		vi_icon_path, sizeof vi_icon_path);

	/* Initialize visual state */
	vi_state_init(file_name);

	/* Initialize visual systems */
	vi_mem_system_init();
	vi_evg_gpu_init();
	vi_si_gpu_init();
	vi_x86_cpu_init();

	/* Create checkpoints */
	vi_state_create_checkpoints();

	/* Initialize GTK */
	gtk_init(NULL, NULL);

	/* Create main window */
	GtkWidget *window;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "M2S-Visual");
	gtk_window_set_icon_from_file(GTK_WINDOW(window), vi_icon_path, NULL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(visual_destroy_event), NULL);

	/* Vertical box */
	GtkWidget *vbox;
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);

	/* Cycle bar */
	vi_cycle_bar_init(visual_cycle_bar_refresh, NULL);
	gtk_box_pack_start(GTK_BOX(vbox), vi_cycle_bar_get_widget(), FALSE, FALSE, 0);

	/* x86 panel */
	if (vi_x86_cpu->active)
	{
		vi_x86_panel = vi_x86_panel_create();
		gtk_box_pack_start(GTK_BOX(vbox), vi_x86_panel_get_widget(vi_x86_panel),
				FALSE, FALSE, 0);
	}

	/* Evergreen panel */
	if (vi_evg_gpu->active)
	{
		vi_evg_panel = vi_evg_panel_create();
		gtk_box_pack_start(GTK_BOX(vbox), vi_evg_panel_get_widget(vi_evg_panel),
				FALSE, FALSE, 0);
	}

	/* Southern Islands panel */
	if (vi_si_gpu->active)
	{
		vi_si_panel = vi_si_panel_create();
		gtk_box_pack_start(GTK_BOX(vbox), vi_si_panel_get_widget(vi_si_panel),
				FALSE, FALSE, 0);
	}

	/* Memory system panel */
	vi_mem_panel = vi_mem_panel_create();
	gtk_box_pack_start(GTK_BOX(vbox), vi_mem_panel_get_widget(vi_mem_panel),
		TRUE, TRUE, 0);

	/* Show */
	gtk_widget_show_all(window);

	/* Run GTK */
	gtk_main();

	/* Free */
	vi_x86_cpu_done();
	vi_evg_gpu_done();
	vi_si_gpu_done();
	vi_mem_system_done();
	vi_cycle_bar_done();
	vi_state_done();
}

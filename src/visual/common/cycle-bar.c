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


#include <lib/mhandle/mhandle.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "cycle-bar.h"
#include "state.h"


static char cycle_bar_back_single_path[MAX_PATH_SIZE];
static char cycle_bar_back_double_path[MAX_PATH_SIZE];
static char cycle_bar_back_triple_path[MAX_PATH_SIZE];

static char cycle_bar_forward_single_path[MAX_PATH_SIZE];
static char cycle_bar_forward_double_path[MAX_PATH_SIZE];
static char cycle_bar_forward_triple_path[MAX_PATH_SIZE];

static char cycle_bar_go_path[MAX_PATH_SIZE];


struct vi_cycle_bar_t
{
	GtkWidget *widget;
	GtkWidget *scale;
	GtkWidget *go_to_entry;

	vi_cycle_bar_refresh_func_t refresh_func;
	void *refresh_func_arg;
};


static struct vi_cycle_bar_t *vi_cycle_bar;


static void vi_cycle_bar_go_to_cycle(long long cycle)
{
	long long num_cycles;

	char str[MAX_STRING_SIZE];

	/* Adjust cycle range */
	num_cycles = vi_state_get_num_cycles();
	cycle = MAX(0, cycle);
	cycle = MIN(cycle, num_cycles);

	/* Write new cycle in text entry */
	snprintf(str, sizeof str, "%lld", cycle);
	gtk_entry_set_text(GTK_ENTRY(vi_cycle_bar->go_to_entry), str);

	/* Set value of scale */
	gtk_range_set_value(GTK_RANGE(vi_cycle_bar->scale), cycle);

	/* Refresh */
	if (vi_cycle_bar->refresh_func)
		vi_cycle_bar->refresh_func(vi_cycle_bar->refresh_func_arg, cycle);
}


static gboolean vi_cycle_bar_change_value_event(GtkWidget *widget, GtkScrollType scroll,
	double value, gpointer data)
{
	vi_cycle_bar_go_to_cycle(value);
	return FALSE;
}


static gboolean vi_cycle_bar_back_single_clicked_event(GtkWidget *button, gpointer data)
{
	long long cycle;

	cycle = gtk_range_get_value(GTK_RANGE(vi_cycle_bar->scale));
	vi_cycle_bar_go_to_cycle(cycle - 1);
	return FALSE;
}


static gboolean vi_cycle_bar_back_double_clicked_event(GtkWidget *button, gpointer data)
{
	long long cycle;

	cycle = gtk_range_get_value(GTK_RANGE(vi_cycle_bar->scale));
	vi_cycle_bar_go_to_cycle(cycle - 10);
	return FALSE;
}


static gboolean vi_cycle_bar_back_triple_clicked_event(GtkWidget *button, gpointer data)
{
	long long cycle;

	cycle = gtk_range_get_value(GTK_RANGE(vi_cycle_bar->scale));
	vi_cycle_bar_go_to_cycle(cycle - 100);
	return FALSE;
}


static gboolean vi_cycle_bar_forward_single_clicked_event(GtkWidget *button, gpointer data)
{
	long long cycle;

	cycle = gtk_range_get_value(GTK_RANGE(vi_cycle_bar->scale));
	vi_cycle_bar_go_to_cycle(cycle + 1);
	return FALSE;
}


static gboolean vi_cycle_bar_forward_double_clicked_event(GtkWidget *button, gpointer data)
{
	long long cycle;

	cycle = gtk_range_get_value(GTK_RANGE(vi_cycle_bar->scale));
	vi_cycle_bar_go_to_cycle(cycle + 10);
	return FALSE;
}


static gboolean vi_cycle_bar_forward_triple_clicked_event(GtkWidget *button, gpointer data)
{
	long long cycle;

	cycle = gtk_range_get_value(GTK_RANGE(vi_cycle_bar->scale));
	vi_cycle_bar_go_to_cycle(cycle + 100);
	return FALSE;
}


static gboolean vi_cycle_bar_go_to_clicked_event(GtkWidget *button, gpointer data)
{
	long long cycle;

	cycle = atoll(gtk_entry_get_text(GTK_ENTRY(vi_cycle_bar->go_to_entry)));
	vi_cycle_bar_go_to_cycle(cycle);
	return FALSE;
}


static gboolean cycle_bar_go_to_key_press_event(GtkWidget *entry, GdkEventKey *event, gpointer data)
{
	long long cycle;

	if (event->type == GDK_KEY_PRESS && (event->keyval == 0xff8d || event->keyval == 0xff0d))
	{
		cycle = atoll(gtk_entry_get_text(GTK_ENTRY(vi_cycle_bar->go_to_entry)));
		vi_cycle_bar_go_to_cycle(cycle);
		gtk_editable_select_region(GTK_EDITABLE(vi_cycle_bar->go_to_entry), 0, -1);
	}
	return FALSE;
}


void vi_cycle_bar_init(vi_cycle_bar_refresh_func_t refresh_func, void *user_data)
{
	char *m2s_images_path = "images";

	/* Initialize */
	vi_cycle_bar = xcalloc(1, sizeof(struct vi_cycle_bar_t));
	vi_cycle_bar->refresh_func = refresh_func;
	vi_cycle_bar->refresh_func_arg = user_data;

	/* Icons */
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

	/* Buttons */
	GtkWidget *back_single_button = gtk_button_new();
	GtkWidget *back_double_button = gtk_button_new();
	GtkWidget *back_triple_button = gtk_button_new();
	GtkWidget *forward_single_button = gtk_button_new();
	GtkWidget *forward_double_button = gtk_button_new();
	GtkWidget *forward_triple_button = gtk_button_new();
	g_signal_connect(G_OBJECT(back_single_button), "clicked", G_CALLBACK(vi_cycle_bar_back_single_clicked_event), vi_cycle_bar);
	g_signal_connect(G_OBJECT(back_double_button), "clicked", G_CALLBACK(vi_cycle_bar_back_double_clicked_event), vi_cycle_bar);
	g_signal_connect(G_OBJECT(back_triple_button), "clicked", G_CALLBACK(vi_cycle_bar_back_triple_clicked_event), vi_cycle_bar);
	g_signal_connect(G_OBJECT(forward_single_button), "clicked", G_CALLBACK(vi_cycle_bar_forward_single_clicked_event), vi_cycle_bar);
	g_signal_connect(G_OBJECT(forward_double_button), "clicked", G_CALLBACK(vi_cycle_bar_forward_double_clicked_event), vi_cycle_bar);
	g_signal_connect(G_OBJECT(forward_triple_button), "clicked", G_CALLBACK(vi_cycle_bar_forward_triple_clicked_event), vi_cycle_bar);

	/* Scale */
	long long num_cycles = vi_state_get_num_cycles();
	GtkWidget *scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, num_cycles, 1);
	gtk_widget_set_size_request(scale, 100, 32);
	gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
	g_signal_connect(G_OBJECT(scale), "change-value", G_CALLBACK(vi_cycle_bar_change_value_event), vi_cycle_bar);
	vi_cycle_bar->scale = scale;


	/* Images */
	GtkWidget *back_single_image = gtk_image_new_from_file(cycle_bar_back_single_path);
	GtkWidget *back_double_image = gtk_image_new_from_file(cycle_bar_back_double_path);
	GtkWidget *back_triple_image = gtk_image_new_from_file(cycle_bar_back_triple_path);
	GtkWidget *forward_single_image = gtk_image_new_from_file(cycle_bar_forward_single_path);
	GtkWidget *forward_double_image = gtk_image_new_from_file(cycle_bar_forward_double_path);
	GtkWidget *forward_triple_image = gtk_image_new_from_file(cycle_bar_forward_triple_path);
	gtk_container_add(GTK_CONTAINER(back_single_button), back_single_image);
	gtk_container_add(GTK_CONTAINER(back_double_button), back_double_image);
	gtk_container_add(GTK_CONTAINER(back_triple_button), back_triple_image);
	gtk_container_add(GTK_CONTAINER(forward_single_button), forward_single_image);
	gtk_container_add(GTK_CONTAINER(forward_double_button), forward_double_image);
	gtk_container_add(GTK_CONTAINER(forward_triple_button), forward_triple_image);

	/* Table */
	GtkWidget *navigation_table = gtk_table_new(1, 7, FALSE);
	gtk_table_attach(GTK_TABLE(navigation_table), back_triple_button, 0, 1, 0, 1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(navigation_table), back_double_button, 1, 2, 0, 1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(navigation_table), back_single_button, 2, 3, 0, 1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(navigation_table), scale, 3, 4, 0, 1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(navigation_table), forward_single_button, 4, 5, 0, 1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(navigation_table), forward_double_button, 5, 6, 0, 1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(navigation_table), forward_triple_button, 6, 7, 0, 1, 0, 0, 0, 0);

	/* Label */
	GtkWidget *go_to_label = gtk_label_new("Current cycle:");
	gtk_misc_set_alignment(GTK_MISC(go_to_label), 0, 0.5);

	/* Go-to-cycle button */
	GtkWidget *go_to_button = gtk_button_new();
	GtkWidget *go_to_image = gtk_image_new_from_file(cycle_bar_go_path);
	gtk_container_add(GTK_CONTAINER(go_to_button), go_to_image);
	g_signal_connect(G_OBJECT(go_to_button), "clicked", G_CALLBACK(vi_cycle_bar_go_to_clicked_event), vi_cycle_bar);

	/* Go-to-cycle text entry */
	GtkWidget *go_to_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(go_to_entry), "0");
	gtk_entry_set_width_chars(GTK_ENTRY(go_to_entry), 10);
	g_signal_connect(G_OBJECT(go_to_entry), "key-press-event", G_CALLBACK(cycle_bar_go_to_key_press_event), vi_cycle_bar);
	vi_cycle_bar->go_to_entry = go_to_entry;

	/* Table */
	GtkWidget *go_to_table = gtk_table_new(2, 2, FALSE);
	gtk_table_attach(GTK_TABLE(go_to_table), go_to_label, 0, 2, 0, 1, GTK_FILL | GTK_EXPAND, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(go_to_table), go_to_entry, 0, 1, 1, 2, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(go_to_table), go_to_button, 1, 2, 1, 2, 0, 0, 0, 0);

	/* Spacer */
	GtkWidget *spacer = gtk_label_new("");
	gtk_widget_set_size_request(spacer, 5, -1);

	/* Horizontal box with the two tables */
	GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start(GTK_BOX(hbox), navigation_table, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), spacer, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), go_to_table, FALSE, FALSE, 0);

	/* Frame with everything */
	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(frame), hbox);

	/* Main widget */
	vi_cycle_bar->widget = frame;
}


void vi_cycle_bar_done(void)
{
	free(vi_cycle_bar);
}


GtkWidget *vi_cycle_bar_get_widget(void)
{
	return vi_cycle_bar->widget;
}


long long vi_cycle_bar_get_cycle(void)
{
	long long cycle;

	cycle = gtk_range_get_value(GTK_RANGE(vi_cycle_bar->scale));
	return cycle;
}

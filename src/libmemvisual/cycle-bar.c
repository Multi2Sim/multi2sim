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


char cycle_bar_back_single_path[MAX_PATH_SIZE];
char cycle_bar_back_double_path[MAX_PATH_SIZE];
char cycle_bar_back_triple_path[MAX_PATH_SIZE];

char cycle_bar_forward_single_path[MAX_PATH_SIZE];
char cycle_bar_forward_double_path[MAX_PATH_SIZE];
char cycle_bar_forward_triple_path[MAX_PATH_SIZE];


struct cycle_bar_t
{
	GtkWidget *widget;
	GtkWidget *scale;
};


static void cycle_bar_destroy_event(GtkWidget *widget, struct cycle_bar_t *cycle_bar)
{
	free(cycle_bar);
}


struct cycle_bar_t *cycle_bar_create(void)
{
	struct cycle_bar_t *cycle_bar;

	/* Allocate */
	cycle_bar = calloc(1, sizeof(struct cycle_bar_t));
	if (!cycle_bar)
		fatal("%s: out of memory", __FUNCTION__);

	/* Buttons */
	GtkWidget *back_single_button = gtk_button_new();
	GtkWidget *back_double_button = gtk_button_new();
	GtkWidget *back_triple_button = gtk_button_new();
	GtkWidget *forward_single_button = gtk_button_new();
	GtkWidget *forward_double_button = gtk_button_new();
	GtkWidget *forward_triple_button = gtk_button_new();

	/* Scale */
	GtkWidget *scale = gtk_hscale_new_with_range(0, 100, 1);
	gtk_widget_set_size_request(scale, 100, 32);
	gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
	//gtk_range_set_range(GTK_RANGE(cycle_scale), 0, vgpu->max_cycles);
	//g_signal_connect(G_OBJECT(cycle_scale), "change-value", G_CALLBACK(cycle_scale_change_value_event), vgpu);
	cycle_bar->scale = scale;


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
	GtkWidget *table = gtk_table_new(1, 7, FALSE);
	gtk_table_attach(GTK_TABLE(table), back_triple_button, 0, 1, 0, 1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), back_double_button, 1, 2, 0, 1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), back_single_button, 2, 3, 0, 1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), scale, 3, 4, 0, 1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), forward_single_button, 4, 5, 0, 1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), forward_double_button, 5, 6, 0, 1, 0, 0, 0, 0);
	gtk_table_attach(GTK_TABLE(table), forward_triple_button, 6, 7, 0, 1, 0, 0, 0, 0);

	/* Frame with everything */
	GtkWidget *frame = gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(frame), table);

	/* Main widget */
	cycle_bar->widget = frame;
	g_signal_connect(G_OBJECT(cycle_bar->widget), "destroy",
		G_CALLBACK(cycle_bar_destroy_event), cycle_bar);

	/* Return */
	return cycle_bar;
}


void cycle_bar_free(struct cycle_bar_t *cycle_bar)
{
	free(cycle_bar);
}


GtkWidget *cycle_bar_get_widget(struct cycle_bar_t *cycle_bar)
{
	return cycle_bar->widget;
}

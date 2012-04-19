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

#include <visual-evergreen.h>



struct vi_evg_time_dia_t
{
	GtkWidget *widget;

	struct vi_evg_compute_unit_t *compute_unit;
};


static void vi_evg_time_dia_widget_destroy(GtkWidget *widget, struct vi_evg_time_dia_t *time_dia)
{
	vi_evg_time_dia_free(time_dia);
}


struct vi_evg_time_dia_t *vi_evg_time_dia_create(struct vi_evg_compute_unit_t *compute_unit)
{
	struct vi_evg_time_dia_t *time_dia;

	/* Allocate */
	time_dia = calloc(1, sizeof(struct vi_evg_time_dia_t));
	if (!time_dia)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	time_dia->compute_unit = compute_unit;

	/* Layout */
	GtkWidget *layout;
	layout = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(layout, 100, 100);

	/* Widget */
	time_dia->widget = layout;
	g_signal_connect(G_OBJECT(time_dia->widget), "destroy",
		G_CALLBACK(vi_evg_time_dia_widget_destroy), time_dia);

	/* Return */
	return time_dia;
}


void vi_evg_time_dia_free(struct vi_evg_time_dia_t *time_dia)
{
	free(time_dia);
}


void vi_evg_time_dia_refresh(struct vi_evg_time_dia_t *time_dia)
{
}


GtkWidget *vi_evg_time_dia_get_widget(struct vi_evg_time_dia_t *time_dia)
{
	return time_dia->widget;
}

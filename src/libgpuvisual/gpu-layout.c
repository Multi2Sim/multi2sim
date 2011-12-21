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

#include <gpuvisual-private.h>

/* Size for compute units */
#define VGPU_COMPUTE_UNIT_WIDTH  100
#define VGPU_COMPUTE_UNIT_HEIGHT  100
#define VGPU_COMPUTE_UNIT_SPACING  15


/* Status indicator - draw a circle */
static gboolean compute_unit_status_expose_event(GtkWidget *widget, GdkEventConfigure *event, struct vgpu_compute_unit_t *compute_unit)
{
	GdkWindow *window;
	cairo_t *cr;

	int width;
	int height;

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);

	window = gtk_widget_get_window(widget);
	cr = gdk_cairo_create(window);
	if (list_count(compute_unit->work_group_list))
		cairo_set_source_rgb(cr, 1, 0, 0);
	else
		cairo_set_source_rgb(cr, 0, 1, 0);
	cairo_set_line_width(cr, 1);
	cairo_arc(cr, width / 2, height / 2.0, MIN(width, height) / 3.0, 0., 2. * 3.141592);
	cairo_fill_preserve(cr);
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_stroke(cr);
	cairo_destroy(cr);
	return FALSE;
}


static gboolean timing_dia_toggle_button_toggled_event(GtkWidget *widget, struct vgpu_compute_unit_t *compute_unit)
{
	int active;
	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	if (active && !compute_unit->timing_dia_active)
		timing_dia_window_show(compute_unit);
	if (!active && compute_unit->timing_dia_active)
		timing_dia_window_hide(compute_unit);
	return FALSE;
}


static gboolean block_dia_toggle_button_toggled_event(GtkWidget *widget, struct vgpu_compute_unit_t *compute_unit)
{
	int active;
	active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
	if (active && !compute_unit->block_dia_active)
		block_dia_window_show(compute_unit);
	if (!active && compute_unit->block_dia_active)
		block_dia_window_hide(compute_unit);
	return FALSE;
}


/* Refresh vgpu layout contents */
static void vgpu_layout_refresh(struct vgpu_t *vgpu)
{
	int i;
	int x, y;
	int width;

	GtkWidget *layout = vgpu->layout;

	/* Delete all components */
	GList *list;
	list = gtk_container_get_children(GTK_CONTAINER(layout));
	while (list)
	{
		GtkWidget *widget;
		widget = GTK_WIDGET(list->data);
		gtk_container_remove(GTK_CONTAINER(layout), widget);
		list = g_list_remove(list, list->data);
	}

	/* Draw compute units */
	x = y = VGPU_COMPUTE_UNIT_SPACING;
	width = gtk_widget_get_allocated_width(vgpu->layout);
	for (i = 0; i < vgpu->num_compute_units; i++)
	{
		struct vgpu_compute_unit_t *compute_unit;
		char str[MAX_STRING_SIZE];

		compute_unit = list_get(vgpu->compute_unit_list, i);

		/* Position */
		if (x > VGPU_COMPUTE_UNIT_SPACING && x + VGPU_COMPUTE_UNIT_WIDTH + VGPU_COMPUTE_UNIT_SPACING >= width)
		{
			x = VGPU_COMPUTE_UNIT_SPACING;
			y += VGPU_COMPUTE_UNIT_SPACING + VGPU_COMPUTE_UNIT_HEIGHT;
		}

		/* Event box and frame */
		GtkWidget *event_box;
		GtkWidget *compute_unit_frame;
		snprintf(str, sizeof str, "CU-%d", compute_unit->id);
		event_box = gtk_event_box_new();
		compute_unit_frame = gtk_frame_new(str);
		gtk_widget_set_size_request(compute_unit_frame, VGPU_COMPUTE_UNIT_WIDTH, VGPU_COMPUTE_UNIT_HEIGHT);
		gtk_container_add(GTK_CONTAINER(event_box), compute_unit_frame);
		gtk_layout_put(GTK_LAYOUT(layout), event_box, x, y);

		/* Table */
		GtkWidget *table;
		table = gtk_table_new(2, 4, FALSE);
		gtk_container_add(GTK_CONTAINER(compute_unit_frame), table);

		/* Status */
		GtkWidget *status_area;
		status_area = gtk_drawing_area_new();
		gtk_widget_set_size_request(status_area, VGPU_COMPUTE_UNIT_WIDTH / 4, -1);
		g_signal_connect(G_OBJECT(status_area), "draw", G_CALLBACK(compute_unit_status_expose_event),
			compute_unit);
		gtk_table_attach(GTK_TABLE(table), status_area, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
		compute_unit->status_widget = status_area;

		/* Toggle button */
		GtkWidget *timing_dia_toggle_button;
		timing_dia_toggle_button = gtk_toggle_button_new_with_label("T");
		gtk_button_set_focus_on_click(GTK_BUTTON(timing_dia_toggle_button), FALSE);
		gtk_table_attach(GTK_TABLE(table), timing_dia_toggle_button, 1, 2, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
		if (compute_unit->timing_dia_active)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(timing_dia_toggle_button), TRUE);
		g_signal_connect(G_OBJECT(timing_dia_toggle_button), "toggled", G_CALLBACK(timing_dia_toggle_button_toggled_event), compute_unit);
		compute_unit->timing_dia_toggle_button = timing_dia_toggle_button;

		/* Toggle button */
		GtkWidget *block_dia_toggle_button;
		block_dia_toggle_button = gtk_toggle_button_new_with_label("B");
		gtk_button_set_focus_on_click(GTK_BUTTON(block_dia_toggle_button), FALSE);
		gtk_table_attach(GTK_TABLE(table), block_dia_toggle_button, 2, 3, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
		if (compute_unit->block_dia_active)
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(block_dia_toggle_button), TRUE);
		g_signal_connect(G_OBJECT(block_dia_toggle_button), "toggled", G_CALLBACK(block_dia_toggle_button_toggled_event), compute_unit);
		compute_unit->block_dia_toggle_button = block_dia_toggle_button;

		/* Empty space */
		GtkWidget *pad;
		pad = gtk_drawing_area_new();
		gtk_widget_set_size_request(pad, VGPU_COMPUTE_UNIT_WIDTH / 4, -1);
		gtk_table_attach(GTK_TABLE(table), pad, 3, 4, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);

		/* Layout */
		compute_unit->work_group_list_layout = list_layout_new(main_window, "Work-group list",
			compute_unit->work_group_list, 12,
			work_group_get_name, work_group_info_popup);
		gtk_table_attach(GTK_TABLE(table), compute_unit->work_group_list_layout->layout, 0, 4, 1, 2,
			GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 0, 0);

		/* Insert into layout */
		gtk_widget_show_all(event_box);

		/* Next position */
		x += VGPU_COMPUTE_UNIT_SPACING + VGPU_COMPUTE_UNIT_WIDTH;
	}

	/* Adjust layout size and record new dimensions */
	vgpu->layout_width = width;
	vgpu->layout_height = y + VGPU_COMPUTE_UNIT_HEIGHT + VGPU_COMPUTE_UNIT_SPACING;
	gtk_widget_set_size_request(layout, -1, vgpu->layout_height);
}


/* Draw vgpu */
static gboolean vgpu_widget_size_allocate_event(GtkWidget *widget, GdkEventConfigure *event, struct vgpu_t *vgpu)
{
	struct vgpu_compute_unit_t *compute_unit;
	int width, height;
	int i;

	/* Check if dimensions have actually changed */
	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	if (vgpu->layout_width == width && vgpu->layout_height == height)
		return FALSE;

	/* Refresh vgpu layout */
	vgpu_layout_refresh(vgpu);
	for (i = 0; i < vgpu->num_compute_units; i++)
	{
		compute_unit = list_get(vgpu->compute_unit_list, i);
		list_layout_refresh(compute_unit->work_group_list_layout);
	}

	return FALSE;
}


/* Create vgpu widget */
GtkWidget *vgpu_widget_new(struct vgpu_t *vgpu)
{
	GdkColor color;

	/* Create scrolled window and layout */
	vgpu->widget = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_size_request(vgpu->widget, VGPU_COMPUTE_UNIT_WIDTH + VGPU_COMPUTE_UNIT_SPACING * 2 + 5, -1);
	vgpu->layout = gtk_layout_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(vgpu->widget), vgpu->layout);
	gtk_widget_add_events(vgpu->layout, GDK_STRUCTURE_MASK);
	g_signal_connect(G_OBJECT(vgpu->widget), "size-allocate", G_CALLBACK(vgpu_widget_size_allocate_event), vgpu);

	/* Make it white */
	gdk_color_parse("white", &color);
	gtk_widget_modify_bg(vgpu->layout, GTK_STATE_NORMAL, &color);

	/* Return it */
	return vgpu->widget;
}


void vgpu_widget_refresh(struct vgpu_t *vgpu)
{
	struct vgpu_compute_unit_t *compute_unit;
	int i;

	vgpu_layout_refresh(vgpu);
	for (i = 0; i < vgpu->num_compute_units; i++) {
		compute_unit = list_get(vgpu->compute_unit_list, i);
		list_layout_refresh(compute_unit->work_group_list_layout);
		compute_unit_status_expose_event(compute_unit->status_widget, NULL, compute_unit);
		timing_dia_window_refresh(compute_unit);
		block_dia_window_refresh(compute_unit);
	}
}


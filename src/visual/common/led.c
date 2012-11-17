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

#include <math.h>

#include <lib/mhandle/mhandle.h>

#include "led.h"


struct vi_led_t
{
	GtkWidget *widget;

	GdkColor color;
};


static void vi_led_destroy(GtkWidget *widget, struct vi_led_t *led)
{
	vi_led_free(led);
}


static gboolean vi_led_draw(GtkWidget *widget, GdkEventConfigure *event, struct vi_led_t *led)
{
	GdkWindow *window;
	cairo_t *cr;

	int width;
	int height;

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);

	window = gtk_widget_get_window(widget);
	cr = gdk_cairo_create(window);

	/* Color */
	cairo_set_source_rgb(cr, (double) led->color.red / 0xffff,
		(double) led->color.green / 0xffff, (double) led->color.blue / 0xffff);

	/* Circle */
	cairo_set_line_width(cr, 1);
	cairo_arc(cr, width / 2, height / 2.0, MIN(width, height) / 3.0, 0., 2 * M_PI);
	cairo_fill_preserve(cr);
	cairo_set_source_rgb(cr, 0, 0, 0);

	/* Finish */
	cairo_stroke(cr);
	cairo_destroy(cr);
	return FALSE;
}


struct vi_led_t *vi_led_create(int radius)
{
	struct vi_led_t *led;

	/* Initialize */
	led = xcalloc(1, sizeof(struct vi_led_t));
	gdk_color_parse("green", &led->color);

	/* Drawing box */
	GtkWidget *drawing_area = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawing_area, radius * 2, radius * 2);
	g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(vi_led_draw), led);

	/* Main widget */
	led->widget = drawing_area;
	g_signal_connect(G_OBJECT(led->widget), "destroy", G_CALLBACK(vi_led_destroy), led);

	/* Return */
	return led;
}


void vi_led_free(struct vi_led_t *led)
{
	free(led);
}


void vi_led_set_color(struct vi_led_t *led, GdkColor *color)
{
	led->color = *color;
	vi_led_draw(led->widget, NULL, led);
}


GtkWidget *vi_led_get_widget(struct vi_led_t *led)
{
	return led->widget;
}


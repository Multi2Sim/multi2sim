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


static gboolean info_popup_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	int width;
	int height;

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	if (event->x < 0 || event->y < 0 || event->x > width || event->y > height)
	{
		gdk_pointer_ungrab(GDK_CURRENT_TIME);
		gtk_widget_destroy(widget);
	}
	return FALSE;
}


static gboolean info_popup_motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	int width;
	int height;

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	if (event->x < -20 || event->y < -20 || event->x > width + 20 || event->y > height + 20)
	{
		gdk_pointer_ungrab(GDK_CURRENT_TIME);
		gtk_widget_destroy(widget);
	}
	return FALSE;
}


static void info_popup_destroy_event(GtkWidget *widget, struct info_popup_t *popup)
{
	info_popup_free(popup);
}


struct info_popup_t *info_popup_create(char *text)
{
	struct info_popup_t *popup;

	/* Allocate */
	popup = calloc(1, sizeof(struct info_popup_t));
	if (!popup)
		fatal("%s: out of memory", __FUNCTION__);

	/* Create window */
	GtkWidget *window;
	window = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK);
	g_signal_connect(G_OBJECT(window), "button-press-event",
		G_CALLBACK(info_popup_button_press_event), popup);
	g_signal_connect(G_OBJECT(window), "motion-notify-event",
		G_CALLBACK(info_popup_motion_event), popup);
	g_signal_connect(G_OBJECT(window), "destroy",
		G_CALLBACK(info_popup_destroy_event), popup);
	popup->window = window;

	/* Create label with text */
	GtkWidget *label;
	label = gtk_label_new(text);
	gtk_widget_set_size_request(window, -1, 250);
	//gtk_label_set_selectable(GTK_LABEL(label), TRUE);
	gtk_label_set_use_markup(GTK_LABEL(label), TRUE);
	gtk_label_set_line_wrap_mode(GTK_LABEL(label), TRUE);
	//gtk_label_select_region(GTK_LABEL(label), 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0);

	/* Label font */
	PangoAttrList *attrs;
	attrs = pango_attr_list_new();
	PangoAttribute *attr_family = pango_attr_family_new("Courier");
	PangoAttribute *attr_size = pango_attr_size_new(12 << 10);
	pango_attr_list_insert(attrs, attr_family);
	pango_attr_list_insert(attrs, attr_size);
	gtk_label_set_attributes(GTK_LABEL(label), attrs);

	/* Scrolled window */
	GtkWidget *scrolled_window;
	GtkWidget *viewport;
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
		GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
	viewport = gtk_viewport_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(viewport), label);
	gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);
	gtk_container_add(GTK_CONTAINER(window), scrolled_window);
	
	/* Background color */
	GdkColor color;
	gdk_color_parse("#ffffa0", &color);
	gtk_widget_modify_bg(viewport, GTK_STATE_NORMAL, &color);

	/* Return */
	return popup;
}


void info_popup_free(struct info_popup_t *popup)
{
	free(popup);
}


void info_popup_show(char *text)
{
	struct info_popup_t *popup;

	popup = info_popup_create(text);
	gtk_widget_show_all(popup->window);
	gdk_pointer_grab(gtk_widget_get_window(popup->window), TRUE,
		GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK,
		NULL, NULL, GDK_CURRENT_TIME);
}

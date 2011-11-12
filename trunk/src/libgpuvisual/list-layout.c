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


static gboolean list_layout_popup_button_press_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	int width;
	int height;

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	if (event->x < 0 || event->y < 0 || event->x > width || event->y > height) {
		gdk_pointer_ungrab(GDK_CURRENT_TIME);
		gtk_widget_destroy(widget);
	}
	return FALSE;
}


static gboolean list_layout_popup_motion_event(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	int width;
	int height;

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	if (event->x < -20 || event->y < -20 || event->x > width + 20 || event->y > height + 20) {
		gdk_pointer_ungrab(GDK_CURRENT_TIME);
		gtk_widget_destroy(widget);
	}
	return FALSE;
}


gboolean list_layout_label_enter_notify_event(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	GtkWidget *label;
	GdkColor color;
	GdkWindow *window;

	label = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(widget))->data);
	gdk_color_parse("red", &color);
	gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &color);

	PangoAttrList *attrs;
	attrs = gtk_label_get_attributes(GTK_LABEL(label));
	PangoAttribute *underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
	pango_attr_list_change(attrs, underline_attr);

	GdkCursor *cursor;
	cursor = gdk_cursor_new(GDK_HAND1);
	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, cursor);
	gdk_cursor_unref(cursor);

	return FALSE;
}


gboolean list_layout_label_leave_notify_event(GtkWidget *widget, GdkEventCrossing *event, gpointer data)
{
	GtkWidget *label;
	GdkColor color;
	GdkWindow *window;

	label = GTK_WIDGET(gtk_container_get_children(GTK_CONTAINER(widget))->data);
	gdk_color_parse("black", &color);
	gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &color);
	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, NULL);

	/* Not underlined */
	PangoAttrList *attrs;
	attrs = gtk_label_get_attributes(GTK_LABEL(label));
	PangoAttribute *underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_NONE);
	pango_attr_list_change(attrs, underline_attr);

	return FALSE;
}


static void list_layout_label_clicked_event(GtkWidget *widget, GdkEventButton *event, struct list_layout_item_t *list_layout_item)
{
	struct list_layout_t *list_layout = list_layout_item->list_layout;

	assert(list_layout_item->event_box == widget);
	if (list_layout_item->item && list_layout->item_info_popup)
		(*list_layout->item_info_popup)(list_layout_item->item);
}


struct list_layout_item_t *list_layout_item_create(GtkWidget *event_box, GtkWidget *label,
	void *item, struct list_layout_t *list_layout)
{
	struct list_layout_item_t *list_layout_item;

	list_layout_item = calloc(1, sizeof(struct list_layout_item_t));
	list_layout_item->event_box = event_box;
	list_layout_item->label = label;
	list_layout_item->item = item;
	list_layout_item->list_layout = list_layout;
	return list_layout_item;
}


/* Free all elements from a list containing 'list_layout_item_t'.
 * For each element, the created widget is destroyed, and the list_layout_item_t is freed. */
void list_layout_item_list_clear(struct list_t *list_layout_item_list)
{
	struct list_layout_item_t *list_layout_item;

	while (list_count(list_layout_item_list)) {
		list_layout_item = list_remove_at(list_layout_item_list, 0);
		gtk_widget_destroy(list_layout_item->event_box);
		free(list_layout_item);
	}
}


static void list_layout_popup_destroy_event(GtkWidget *widget, struct list_layout_t *list_layout)
{
	list_layout_item_list_clear(list_layout->list_layout_popup_item_list);
	list_free(list_layout->list_layout_popup_item_list);
}


static void list_layout_popup_show(struct list_layout_t *list_layout)
{
	int i;
	int count;
	char str[MAX_STRING_SIZE];

	/* Create list of 'list_layout_item_t', each containing an event box, a label, and a pointer.  */
	list_layout->list_layout_popup_item_list = list_create();

	/* Create window */
	GtkWidget *window;
	window = gtk_window_new(GTK_WINDOW_POPUP);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
	gtk_widget_set_size_request(window, 200, 250);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK);
	g_signal_connect(G_OBJECT(window), "button-press-event", G_CALLBACK(list_layout_popup_button_press_event), NULL);
	g_signal_connect(G_OBJECT(window), "motion-notify-event", G_CALLBACK(list_layout_popup_motion_event), NULL);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(list_layout_popup_destroy_event), list_layout);

	GtkWidget *scrolled_window;
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(window), scrolled_window);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	GdkColor color;
	gdk_color_parse("#ffffa0", &color);

	GtkWidget *table;
	count = list_count(list_layout->item_list);
	table = gtk_table_new(count, 1, FALSE);
	for (i = 0; i < count; i++)
	{
		void *item = list_get(list_layout->item_list, i);

		/* Create label */
		GtkWidget *label;
		if (list_layout->item_get_name)
			(*list_layout->item_get_name)(item, str, sizeof(str));
		else
			snprintf(str, sizeof(str), "item-%d", i);
		label = gtk_label_new(str);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(12 << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Event box */
		GtkWidget *event_box;
		event_box = gtk_event_box_new();
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
		gtk_container_add(GTK_CONTAINER(event_box), label);

		/* Create list_layout_item */
		struct list_layout_item_t *list_layout_item;
		list_layout_item = calloc(1, sizeof(struct list_layout_item_t));
		list_layout_item->list_layout = list_layout;
		list_layout_item->item = item;
		list_layout_item->event_box = event_box;
		list_layout_item->label = label;
		list_add(list_layout->list_layout_popup_item_list, list_layout_item);

		/* Events for event box */
		gtk_widget_add_events(event_box, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
		g_signal_connect(G_OBJECT(event_box), "enter-notify-event", G_CALLBACK(list_layout_label_enter_notify_event), NULL);
		g_signal_connect(G_OBJECT(event_box), "leave-notify-event", G_CALLBACK(list_layout_label_leave_notify_event), NULL);
		g_signal_connect(G_OBJECT(event_box), "button-press-event", G_CALLBACK(list_layout_label_clicked_event), list_layout_item);

		gtk_table_attach(GTK_TABLE(table), event_box, 0, 1, i, i + 1, GTK_FILL, GTK_FILL, 0, 0);
	}

	GtkWidget *viewport;
	viewport = gtk_viewport_new(NULL, NULL);
	gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);
	gtk_container_add(GTK_CONTAINER(viewport), table);
	gtk_widget_modify_bg(viewport, GTK_STATE_NORMAL, &color);
	gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);

	gtk_widget_show_all(window);
	gdk_pointer_grab(gtk_widget_get_window(window), TRUE, GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK,
		NULL, NULL, GDK_CURRENT_TIME);
}


static gboolean list_layout_label_more_clicked_event(GtkWidget *widget, GdkEventButton *event, struct list_layout_t *list_layout)
{
	list_layout_popup_show(list_layout);
	return FALSE;
}


static void list_layout_destroy_event(GtkWidget *widget, struct list_layout_t *list_layout)
{
	/* Free list_layout_item_list */
	list_layout_item_list_clear(list_layout->list_layout_item_list);
	list_free(list_layout->list_layout_item_list);

	/* Free object */
	free(list_layout);
}


struct list_layout_t *list_layout_new(struct list_t *list,
	int text_size,
	void (*item_get_name)(void *item, char *buf, int size),
	void (*item_info_popup)(void *item))
{
	struct list_layout_t *list_layout;

	list_layout = calloc(1, sizeof(struct list_layout_t));
	list_layout->text_size = text_size;
	list_layout->item_list = list;
	list_layout->layout = gtk_layout_new(NULL, NULL);
	list_layout->item_get_name = item_get_name;
	list_layout->item_info_popup = item_info_popup;
	list_layout->list_layout_item_list = list_create();

	g_signal_connect(G_OBJECT(list_layout->layout), "destroy", G_CALLBACK(list_layout_destroy_event), list_layout);
	return list_layout;
}


void list_layout_refresh(struct list_layout_t *list_layout)
{
	GtkWidget *layout = list_layout->layout;

	int i;
	int x, y;
	int width, height;

	char str[MAX_STRING_SIZE];
	int count;
	struct list_t *item_list = list_layout->item_list;
	GtkStyle *style;

	/* Get widget size */
	width = gtk_widget_get_allocated_width(layout);
	height = gtk_widget_get_allocated_height(layout);

	/* Delete all labels */
	list_layout_item_list_clear(list_layout->list_layout_item_list);

	/* Background color */
	GdkColor color;
	style = gtk_widget_get_style(layout);
	color = style->bg[GTK_STATE_NORMAL];

	/* Fill it with labels */
	x = y = 0;
	count = list_count(item_list);
	for (i = 0; i < count; i++)
	{
		int last;
		char temp_str[MAX_STRING_SIZE];
		void *item = list_get(item_list, i);

		/* Create label */
		GtkWidget *label;
		if (list_layout->item_get_name)
			(*list_layout->item_get_name)(item, temp_str, sizeof(temp_str));
		else
			snprintf(temp_str, sizeof(temp_str), "item-%d", i);
		snprintf(str, sizeof(str), "%s%s", temp_str, i < count - 1 ? "," : "");
		label = gtk_label_new(str);

		/* Set label font attributes */
		PangoAttrList *attrs;
		attrs = pango_attr_list_new();
		PangoAttribute *size_attr = pango_attr_size_new_absolute(list_layout->text_size << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Get position */
		GtkRequisition req;
		gtk_widget_size_request(label, &req);
		last = FALSE;
		if (x > 0 && x + req.width >= width) {
			x = 0;
			y += req.height;
			if (y + 2 * req.height >= height && i < count - 1) {
				snprintf(str, sizeof(str), "+ %d more", count - i);
				gtk_label_set_text(GTK_LABEL(label), str);
				gtk_widget_size_request(label, &req);
				last = TRUE;
			}
		}

		/* Create event box */
		GtkWidget *event_box;
		event_box = gtk_event_box_new();
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
		gtk_container_add(GTK_CONTAINER(event_box), label);
		gtk_widget_add_events(event_box, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);

		/* Create list_layout_item */
		struct list_layout_item_t *list_layout_item;
		list_layout_item = calloc(1, sizeof(struct list_layout_item_t));
		list_layout_item->list_layout = list_layout;
		list_layout_item->item = item;
		list_layout_item->event_box = event_box;
		list_layout_item->label = label;
		list_add(list_layout->list_layout_item_list, list_layout_item);

		/* Events for event box */
		g_signal_connect(G_OBJECT(event_box), "enter-notify-event", G_CALLBACK(list_layout_label_enter_notify_event), NULL);
		g_signal_connect(G_OBJECT(event_box), "leave-notify-event", G_CALLBACK(list_layout_label_leave_notify_event), NULL);
		if (last) {
			g_signal_connect(G_OBJECT(event_box), "button-press-event",
				G_CALLBACK(list_layout_label_more_clicked_event), list_layout);
		} else {
			g_signal_connect(G_OBJECT(event_box), "button-press-event",
				G_CALLBACK(list_layout_label_clicked_event), list_layout_item);
		}


		/* Insert event box in layout */
		gtk_layout_put(GTK_LAYOUT(layout), event_box, x, y);
		gtk_widget_show_all(event_box);

		/* Advance */
		x += req.width + 5;
		if (last)
			break;
	}
}


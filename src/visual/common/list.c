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
#include <lib/util/list.h>

#include "list.h"
#include "popup.h"


struct vi_list_item_t
{
	/* Associated GTK widgets */
	GtkWidget *event_box;
	GtkWidget *label;
	GdkColor label_color;

	/* Visual list where it belongs */
	struct vi_list_t *list;

	/* Associated data element from 'list->elem_list' */
	void *elem;
};


struct vi_list_popup_t
{
	/* GTK widgets */
	GtkWidget *window;
	GtkWidget *image_close;

	/* List of 'vi_list_item_t' elements */
	struct list_t *item_list;

	/* Visual list that triggered the pop-up */
	struct vi_list_t *list;
};

struct vi_list_popup_t *vi_list_popup_create(struct vi_list_t *list);
void vi_list_popup_free(struct vi_list_popup_t *popup);

void vi_list_popup_show(struct vi_list_t *list);


struct vi_list_t
{
	/* Widget showing list */
	GtkWidget *widget;

	/* Dimensions after last refresh */
	int width;
	int height;

	/* List of elements in the list */
	struct list_t *elem_list;

	/* List of elements 'vi_list_item_t' currently displayed in the list. This
	 * list will synchronize with 'elem_list' upon a call to 'vi_list_refresh'. */
	struct list_t *item_list;

	/* Call-back functions to get element names and descriptions */
	void (*get_elem_name)(void *elem, char *buf, int size);
	void (*get_elem_desc)(void *elem, char *buf, int size);

	/* Properties */
	char *title;
	int text_size;
};





/*
 * Visual List Item
 */


static struct vi_list_item_t *vi_list_item_create(void)
{
	struct vi_list_item_t *item;

	/* Return */
	item = xcalloc(1, sizeof(struct vi_list_item_t));
	return item;
}


static void vi_list_item_free(struct vi_list_item_t *item)
{
	free(item);
}


static gboolean vi_list_item_enter_notify_event(GtkWidget *widget,
	GdkEventCrossing *event, struct vi_list_item_t *item)
{
	GdkColor color;

	PangoAttrList *attrs;
	PangoAttribute *underline_attr;

	GdkWindow *window;
	GdkCursor *cursor;

	GtkStyle *style;

	style = gtk_widget_get_style(item->label);
	item->label_color = style->fg[GTK_STATE_NORMAL];

	gdk_color_parse("red", &color);
	gtk_widget_modify_fg(item->label, GTK_STATE_NORMAL, &color);

	attrs = gtk_label_get_attributes(GTK_LABEL(item->label));
	underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
	pango_attr_list_change(attrs, underline_attr);

	cursor = gdk_cursor_new(GDK_HAND1);
	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, cursor);
	g_object_unref(cursor);

	return FALSE;
}


static gboolean vi_list_item_leave_notify_event(GtkWidget *widget,
	GdkEventCrossing *event, struct vi_list_item_t *item)
{
	PangoAttrList *attrs;
	PangoAttribute *underline_attr;

	GdkWindow *window;

	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, NULL);

	attrs = gtk_label_get_attributes(GTK_LABEL(item->label));
	underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_NONE);
	pango_attr_list_change(attrs, underline_attr);
	gtk_widget_modify_fg(item->label, GTK_STATE_NORMAL, &item->label_color);

	return FALSE;
}


static gboolean vi_list_item_button_press_event(GtkWidget *widget,
	GdkEventButton *event, struct vi_list_item_t *item)
{
	char text[MAX_LONG_STRING_SIZE];
	struct vi_list_t *list = item->list;

	/* Get item description */
	if (list->get_elem_desc)
		(*list->get_elem_desc)(item->elem, text, sizeof text);
	else
		snprintf(text, sizeof text, "No information.");

	/* Show description pop-up */
	vi_popup_show(text);
	return FALSE;
}


static gboolean vi_list_item_more_press_event(GtkWidget *widget,
	GdkEventButton *event, struct vi_list_item_t *item)
{
	vi_list_popup_show(item->list);
	return FALSE;
}




/*
 * Visual List Popup
 */


/* Path for 'close' icon */
char vi_list_image_close_path[MAX_PATH_SIZE];
char vi_list_image_close_sel_path[MAX_PATH_SIZE];


static gboolean vi_list_popup_image_close_enter_notify_event(GtkWidget *widget,
	GdkEvent *event, struct vi_list_popup_t *popup)
{
	gtk_image_set_from_file(GTK_IMAGE(popup->image_close), vi_list_image_close_sel_path);
	return FALSE;
}


static gboolean vi_list_popup_image_close_leave_notify_event(GtkWidget *widget,
	GdkEvent *event, struct vi_list_popup_t *popup)
{
	gtk_image_set_from_file(GTK_IMAGE(popup->image_close), vi_list_image_close_path);
	return FALSE;
}


static void vi_list_popup_image_close_clicked_event(GtkWidget *widget,
	GdkEventButton *event, struct vi_list_popup_t *popup)
{
	gtk_widget_destroy(popup->window);
}


static void vi_list_popup_destroy_event(GtkWidget *widget, struct vi_list_popup_t *popup)
{
	/* Free item list */
	while (popup->item_list->count)
		vi_list_item_free(list_remove_at(popup->item_list, 0));
	list_free(popup->item_list);

	/* Free pop-up */
	vi_list_popup_free(popup);
}


struct vi_list_popup_t *vi_list_popup_create(struct vi_list_t *list)
{
	struct vi_list_popup_t *popup;

	/* Initialize */
	popup = xcalloc(1, sizeof(struct vi_list_popup_t));
	popup->list = list;

	int i;
	int count;

	/* Create list of 'vi_list_item_t'  */
	popup->item_list = list_create();

	/* Create main window */
	GtkWidget *window;
	window = gtk_window_new(GTK_WINDOW_POPUP);
	popup->window = window;
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
	gtk_widget_set_size_request(window, 200, 250);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(vi_list_popup_destroy_event), popup);

	/* Close button */
	GtkWidget *image_close = gtk_image_new_from_file(vi_list_image_close_path);
	GtkWidget *event_box_close = gtk_event_box_new();
	popup->image_close = image_close;
	gtk_container_add(GTK_CONTAINER(event_box_close), image_close);
	gtk_widget_add_events(event_box_close, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
	g_signal_connect(G_OBJECT(event_box_close), "enter-notify-event",
		G_CALLBACK(vi_list_popup_image_close_enter_notify_event), popup);
	g_signal_connect(G_OBJECT(event_box_close), "leave-notify-event",
		G_CALLBACK(vi_list_popup_image_close_leave_notify_event), popup);
	g_signal_connect(G_OBJECT(event_box_close), "button-press-event",
		G_CALLBACK(vi_list_popup_image_close_clicked_event), popup);

	/* Title */
	GtkWidget *title_label = gtk_label_new(list->title);
	GtkWidget *title_event_box = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(title_event_box), title_label);

	/* Separator below title */
	GtkWidget *hsep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

	/* Scrolled window */
	GtkWidget *scrolled_window;
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	/* Main table */
	GtkWidget *main_table;
	main_table = gtk_table_new(3, 2, FALSE);
	gtk_table_attach(GTK_TABLE(main_table), title_event_box, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(main_table), event_box_close, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(main_table), hsep, 0, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach_defaults(GTK_TABLE(main_table), scrolled_window, 0, 2, 2, 3);
	gtk_container_add(GTK_CONTAINER(window), main_table);

	GdkColor color;
	gdk_color_parse("#ffffa0", &color);

	GtkWidget *table;
	count = list->elem_list->count;
	table = gtk_table_new(count, 1, FALSE);
	for (i = 0; i < count; i++)
	{
		void *elem;
		char str[MAX_STRING_SIZE];
		struct vi_list_item_t *item;

		/* Get element */
		elem = list_get(list->elem_list, i);

		/* Create label */
		GtkWidget *label;
		if (list->get_elem_name)
			(*list->get_elem_name)(elem, str, sizeof str);
		else
			snprintf(str, sizeof str, "item-%d", i);
		label = gtk_label_new(str);
		gtk_misc_set_alignment(GTK_MISC(label), 0, 0);
		gtk_label_set_use_markup(GTK_LABEL(label), TRUE);

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
		item = vi_list_item_create();
		item->list = list;
		item->event_box = event_box;
		item->label = label;
		item->elem = elem;
		list_add(list->item_list, item);

		/* Events for event box */
		gtk_widget_add_events(event_box, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
		g_signal_connect(G_OBJECT(event_box), "enter-notify-event",
			G_CALLBACK(vi_list_item_enter_notify_event), item);
		g_signal_connect(G_OBJECT(event_box), "leave-notify-event",
			G_CALLBACK(vi_list_item_leave_notify_event), item);
		g_signal_connect(G_OBJECT(event_box), "button-press-event",
			G_CALLBACK(vi_list_item_button_press_event), item);

		gtk_table_attach(GTK_TABLE(table), event_box, 0, 1, i, i + 1, GTK_FILL, GTK_FILL, 0, 0);
	}

	GtkWidget *viewport;
	viewport = gtk_viewport_new(NULL, NULL);
	gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);
	gtk_container_add(GTK_CONTAINER(viewport), table);
	gtk_widget_modify_bg(viewport, GTK_STATE_NORMAL, &color);
	gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);

	/* Return */
	return popup;
}


void vi_list_popup_free(struct vi_list_popup_t *popup)
{
	free(popup);
}


void vi_list_popup_show(struct vi_list_t *list)
{
	struct vi_list_popup_t *popup;

	popup = vi_list_popup_create(list);
	gtk_widget_show_all(popup->window);
}




/*
 * Visual List
 */


static void vi_list_size_allocate_event(GtkWidget *widget, GdkRectangle *allocation, struct vi_list_t *list)
{
	if (allocation->width != list->width || allocation->height != list->height)
		vi_list_refresh(list);
}


struct vi_list_t *vi_list_create(char *title, int width, int height,
	vi_list_get_elem_name_func_t get_elem_name,
	vi_list_get_elem_name_func_t get_elem_desc)
{
	struct vi_list_t *list;

	/* Title */
	list = xcalloc(1, sizeof(struct vi_list_t));
	list->title = xstrdup(title);

	/* Initialize */
	list->elem_list = list_create();
	list->item_list = list_create();
	list->text_size = 12;
	list->get_elem_name = get_elem_name;
	list->get_elem_desc = get_elem_desc;

	/* GTK widget */
	list->widget = gtk_layout_new(NULL, NULL);
	g_signal_connect(G_OBJECT(list->widget), "size_allocate", G_CALLBACK(vi_list_size_allocate_event), list);
	gtk_widget_set_size_request(list->widget, width, height);

	/* Return */
	return list;
}


void vi_list_free(struct vi_list_t *list)
{
	/* Item list */
	while (list->item_list->count)
		vi_list_item_free(list_remove_at(list->item_list, 0));
	list_free(list->item_list);

	/* List of elements */
	list_free(list->elem_list);

	/* Object */
	free(list->title);
	free(list);
}


int vi_list_count(struct vi_list_t *list)
{
	return list_count(list->elem_list);
}


void vi_list_add(struct vi_list_t *list, void *elem)
{
	return list_add(list->elem_list, elem);
}


void *vi_list_get(struct vi_list_t *list, int index)
{
	return list_get(list->elem_list, index);
}


void *vi_list_remove_at(struct vi_list_t *list, int index)
{
	return list_remove_at(list->elem_list, index);
}


void vi_list_refresh(struct vi_list_t *list)
{
	int width;
	int height;

	int x;
	int y;

	int count;
	int i;

	GtkStyle *style;
	GList *child;

	/* Clear current item list and empty layout */
	while (list->item_list->count)
		vi_list_item_free(list_remove_at(list->item_list, 0));
	while ((child = gtk_container_get_children(GTK_CONTAINER(list->widget))))
		gtk_container_remove(GTK_CONTAINER(list->widget), child->data);

	/* Get 'list' widget size */
	width = gtk_widget_get_allocated_width(list->widget);
	height = gtk_widget_get_allocated_height(list->widget);
	list->width = width;
	list->height = height;

	/* Background color */
	GdkColor color;
	style = gtk_widget_get_style(list->widget);
	color = style->bg[GTK_STATE_NORMAL];

	/* Fill it with labels */
	x = 0;
	y = 0;
	count = list->elem_list->count;
	for (i = 0; i < count; i++)
	{
		int last;

		struct vi_list_item_t *item;
		void *elem;

		char str1[MAX_STRING_SIZE];
		char str2[MAX_STRING_SIZE];
		char *comma;

		GtkWidget *label;
		GtkWidget *event_box;

		PangoAttrList *attrs;
		PangoAttribute *size_attr;

		GtkRequisition req;

		/* Create list item */
		item = vi_list_item_create();

		/* Get current element */
		elem = list_get(list->elem_list, i);

		/* Create label */
		comma = i < count - 1 ? "," : "";
		if (list->get_elem_name)
			(*list->get_elem_name)(elem, str1, sizeof str1);
		else
			snprintf(str1, sizeof str1, "item-%d", i);
		snprintf(str2, sizeof str2, "%s%s", str1, comma);
		label = gtk_label_new(str2);
		gtk_label_set_use_markup(GTK_LABEL(label), TRUE);

		/* Set label font attributes */
		attrs = pango_attr_list_new();
		size_attr = pango_attr_size_new_absolute(list->text_size << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Get position */
		gtk_widget_get_preferred_size(label, &req, NULL);
		last = 0;
		if (x > 0 && x + req.width >= width)
		{
			x = 0;
			y += req.height;
			if (y + 2 * req.height >= height && i < count - 1)
			{
				snprintf(str1, sizeof str1, "+ %d more", count - i);
				gtk_label_set_text(GTK_LABEL(label), str1);
				gtk_widget_get_preferred_size(label, &req, NULL);
				last = 1;
			}
		}

		/* Create event box */
		event_box = gtk_event_box_new();
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
		gtk_container_add(GTK_CONTAINER(event_box), label);
		gtk_widget_add_events(event_box, GDK_ENTER_NOTIFY_MASK |
			GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);

		/* Events for event box */
		g_signal_connect(G_OBJECT(event_box), "enter-notify-event",
			G_CALLBACK(vi_list_item_enter_notify_event), item);
		g_signal_connect(G_OBJECT(event_box), "leave-notify-event",
			G_CALLBACK(vi_list_item_leave_notify_event), item);
		if (last)
			g_signal_connect(G_OBJECT(event_box), "button-press-event",
				G_CALLBACK(vi_list_item_more_press_event), item);
		else
			g_signal_connect(G_OBJECT(event_box), "button-press-event",
				G_CALLBACK(vi_list_item_button_press_event), item);

		/* Insert event box in 'list' layout */
		gtk_layout_put(GTK_LAYOUT(list->widget), event_box, x, y);

		/* Initialize item */
		item->list = list;
		item->elem = elem;
		item->event_box = event_box;
		item->label = label;
		list_add(list->item_list, item);

		/* Advance */
		x += req.width + 5;
		if (last)
			break;
	}

	/* Show all new widgets */
	gtk_widget_show_all(list->widget);
	gtk_container_check_resize(GTK_CONTAINER(list->widget));
}


GtkWidget *vi_list_get_widget(struct vi_list_t *list)
{
	return list->widget;
}

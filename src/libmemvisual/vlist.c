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


/*
 * Visual List Item
 */


static struct vlist_item_t *vlist_item_create(void)
{
	struct vlist_item_t *item;
	/* Allocate */
	item = calloc(1, sizeof(struct vlist_item_t));
	if (!item)
		fatal("%s: out of memory", __FUNCTION__);

	/* Return */
	return item;
}


static void vlist_item_free(struct vlist_item_t *item)
{
	free(item);
}


static gboolean vlist_item_enter_notify_event(GtkWidget *widget,
	GdkEventCrossing *event, struct vlist_item_t *item)
{
	GdkColor color;

	PangoAttrList *attrs;
	PangoAttribute *underline_attr;

	GdkWindow *window;
	GdkCursor *cursor;

	gdk_color_parse("red", &color);
	gtk_widget_modify_fg(item->label, GTK_STATE_NORMAL, &color);

	attrs = gtk_label_get_attributes(GTK_LABEL(item->label));
	underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
	pango_attr_list_change(attrs, underline_attr);

	cursor = gdk_cursor_new(GDK_HAND1);
	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, cursor);
	gdk_cursor_unref(cursor);

	return FALSE;
}


static gboolean vlist_item_leave_notify_event(GtkWidget *widget,
	GdkEventCrossing *event, struct vlist_item_t *item)
{
	PangoAttrList *attrs;
	PangoAttribute *underline_attr;

	GdkColor color;
	GdkWindow *window;

	gdk_color_parse("black", &color);
	gtk_widget_modify_fg(item->label, GTK_STATE_NORMAL, &color);

	window = gtk_widget_get_parent_window(widget);
	gdk_window_set_cursor(window, NULL);

	attrs = gtk_label_get_attributes(GTK_LABEL(item->label));
	underline_attr = pango_attr_underline_new(PANGO_UNDERLINE_NONE);
	pango_attr_list_change(attrs, underline_attr);

	return FALSE;
}


static void vlist_item_destroy_event(GtkWidget *widget, struct vlist_item_t *item)
{
	item->event_box = NULL;
	item->label = NULL;
}


static gboolean vlist_item_button_press_event(GtkWidget *widget,
	GdkEventButton *event, struct vlist_item_t *item)
{
	printf("Button pressed clicked\n");
	return FALSE;
}


static gboolean vlist_item_more_press_event(GtkWidget *widget,
	GdkEventButton *event, struct vlist_item_t *item)
{
	struct vlist_popup_t *popup;

	popup = vlist_popup_create(item->vlist);
	gtk_widget_show_now(popup->window);
	vlist_popup_free(popup);
	return FALSE;
}




/*
 * Visual List Popup
 */

struct vlist_popup_t *vlist_popup_create(struct vlist_t *vlist)
{
	struct vlist_popup_t *popup;

	/* Allocate */
	popup = calloc(1, sizeof(struct vlist_popup_t));
	if (!popup)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	popup->vlist = vlist;

	int i;
	int count;

	/* Create list of 'vlist_item_t'  */
	popup->item_list = list_create();

	/* Create main window */
	GtkWidget *window;
	window = gtk_window_new(GTK_WINDOW_POPUP);
	popup->window = window;
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
	gtk_widget_set_size_request(window, 200, 250);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);
	//gtk_window_set_transient_for(GTK_WINDOW(popup->window), GTK_WINDOW(list_layout->parent_window)); FIXME
	//g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(list_layout_popup_destroy_event), list_layout); FIXME

	/* Close button */
	extern char *img_close_path;  /* FIXME */
	GtkWidget *img_close = gtk_image_new_from_file(img_close_path);
	GtkWidget *evbox_close = gtk_event_box_new();
	gtk_container_add(GTK_CONTAINER(evbox_close), img_close);
	gtk_widget_add_events(evbox_close, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
	/*g_signal_connect(G_OBJECT(evbox_close), "enter-notify-event",
		G_CALLBACK(vlist_popup_img_close_enter_notify_event), popup);
	g_signal_connect(G_OBJECT(evbox_close), "leave-notify-event",
		G_CALLBACK(vlist_popup_img_close_leave_notify_event), popup);
	g_signal_connect(G_OBJECT(evbox_close), "button-press-event",
		G_CALLBACK(vlist_popup_img_close_clicked_event), popup);*/
	popup->img_close = img_close;

	/* Title and separator */
	GtkWidget *title_label = gtk_label_new(vlist->title);
	GtkWidget *hsep = gtk_hseparator_new();

	/* Scrolled window */
	GtkWidget *scrolled_window;
	scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	/* Main table */
	GtkWidget *main_table;
	main_table = gtk_table_new(3, 2, FALSE);
	gtk_table_attach(GTK_TABLE(main_table), title_label, 0, 1, 0, 1, GTK_FILL | GTK_EXPAND, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(main_table), evbox_close, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach(GTK_TABLE(main_table), hsep, 0, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	gtk_table_attach_defaults(GTK_TABLE(main_table), scrolled_window, 0, 2, 2, 3);
	gtk_container_add(GTK_CONTAINER(window), main_table);

	GdkColor color;
	gdk_color_parse("#ffffa0", &color);

	GtkWidget *table;
	count = vlist->elem_list->count;
	table = gtk_table_new(count, 1, FALSE);
	for (i = 0; i < count; i++)
	{
		void *elem;
		char str[MAX_STRING_SIZE];
		struct vlist_item_t *item;

		/* Get element */
		elem = list_get(vlist->elem_list, i);

		/* Create label */
		GtkWidget *label;
		if (vlist->get_elem_name)
			(*vlist->get_elem_name)(elem, str, sizeof str);
		else
			snprintf(str, sizeof str, "item-%d", i);
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
		item = vlist_item_create();
		item->vlist = vlist;
		item->event_box = event_box;
		item->label = label;
		item->elem = elem;
		list_add(vlist->item_list, item);

		/* Events for event box */
		gtk_widget_add_events(event_box, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK);
		g_signal_connect(G_OBJECT(event_box), "enter-notify-event",
			G_CALLBACK(vlist_item_enter_notify_event), item);
		g_signal_connect(G_OBJECT(event_box), "leave-notify-event",
			G_CALLBACK(vlist_item_leave_notify_event), item);
		g_signal_connect(G_OBJECT(event_box), "button-press-event",
			G_CALLBACK(vlist_item_button_press_event), item);

		gtk_table_attach(GTK_TABLE(table), event_box, 0, 1, i, i + 1, GTK_FILL, GTK_FILL, 0, 0);
	}

	GtkWidget *viewport;
	viewport = gtk_viewport_new(NULL, NULL);
	gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);
	gtk_container_add(GTK_CONTAINER(viewport), table);
	gtk_widget_modify_bg(viewport, GTK_STATE_NORMAL, &color);
	gtk_container_add(GTK_CONTAINER(scrolled_window), viewport);

	gtk_widget_show_all(window);

	/* Return */
	return popup;
}


void vlist_popup_free(struct vlist_popup_t *popup)
{
	free(popup);
}




/*
 * Visual List
 */


static void vlist_item_list_clear(struct vlist_t *vlist)
{
	struct vlist_item_t *item;

	while (vlist->item_list->count)
	{
		item = list_remove_at(vlist->item_list, 0);

		/* Destroy widget if it still exists */
		if (item->event_box)
			gtk_widget_destroy(item->event_box);

		/* Free item */
		vlist_item_free(item);
	}
}


static void vlist_size_allocate_event(GtkWidget *widget, GdkRectangle *allocation, struct vlist_t *vlist)
{
	if (allocation->width != vlist->width || allocation->height != vlist->height)
		vlist_refresh(vlist);
}


struct vlist_t *vlist_create(char *title, int width, int height,
	void (*get_elem_name)(void *elem, char *buf, int size),
	void (*get_elem_desc)(void *elem, char *buf, int size))
{
	struct vlist_t *vlist;

	/* Allocate */
	vlist = calloc(1, sizeof(struct vlist_t));
	if (!vlist)
		fatal("%s: out of memory", __FUNCTION__);

	/* Title */
	vlist->title = strdup(title);
	if (!vlist->title)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	vlist->elem_list = list_create();
	vlist->item_list = list_create();
	vlist->text_size = 12;
	vlist->get_elem_name = get_elem_name;
	vlist->get_elem_desc = get_elem_desc;

	/* GTK widget */
	vlist->widget = gtk_layout_new(NULL, NULL);
	g_signal_connect(G_OBJECT(vlist->widget), "size_allocate", G_CALLBACK(vlist_size_allocate_event), vlist);
	gtk_widget_set_size_request(vlist->widget, width, height);

	/* Return */
	return vlist;
}


void vlist_free(struct vlist_t *vlist)
{
	/* Item list */
	vlist_item_list_clear(vlist);
	list_free(vlist->item_list);

	/* List of elements */
	list_free(vlist->elem_list);

	/* Object */
	free(vlist->title);
	free(vlist);
}


void vlist_refresh(struct vlist_t *vlist)
{
	int width;
	int height;

	int x;
	int y;

	int count;
	int i;

	GtkStyle *style;

	/* Clear current list */
	vlist_item_list_clear(vlist);

	/* Get 'vlist' widget size */
	width = gtk_widget_get_allocated_width(vlist->widget);
	height = gtk_widget_get_allocated_height(vlist->widget);
	vlist->width = width;
	vlist->height = height;

	/* Background color */
	GdkColor color;
	style = gtk_widget_get_style(vlist->widget);
	color = style->bg[GTK_STATE_NORMAL];

	/* Fill it with labels */
	x = 0;
	y = 0;
	count = vlist->elem_list->count;
	for (i = 0; i < count; i++)
	{
		int last;

		struct vlist_item_t *item;
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
		item = vlist_item_create();

		/* Get current element */
		elem = list_get(vlist->elem_list, i);

		/* Create label */
		comma = i < count - 1 ? "," : "";
		if (vlist->get_elem_name)
			(*vlist->get_elem_name)(elem, str1, sizeof str1);
		else
			snprintf(str1, sizeof str1, "item-%d", i);
		snprintf(str2, sizeof str2, "%s%s", str1, comma);
		label = gtk_label_new(str2);

		/* Set label font attributes */
		attrs = pango_attr_list_new();
		size_attr = pango_attr_size_new_absolute(vlist->text_size << 10);
		pango_attr_list_insert(attrs, size_attr);
		gtk_label_set_attributes(GTK_LABEL(label), attrs);

		/* Get position */
		gtk_widget_size_request(label, &req);
		last = 0;
		if (x > 0 && x + req.width >= width)
		{
			x = 0;
			y += req.height;
			if (y + 2 * req.height >= height && i < count - 1)
			{
				snprintf(str1, sizeof str1, "+ %d more", count - i);
				gtk_label_set_text(GTK_LABEL(label), str1);
				gtk_widget_size_request(label, &req);
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
			G_CALLBACK(vlist_item_enter_notify_event), item);
		g_signal_connect(G_OBJECT(event_box), "leave-notify-event",
			G_CALLBACK(vlist_item_leave_notify_event), item);
		g_signal_connect(G_OBJECT(event_box), "destroy",
			G_CALLBACK(vlist_item_destroy_event), item);
		if (last)
			g_signal_connect(G_OBJECT(event_box), "button-press-event",
				G_CALLBACK(vlist_item_more_press_event), item);
		else
			g_signal_connect(G_OBJECT(event_box), "button-press-event",
				G_CALLBACK(vlist_item_button_press_event), item);

		/* Insert event box in 'vlist' layout */
		gtk_layout_put(GTK_LAYOUT(vlist->widget), event_box, x, y);

		/* Initialize item */
		item->vlist = vlist;
		item->elem = elem;
		item->event_box = event_box;
		item->label = label;
		list_add(vlist->item_list, item);

		/* Advance */
		x += req.width + 5;
		if (last)
			break;
	}

	/* Show all new widgets */
	gtk_widget_show_all(vlist->widget);
	gtk_container_check_resize(GTK_CONTAINER(vlist->widget));
}

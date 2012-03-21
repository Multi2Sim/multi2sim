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


gboolean vlist_item_enter_notify_event(GtkWidget *widget, GdkEventCrossing *event, struct vlist_item_t *item)
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


gboolean vlist_item_leave_notify_event(GtkWidget *widget, GdkEventCrossing *event, struct vlist_item_t *item)
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




/*
 * Visual List
 */


static void vlist_item_list_clear(struct vlist_t *vlist)
{
	struct vlist_item_t *item;

/*	while (vlist->item_list->count)
	{
		item = list_remove_at(vlist->item_list, 0);
		gtk_widget_destroy(item->event_box);
		vlist_item_free(item);
	}
*/
	int i;
	for (i = 0; i < 4 && vlist->item_list->count; i++)
	{
		item = list_remove_at(vlist->item_list, 0);
		gtk_container_remove(GTK_CONTAINER(vlist->widget), item->event_box);
		vlist_item_free(item);
	}
}

struct vlist_t *vlist_create(void)
{
	struct vlist_t *vlist;

	/* Allocate */
	vlist = calloc(1, sizeof(struct vlist_t));
	if (!vlist)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	vlist->elem_list = list_create();
	vlist->item_list = list_create();
	vlist->text_size = 12;

	/* GTK widget */
	vlist->widget = gtk_layout_new(NULL, NULL);
	gtk_widget_set_size_request(vlist->widget, 200, 20);  // FIXME

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

	/* Get 'vlist' widget size */
	width = gtk_widget_get_allocated_width(vlist->widget);
	height = gtk_widget_get_allocated_height(vlist->widget);

	/* Clear current contents */
	{
		GList *child;
		while ((child = gtk_container_get_children(GTK_CONTAINER(vlist->widget))))
			gtk_container_remove(GTK_CONTAINER(vlist->widget), child->data);
	}

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

		char str[MAX_STRING_SIZE];
		char *comma;

		GtkWidget *label;
		GtkWidget *event_box;

		PangoAttrList *attrs;
		PangoAttribute *size_attr;

		GtkRequisition req;

		/* Get current element */
		elem = list_get(vlist->elem_list, i);

		/* Create label */
		comma = i < count - 1 ? "," : "";
		/*if (list_layout->item_get_name)
			(*list_layout->item_get_name)(item, temp_str, sizeof temp_str);
		else*/
		snprintf(str, sizeof str, "item-%d%s", i, comma);
		label = gtk_label_new(str);

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
				snprintf(str, sizeof str, "+ %d more", count - i);
				gtk_label_set_text(GTK_LABEL(label), str);
				gtk_widget_size_request(label, &req);
				last = 1;
			}
		}

		/* Create event box */
		event_box = gtk_event_box_new();
		gtk_widget_modify_bg(event_box, GTK_STATE_NORMAL, &color);
		gtk_container_add(GTK_CONTAINER(event_box), label);
		gtk_widget_add_events(event_box, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);
		g_signal_connect(G_OBJECT(event_box), "enter-notify-event", G_CALLBACK(vlist_item_enter_notify_event), item);
		g_signal_connect(G_OBJECT(event_box), "leave-notify-event", G_CALLBACK(vlist_item_leave_notify_event), item);

		/* Events for event box */
		/*if (last) {
			g_signal_connect(G_OBJECT(ebox), "button-press-event",
				G_CALLBACK(list_layout_label_more_clicked_event), list_layout);
		} else {
			g_signal_connect(G_OBJECT(ebox), "button-press-event",
				G_CALLBACK(list_layout_label_clicked_event), list_layout_item);
		}*/


		/* Insert event box in 'vlist' layout */
		{
			///////////////
			static int my_y;

			my_y += 10;
			y = my_y;
			x = 0;
		}
		gtk_layout_put(GTK_LAYOUT(vlist->widget), event_box, x, y);

		/* Create list item */
		item = vlist_item_create();
		item->vlist = vlist;
		item->elem = elem;
		item->event_box = event_box;
		item->label = label;
		item->x = x;
		item->y = y;
		list_add(vlist->item_list, item);

		/* Advance */
		x += req.width + 5;
		if (last)
			break;
	}

	/* Show all new widgets */
	gtk_widget_show_all(vlist->widget);


	{
		int i;
		printf("------------\n");
		for (i = 0; i < vlist->item_list->count; i++)
		{
			struct vlist_item_t *item;
			item = list_get(vlist->item_list, i);
			printf("item-%d: x=%d, y=%d\n", i, item->x, item->y);
		}
	}
}

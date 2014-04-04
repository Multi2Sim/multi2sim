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
#include <gtk/gtk.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>
#include <visual/memory/mod.h>

#include "link.h"
#include "net.h"
#include "net-widget.h"
#include "node.h"
#include "node-widget.h"

#define VI_NODE_BOARD_PADDING           10
#define VI_NODE_BOARD_WIDTH             70
#define VI_NODE_BOARD_HEIGHT            70
# define M_PI           3.14159265358979323846  /* pi */


static void vi_net_widget_destroy(GtkWidget *widget, struct vi_net_widget_t *net_widget);

GtkWidget *vi_net_widget_get_widget(struct vi_net_widget_t *net_widget)
{
        return net_widget->widget;
}

static void vi_net_widget_destroy(GtkWidget *widget, struct vi_net_widget_t *net_widget)
{
        vi_net_widget_free(net_widget);
}


void vi_net_widget_free(struct vi_net_widget_t *net_widget)
{

        /* Free widget */
        list_free(net_widget->link_board_list);
        list_free(net_widget->node_board_list);
        free(net_widget);
}

/* Creating Network Widget */
static struct vi_node_board_t  *vi_node_board_create                (struct vi_net_node_t *node);
static gboolean 	        vi_node_board_toggle_button_toggled (GtkWidget *widget, struct vi_node_board_t *board);
static void 		        vi_node_board_destroy               (GtkWidget *widget, struct vi_node_board_t *board);

/* We named them board since it is easy */
static struct vi_link_board_t  *vi_link_board_create                (struct vi_net_sub_link_t *subLink);
static void                     vi_link_board_destroy               (GtkWidget *widget, struct vi_link_board_t *subLink);
static struct vi_node_window_t *vi_node_window_create               (struct vi_net_node_t * node, GtkWidget *parent_toggle_button);
static void                     vi_node_window_free                 (struct vi_node_window_t *node_window);
static void                     vi_node_window_destroy              (GtkWidget *widget, struct vi_node_window_t *node_window);
static gboolean                 vi_node_window_delete               (GtkWidget *widget, GdkEvent *event, struct vi_node_window_t *node_window);
static GtkWidget               *vi_node_window_get_widget           (struct vi_node_window_t *node_window);

static gboolean vi_link_board_draw (GtkWidget *widget, GdkEventConfigure *event, struct vi_link_board_t *board);

struct vi_net_widget_t *vi_net_widget_create(struct vi_net_t *net)
{
        struct vi_net_widget_t *panel;

        int layout_width;
        int layout_height;

        /* Initialize */
        panel = xcalloc(1, sizeof(struct vi_net_widget_t));
        panel->node_board_list = list_create();
        panel->link_board_list = list_create();

        /* Layout */
        GtkWidget *layout;
        GdkColor color;
        layout = gtk_layout_new(NULL, NULL);
        gdk_color_parse("white", &color);
        gtk_widget_modify_bg(layout, GTK_STATE_NORMAL, &color);

        /* Frame */
        GtkWidget *frame;
        frame = gtk_frame_new("Network");

        /* Scrolled window */
        GtkWidget *scrolled_window;
        scrolled_window = gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), layout);
        gtk_widget_set_size_request(scrolled_window, VI_NODE_BOARD_WIDTH * 3 / 2, VI_NODE_BOARD_HEIGHT * 3 / 2);
        gtk_container_add(GTK_CONTAINER(frame), scrolled_window);

        layout_width = VI_NODE_BOARD_WIDTH;
        layout_height = VI_NODE_BOARD_HEIGHT;

        /* Insert Links */
        struct vi_net_link_t *link;
        char *link_name;

        HASH_TABLE_FOR_EACH(net->link_table, link_name, link)
        {
                for (int i = 0; i < list_count(link->sublink_list); i++)
                {
                        struct vi_net_sub_link_t *subLink;
                        subLink = list_get(link->sublink_list , i);

                        /* updating X and Ys to Center */
                        subLink->src_x = (VI_NODE_BOARD_WIDTH/2 +
                                        ( 2 * subLink->src_x) * (VI_NODE_BOARD_PADDING + VI_NODE_BOARD_WIDTH));
                        subLink->src_y = (VI_NODE_BOARD_HEIGHT/2 +
                                        ( 2 * subLink->src_y) * (VI_NODE_BOARD_PADDING + VI_NODE_BOARD_HEIGHT));

                        subLink->dst_x = (VI_NODE_BOARD_WIDTH/2 +
                                        ( 2 * subLink->dst_x) * (VI_NODE_BOARD_PADDING + VI_NODE_BOARD_WIDTH));
                        subLink->dst_y = (VI_NODE_BOARD_HEIGHT/2 +
                                        ( 2 * subLink->dst_y) * (VI_NODE_BOARD_PADDING + VI_NODE_BOARD_HEIGHT));

                        struct vi_link_board_t *drwSubLink;
                        drwSubLink = vi_link_board_create(subLink);
                        list_add(panel->link_board_list, drwSubLink);
                        gtk_layout_put(GTK_LAYOUT(layout), drwSubLink->widget, 0,0);
              }
        }

        /* Insert Node Boards */
        struct vi_node_board_t *board;
        int node_index;
        struct vi_net_node_t *node;
        LIST_FOR_EACH(net->node_list, node_index)
        {
                node = list_get(net->node_list, node_index);

                int x;
                int y;

                /* Get board Position */
                x = VI_NODE_BOARD_PADDING + (2 * node->X) * (VI_NODE_BOARD_PADDING + VI_NODE_BOARD_WIDTH);
                y = VI_NODE_BOARD_PADDING + ( 2 * node->Y) * (VI_NODE_BOARD_PADDING + VI_NODE_BOARD_HEIGHT);

                board = vi_node_board_create(node);
                list_add(panel->node_board_list, board );
                gtk_layout_put(GTK_LAYOUT(layout), board->widget, x, y);

                /* Size of layout */
                layout_width = MAX(layout_width, x + VI_NODE_BOARD_WIDTH + VI_NODE_BOARD_PADDING);
                layout_height = MAX(layout_height, y + VI_NODE_BOARD_HEIGHT + VI_NODE_BOARD_PADDING);

        }

        LIST_FOR_EACH(net->dummy_node_list, node_index)
        {
                node = list_get(net->dummy_node_list, node_index);

                int x;
                int y;

                /* Get Central position for a dummy LED */
                x =  VI_NODE_BOARD_PADDING/2 + VI_NODE_BOARD_WIDTH/2 + (2 * node->X ) *
                                (VI_NODE_BOARD_PADDING + VI_NODE_BOARD_WIDTH);
                y =  VI_NODE_BOARD_PADDING/2 + VI_NODE_BOARD_HEIGHT/2 + (2 * node->Y ) *
                                (VI_NODE_BOARD_PADDING + VI_NODE_BOARD_HEIGHT);
                board = vi_node_board_create(node);
                list_add(panel->node_board_list, board);
                gtk_layout_put(GTK_LAYOUT(layout), board->widget, x, y);

                /* Size of layout */
                layout_width = MAX(layout_width, x + VI_NODE_BOARD_WIDTH + VI_NODE_BOARD_PADDING);
                layout_height = MAX(layout_height, y + VI_NODE_BOARD_HEIGHT + VI_NODE_BOARD_PADDING);
        }

        /* Set layout size */
        gtk_widget_set_size_request(layout, layout_width, layout_height);

        /* Assign panel widget */
        panel->widget = frame;
        g_signal_connect(G_OBJECT(panel->widget), "destroy",
                        G_CALLBACK(vi_net_widget_destroy), panel);

        /* Return */
        return panel;
}
static struct vi_node_board_t *vi_node_board_create(struct vi_net_node_t *node)
{
        struct vi_node_board_t *board;

        char str[MAX_STRING_SIZE];

        /* Initialize */
        board = xcalloc(1, sizeof(struct vi_node_board_t));
        board->node = node;

        /* Color */
        GdkRGBA frame_color;

        int board_width = VI_NODE_BOARD_WIDTH;
        int board_height = VI_NODE_BOARD_HEIGHT;
        int has_label = 1;

        switch (node->type) {
        /* BLUE For Node */
        case vi_net_node_end:
                frame_color.red = .56;
                frame_color.green = 0.76;
                frame_color.blue = 0.83;
                frame_color.alpha = 1;
                break;
                /* Pink For Switch */
        case vi_net_node_switch:
                frame_color.red = .83;
                frame_color.green = 0.38;
                frame_color.blue = 0.72;
                frame_color.alpha = 1;
                break;
                /* ---- For Switch */
        case vi_net_node_bus:
        case vi_net_node_photonic:
                frame_color.red = .56;
                frame_color.green = .152;
                frame_color.blue = 0.53;
                frame_color.alpha = 1;
                break;
        case vi_net_node_dummy:
                frame_color.red = 0.74;
                frame_color.green = 0.74;
                frame_color.blue = 0.74;
                frame_color.alpha = 1;
                board_width = VI_NODE_BOARD_PADDING;
                board_height = VI_NODE_BOARD_PADDING;
                has_label = 0;
                break;
        default:
                break;
        }


        /* Frame */
        GtkWidget *frame = gtk_frame_new(NULL);
        GtkWidget *event_box = gtk_event_box_new();
        gtk_container_add(GTK_CONTAINER(event_box), frame);
        gtk_widget_override_background_color(event_box, GTK_STATE_FLAG_NORMAL, &frame_color);
        gtk_widget_set_size_request(frame, board_width , board_height);

        /* Vertical box */
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

        gtk_container_add(GTK_CONTAINER(frame), vbox);

        /* Name */
        if (has_label == 1)
        {
                if (node->mod)
                {
                        snprintf(str, sizeof str, "%s\n[%s]", node->name, node->mod->name);
                }
                else
                        snprintf(str, sizeof str, "%s", node->name);

                GtkWidget *label = gtk_label_new(NULL);
                gtk_label_set_markup(GTK_LABEL(label), str);
                gtk_label_set_justify (GTK_LABEL(label), GTK_JUSTIFY_CENTER);
                gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
                gtk_box_pack_start(GTK_BOX(vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 0);

                /* Horizontal box */
                GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
                gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

                /* Toggle button */
                GtkWidget *toggle_button = gtk_toggle_button_new_with_label("Detail");
                gtk_box_pack_start(GTK_BOX(hbox), toggle_button, TRUE, TRUE, 0);
                g_signal_connect(G_OBJECT(toggle_button), "toggled",
                                G_CALLBACK(vi_node_board_toggle_button_toggled), board);
                board->toggle_button = toggle_button;
        }
        /* Main widget */
        board->widget = event_box;
        g_signal_connect(G_OBJECT(board->widget), "destroy", G_CALLBACK(vi_node_board_destroy), board);

        /* Return */
        return board;

}

static GtkWidget *vi_node_window_get_widget(struct vi_node_window_t *node_window)
{
        return node_window->widget;
}

static gboolean vi_node_board_toggle_button_toggled (GtkWidget *widget, struct vi_node_board_t *board)
{
        struct vi_node_window_t *node_window;

        int active;

        /* Get button state */
        active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(board->toggle_button));

        /* Show */
        if (active && !board->node_window)
        {

                node_window = vi_node_window_create(board->node, board->toggle_button);
                board->node_window = node_window;
        }

        /* Hide */
        if (!active && board->node_window)
        {
                gtk_widget_destroy(vi_node_window_get_widget(board->node_window));
                board->node_window = NULL;
        }

        return FALSE;

}

static void vi_node_board_free(struct vi_node_board_t *board)
{
        /* Destroy pop-up window */
        if (board->node_window)
                gtk_widget_destroy(vi_node_window_get_widget(board->node_window));

        /* Free */
        free(board);
}

static void vi_node_board_destroy(GtkWidget *widget, struct vi_node_board_t *board)
{
        vi_node_board_free(board);
}

static struct vi_node_window_t *vi_node_window_create(struct vi_net_node_t * node, GtkWidget *parent_toggle_button)
{
        struct vi_node_window_t *node_window;

        char str[MAX_STRING_SIZE];

        /* Initialize */
        node_window = xcalloc(1, sizeof(struct vi_node_window_t));
        node_window->node = node;
        node_window->parent_toggle_button = parent_toggle_button;

        /* Main window */
        snprintf(str, sizeof str, "Network %s", node->name);
        GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), str);

        /* Module widget */
        struct vi_node_widget_t *node_widget;
        node_widget = vi_node_widget_create(node->name);
        gtk_container_add(GTK_CONTAINER(window), vi_node_widget_get_widget(node_widget));
        node_window->node_widget = node_widget;

        /* Associated widget */
        node_window->widget = window;
        gtk_widget_show_all(node_window->widget);
        g_signal_connect(G_OBJECT(node_window->widget), "destroy", G_CALLBACK(vi_node_window_destroy), node_window);
        g_signal_connect(G_OBJECT(node_window->widget), "delete_event", G_CALLBACK(vi_node_window_delete), node_window);

        return node_window;

}


static void vi_node_window_free(struct vi_node_window_t *node_window)
{
        free(node_window);
}

static void vi_node_window_destroy(GtkWidget *widget, struct vi_node_window_t *node_window)
{
        vi_node_window_free(node_window);
}

static gboolean vi_node_window_delete(GtkWidget *widget, GdkEvent *event, struct vi_node_window_t *node_window)
{
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(node_window->parent_toggle_button), FALSE);
        return TRUE;
}

static gboolean vi_link_board_draw (GtkWidget *widget, GdkEventConfigure *event, struct vi_link_board_t *board)
{
        GdkWindow *window;
        cairo_t *cr;

        struct vi_net_sub_link_t *subLink;
        subLink = board->subLink;
        struct vi_net_link_t *link;
        link = subLink->link;
/*
        int width;
        int height;

        width = gtk_widget_get_allocated_width(widget);
        height = gtk_widget_get_allocated_height(widget);
*/
        window = gtk_widget_get_window(widget);
        cr = gdk_cairo_create(window);

        /* Circle */
        cairo_set_source_rgb(cr, link->color.red,
                link->color.green, link->color.blue);

        cairo_set_line_width(cr, 10);
        cairo_move_to(cr, (int) subLink->src_x, (int) subLink->src_y);
        cairo_line_to(cr, (int) subLink->dst_x, (int) subLink->dst_y);
        cairo_stroke(cr);
//        cairo_arc(cr, width / 2, height / 2.0, MIN(width, height) / 3.0, 0., 2 * M_PI);
//        cairo_fill_preserve(cr);
//        cairo_set_source_rgb(cr, 0, 0, 0);

        /* Finish */
        cairo_stroke(cr);
        cairo_destroy(cr);
        return FALSE;

        /* Color
        cairo_set_line_width(cr, 1);
        cairo_move_to(cr, subLink->src_x, subLink->src_y);
        cairo_line_to(cr, subLink->dst_x, subLink->dst_y);
        cairo_set_source_rgb(cr, 0, 0, 0);

        cairo_stroke(cr);
        cairo_destroy(cr);
        return FALSE; */
}

static void vi_link_board_free(struct vi_link_board_t *board)
{
        free(board);
}
static void vi_link_board_destroy(GtkWidget *widget, struct vi_link_board_t *board)
{
        vi_link_board_free(board);
}
static struct vi_link_board_t  *vi_link_board_create (struct vi_net_sub_link_t *subLink)
{
        struct vi_link_board_t *board;

        /* Initialize */
        board = xcalloc(1, sizeof(struct vi_link_board_t));
        board->subLink = subLink;
        /* Drawing box */
        GdkRGBA color;
        color.red = 1;
        color.green = 1;
        color.blue = 1;
        color.alpha = 0;

        GtkWidget *drawing_area = gtk_drawing_area_new();
        gtk_widget_set_size_request(drawing_area,
                        MAX(subLink->src_x, subLink->dst_x) , MAX(subLink->src_y, subLink->dst_y));
        gtk_widget_override_background_color(drawing_area, GTK_STATE_NORMAL, &color);
        g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(vi_link_board_draw), board);

        /* Main widget */
        board->widget = drawing_area;
        g_signal_connect(G_OBJECT(board->widget), "destroy", G_CALLBACK(vi_link_board_destroy), board);

        /* Return */
        return board;
}

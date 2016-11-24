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

#ifndef VISUAL_NETWORK_LINK_H
#define VISUAL_NETWORK_LINK_H
#include <gtk/gtk.h>

struct vi_net_sub_link_t
{
        double src_x;
        double src_y;
        double dst_x;
        double dst_y;

        struct vi_net_link_t *link;
        int index;
};

struct vi_net_link_t
{
        long long transferred_bytes;
        int last_packet_size;
        long long busy_cycle;

        int   vc_number;
        int   bandwidth;
	char *name;
	GdkRGBA color;

	struct list_t *sublink_list;

	struct vi_net_node_t *src_node;
	struct vi_net_node_t *dst_node;
};

struct vi_trace_line_t;


struct vi_net_link_t *vi_net_link_create        (struct vi_trace_line_t *trace_line);
void                  vi_net_link_free          (struct vi_net_link_t *link);
void                  vi_link_color_utilization (struct vi_net_link_t *link);
void 		      vi_link_color_per_cycle   (struct vi_net_link_t *link);

struct vi_net_sub_link_t * vi_net_sub_link_create(void);
void                       vi_net_sub_link_free  (struct vi_net_sub_link_t * link);

void vi_link_read_checkpoint(struct vi_net_link_t *link, FILE *f);
void vi_link_write_checkpoint(struct vi_net_link_t *link, FILE *f);


#endif


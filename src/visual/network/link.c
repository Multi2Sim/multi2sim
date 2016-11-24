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

#include <assert.h>
#include <stdio.h>
#include <gtk/gtk.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/hash-table.h>
#include <lib/util/misc.h>
#include <lib/mhandle/mhandle.h>
#include <visual/common/cycle-bar.h>
#include <visual/common/state.h>
#include <visual/common/trace.h>


#include "link.h"
#include "net.h"
#include "net-system.h"

void vi_link_color_utilization(struct vi_net_link_t *link)
{
	GdkRGBA color = link->color;
	double utilization;
	long long current_cycle;

	current_cycle = vi_cycle_bar_get_cycle();

	utilization = (double) link->transferred_bytes / (current_cycle * link->bandwidth);

	if (utilization > 1)
		utilization = 1;

	if (utilization != 0)
	{
		color.blue = 0;
		color.green = ( 2 * utilization <= 1 ) ? 1 : 2 * (1 - utilization ) ;
		color.red = ( 2 * utilization >= 1 ) ? 1 : 2 * utilization ;
	}
	else
	{
		color.red = .8;
		color.green = .8;
		color.blue = .8;
		color.alpha = .8;
	}
	link->color = color;
}

void vi_link_color_per_cycle(struct vi_net_link_t *link)
{
	GdkRGBA color = link->color;
	double utilization;
	long long cycle;
	int lat;
	/* Go to cycle */
	cycle = vi_cycle_bar_get_cycle();

	lat = (link->last_packet_size - 1) / link->bandwidth + 1;
	int remain = 0;
	if (link->busy_cycle >= cycle)
		remain = link->last_packet_size - (cycle + lat - 1 - link->busy_cycle) * link->bandwidth;

	utilization = (double) remain / link->bandwidth;

	if (utilization > 1)
		utilization = 1;

	if (utilization > 0)
	{
		color.blue = 0;
		color.green = ( 2 * utilization <= 1 ) ? 1 : 2 * (1 - utilization ) ;
		color.red = ( 2 * utilization >= 1 ) ? 1 : 2 * utilization ;
	}
	else
	{
		color.red = .8;
		color.green = .8;
		color.blue = .8;
		color.alpha = .8;
	}
	link->color = color;
}
struct vi_net_link_t *vi_net_link_create(struct vi_trace_line_t *trace_line)
{
	struct vi_net_link_t *link;

	/* Allocate link */
	link = xcalloc(1, sizeof(struct vi_net_link_t));
	link->busy_cycle = 0;
	/* Get the corresponding network */
	struct vi_net_t *net;
	char *net_name;
	net_name = vi_trace_line_get_symbol(trace_line, "net_name");
	net = hash_table_get(vi_net_system->net_table, net_name);

	assert(net);

	/* Assign link name */
	char *link_name;
	link_name = vi_trace_line_get_symbol(trace_line, "link_name");
	link->name = xstrdup(link_name);

	/* Get Source node and Destination node of Link */
	char *src_node_name;
	char *dst_node_name;
	struct vi_net_node_t *src, *dst;

	src_node_name = vi_trace_line_get_symbol(trace_line, "src_node");
	src = hash_table_get(net->node_table, src_node_name);

	dst_node_name = vi_trace_line_get_symbol(trace_line, "dst_node");
	dst = hash_table_get(net->node_table, dst_node_name);

	assert(src);
	assert(dst);
	assert(src != dst);

	link->src_node = src;
	link->dst_node = dst;

	/* Get number of Virtual Channels */
	int vc_num;
	vc_num = vi_trace_line_get_symbol_int(trace_line, "vc_num");
	link->vc_number = vc_num;

	int bandwidth;
	bandwidth = vi_trace_line_get_symbol_int(trace_line, "bw");
	link->bandwidth = bandwidth;

	/* Setting initial Link color */
	link->color.red = .8;
	link->color.green = .8;
	link->color.blue = .8;
	link->color.alpha = .8;

	/* Creating link's sub-link list */
	link->sublink_list = list_create();

	/* Insert link in the network Hash table */
	hash_table_insert(net->link_table,link->name, link);

	/* Return Link */
	return link;
}

void vi_net_link_free(struct vi_net_link_t *link)
{
	for (int i = 0 ; i < list_count(link->sublink_list); i++)
	{
		vi_net_sub_link_free(list_get(link->sublink_list, i));
	}
	list_free(link->sublink_list);
	if (link->name)
		free(link->name);
	free(link);
}

struct vi_net_sub_link_t * vi_net_sub_link_create(void)
{
	struct vi_net_sub_link_t *subLink;
	subLink = xcalloc(1, sizeof (struct vi_net_sub_link_t));

	return subLink;
}
void vi_net_sub_link_free  (struct vi_net_sub_link_t * subLink)
{
	free(subLink);
}

void vi_link_read_checkpoint(struct vi_net_link_t *link, FILE *f)
{
	int count;
	int transfered_bytes;
	int last_packet_size;
	long long busy_cycle;

	count = fread(&transfered_bytes, 1, 4, f);
	if (count != 4)
		fatal("%s: error reading from checkpoint", __FUNCTION__);

	count = fread(&last_packet_size, 1, 4, f);
	if (count != 4)
		fatal("%s: error reading from checkpoint", __FUNCTION__);

	count = fread(&busy_cycle, 1, 8, f);
	if (count != 8)
		fatal("%s: error reading from checkpoint", __FUNCTION__);

	link->transferred_bytes = transfered_bytes;
	link->last_packet_size = last_packet_size;
	link->busy_cycle = busy_cycle;
}
void vi_link_write_checkpoint(struct vi_net_link_t *link, FILE *f)
{
	/* Transferred Bytes through Link */
	int count;
	count = fwrite(&link->transferred_bytes, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot write to checkpoint file", __FUNCTION__);

	/* Last packet Size */
	count = fwrite(&link->last_packet_size, 1, 4, f);
	if (count != 4)
		fatal("%s: cannot write to checkpoint file", __FUNCTION__);

	/* Last packet Size */
	count = fwrite(&link->busy_cycle, 1, 8, f);
	if (count != 8)
		fatal("%s: cannot write to checkpoint file", __FUNCTION__);

}


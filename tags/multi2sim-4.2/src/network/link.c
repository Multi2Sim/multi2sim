/* 
 *  Multi2Sim
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

#include <lib/esim/esim.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "buffer.h"
#include "link.h"
#include "net-system.h"
#include "network.h"
#include "node.h"


struct net_link_t *net_link_create(struct net_t *net,
	struct net_node_t *src_node, struct net_node_t *dst_node,
	int bandwidth, int link_src_bsize, int link_dst_bsize,
	int virtual_channel)
{
	struct net_link_t *link;
	struct net_buffer_t *src_buffer;
	struct net_buffer_t *dst_buffer;

	char name[MAX_STRING_SIZE];

	/* Fields */
	link = xcalloc(1, sizeof(struct net_link_t));
	link->net = net;
	link->src_node = src_node;
	link->dst_node = dst_node;
	link->bandwidth = bandwidth;
	link->virtual_channel = virtual_channel;


	for (int i = 0; i < virtual_channel; i++)
	{
		src_buffer = net_node_add_output_buffer(src_node, link_src_bsize);
		src_buffer->kind = net_buffer_link;
		dst_buffer = net_node_add_input_buffer(dst_node, link_dst_bsize);
		dst_buffer->kind = net_buffer_link;

		if (i == 0)
		{
			/* Name */
			snprintf(name, sizeof(name), "link_<%s.%s>_<%s.%s>",
				src_node->name, src_buffer->name,
				dst_node->name, dst_buffer->name);
			link->name = xstrdup(name);
			link->src_buffer = src_buffer;
			link->dst_buffer = dst_buffer;
		}

		/* Connect buffers to link */
		assert(!src_buffer->link);
		assert(!dst_buffer->link);
		src_buffer->link = link;
		dst_buffer->link = link;

	}
	if (bandwidth < 1)
		panic("%s: invalid bandwidth", __FUNCTION__);

	/* Return */
	return link;
}


void net_link_free(struct net_link_t *link)
{
	free(link->name);
	free(link);
}


void net_link_dump_report(struct net_link_t *link, FILE *f)
{
	struct net_t *net = link->net;
	long long cycle;

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	fprintf(f, "[ Network.%s.Link.%s ]\n", net->name, link->name);
	fprintf(f, "Config.Bandwidth = %d\n", link->bandwidth);
	fprintf(f, "TransferredMessages = %lld\n", link->transferred_msgs);
	fprintf(f, "TransferredBytes = %lld\n", link->transferred_bytes);
	fprintf(f, "BusyCycles = %lld\n", link->busy_cycles);
	fprintf(f, "BytesPerCycle = %.4f\n", cycle ?
		(double) link->transferred_bytes / cycle : 0.0);
	fprintf(f, "Utilization = %.4f\n", cycle ?
		(double) link->transferred_bytes / (cycle *
			link->bandwidth) : 0.0);
	fprintf(f, "\n");
}


struct net_buffer_t *net_link_arbitrator_vc(struct net_link_t *link,
	struct net_node_t *node)
{
	struct net_node_t *src_node = link->src_node;
	struct net_node_t *dst_node = link->dst_node;
	int output_buffer_count = list_count(src_node->output_buffer_list);
	int input_buffer_count = list_count(dst_node->input_buffer_list);

	struct net_buffer_t *output_buffer;
	struct net_msg_t *msg;

	/* Keeping indexes of last chosen buffer */
	int last_output_buffer_vc_index;
	int output_buffer_vc_index;
	int i;

	long long cycle;

	/* performing the check */
	assert(node == src_node);

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	/* Getting the indexes of first output buffer and input buffer that
	 * are connected to a link with virtual channel capability */
	int out_vc_count = 0;
	int in_vc_count = 0;
	int first_src_buffer_act_index = 0;
	int first_dst_buffer_act_index = 0;


	for (i = 0; i < output_buffer_count; i++)
	{
		struct net_buffer_t *temp_out_buffer;

		temp_out_buffer = list_get(src_node->output_buffer_list, i);
		if (temp_out_buffer->link == link)
			out_vc_count++;
		if (out_vc_count == 1)
			first_src_buffer_act_index = temp_out_buffer->index;
	}

	for (int j = 0; j < input_buffer_count; j++)
	{
		struct net_buffer_t *temp_in_buffer;

		temp_in_buffer = list_get(dst_node->input_buffer_list, j);
		if (temp_in_buffer->link == link)
			in_vc_count++;
		if (in_vc_count == 1)
			first_dst_buffer_act_index = temp_in_buffer->index;
	}

	/* Checks */
	assert(in_vc_count == link->virtual_channel);
	assert(out_vc_count == link->virtual_channel);

	/* If last decision was within the same cycle, return the same value */
	if (link->sched_when == cycle)
		return link->sched_buffer;

	/* make a new decision */
	link->sched_when = cycle;
	last_output_buffer_vc_index = link->sched_buffer ?
		(link->sched_buffer->index - first_src_buffer_act_index) : 0;

	/* link must be ready */
	if (link->busy >= cycle)
	{
		link->sched_buffer = NULL;
		return NULL;
	}

	/* find output buffer to fetch from */
	for (i = 0; i < link->virtual_channel; i++)
	{
		output_buffer_vc_index =
			(last_output_buffer_vc_index + i +
			1) % link->virtual_channel;
		output_buffer =
			list_get(node->output_buffer_list,
			(output_buffer_vc_index +
				first_src_buffer_act_index));
		assert(output_buffer->link == link);

		/* msg should be at head */
		msg = list_get(output_buffer->msg_list, 0);
		if (!msg)
			continue;

		/* msg must be ready */
		if (msg->busy >= cycle)
			continue;

		/* output buffer must be ready */
		if (output_buffer->read_busy >= cycle)
			continue;

		/* ALL conditions satisfied */
		link->sched_buffer = output_buffer;
		link->src_buffer = output_buffer;
		struct net_buffer_t *input_buffer;

		input_buffer = list_get(dst_node->input_buffer_list,
			(output_buffer_vc_index +
				first_dst_buffer_act_index));
		assert(input_buffer->link == link);
		link->dst_buffer = input_buffer;
		return output_buffer;
	}
	/* No output buffer ready */
	link->sched_buffer = NULL;
	return NULL;
}

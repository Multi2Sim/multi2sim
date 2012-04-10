/*
 *  Multi2Sim
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

#include <network.h>


struct net_link_t *net_link_create(struct net_t *net,
	struct net_node_t *src_node, struct net_buffer_t *src_buffer,
	struct net_node_t *dst_node, struct net_buffer_t *dst_buffer,
	int bandwidth)
{
	struct net_link_t *link;
	char name[MAX_STRING_SIZE];

	/* Create */
	link = calloc(1, sizeof(struct net_link_t));
	if (!link)
		fatal("%s: out of memory", __FUNCTION__);

	/* Fields */
	link->net = net;
	link->src_node = src_node;
	link->src_buffer = src_buffer;
	link->dst_node = dst_node;
	link->dst_buffer = dst_buffer;
	link->bandwidth = bandwidth;
	assert(src_buffer->node == src_node);
	assert(dst_buffer->node == dst_node);
	if (bandwidth < 1)
		panic("%s: invalid bandwidth", __FUNCTION__);

	/* Connect buffers to link */
	assert(!src_buffer->link);
	assert(!dst_buffer->link);
	src_buffer->link = link;
	dst_buffer->link = link;

	/* Name */
	snprintf(name, sizeof(name), "link_<%s.%s>_<%s.%s>", src_node->name,
		src_buffer->name, dst_node->name, dst_buffer->name);
	link->name = strdup(name);

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

	fprintf(f, "[ Network.%s.Link.%s ]\n", net->name, link->name);
	fprintf(f, "Config.Bandwidth = %d\n", link->bandwidth);
	fprintf(f, "TransferredMessages = %lld\n", link->transferred_msgs);
	fprintf(f, "TransferredBytes = %lld\n", link->transferred_bytes);
	fprintf(f, "BusyCycles = %lld\n", link->busy_cycles);
	fprintf(f, "BytesPerCycle = %.4f\n", esim_cycle ?
			(double) link->transferred_bytes / esim_cycle : 0.0);
	fprintf(f, "Utilization = %.4f\n", esim_cycle ?
			(double) link->transferred_bytes / (esim_cycle * link->bandwidth) : 0.0);
	fprintf(f, "\n");
}


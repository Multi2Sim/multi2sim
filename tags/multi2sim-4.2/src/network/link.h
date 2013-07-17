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

#ifndef NETWORK_LINK_H
#define NETWORK_LINK_H

#include <stdio.h>


struct net_link_t
{
	struct net_t *net;
	char *name;

	/* Source node-buffer */
	struct net_node_t *src_node;

	/* Destination node-buffer */
	struct net_node_t *dst_node;

	/* Buffers that have the control of the link */
	struct net_buffer_t *dst_buffer;
	struct net_buffer_t *src_buffer;

	int bandwidth;
	long long busy;		/* Busy until this cycle inclusive */

	/* Scheduling for link */
	int virtual_channel;	/* Number of Virtual Channels on a Link*/
	long long sched_when;	/* The last time a buffer was assigned to Link */
	struct net_buffer_t *sched_buffer;	/* The output buffer to fetch from*/

	/* Stats */
	long long busy_cycles;
	long long transferred_bytes;
	long long transferred_msgs;
};


/* Functions */
void net_link_free(struct net_link_t *link);
struct net_link_t *net_link_create(struct net_t *net,
	struct net_node_t *src_node, struct net_node_t *dst_node,
	int bandwidth, int link_src_bsize, int link_dst_bsize, int vc);

struct net_buffer_t *net_link_arbitrator_vc(struct net_link_t *link,
	struct net_node_t *node);

void net_link_dump_report(struct net_link_t *link, FILE *f);


#endif

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

#ifndef NETWORK_BUS_H
#define NETWORK_BUS_H

#include <stdio.h>


struct net_bus_t
{
	struct net_t *net;
	struct net_node_t *node;
	char *name;
	int index;
	int bandwidth;
	long long busy;		/* Busy until this cycle inclusive */

	/* Stats */
	long long busy_cycles;
	long long transferred_bytes;
	long long transferred_msgs;

	/* Scheduling */
	long long sched_when;
	struct net_buffer_t *sched_buffer;


};
struct net_bus_t *net_bus_create(struct net_t *net, struct net_node_t *node,
	int bandwidth, char *name);
void net_bus_free(struct net_bus_t *bus);
struct net_bus_t *net_bus_arbitration(struct net_node_t *bus_node,
	struct net_buffer_t *buffer);
void net_bus_dump_report(struct net_bus_t *bus, FILE *f);
#endif

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
#ifndef NETWORK_PACKET_H
#define NETWORK_PACKET_H

struct net_packet_t
{
	struct net_t *net;

	struct net_msg_t *msg;
	int tier;
	int session_id;
	int size;
	long long busy; /* in transit until cycle */

	/* Current position in network */ //[K12]
	struct net_node_t *node;
	struct net_buffer_t *buffer;


	/*for Using LINKED_LIST Macros */
	struct net_packet_t *packet_list_next;
	struct net_packet_t *packet_list_prev;

	/* For re-Packetizing */
	struct net_packet_t *flit_list_head, *flit_list_tail;
	int flit_list_count;
	int flit_list_max;

};


struct net_packet_t *net_packet_create(struct net_t *net, struct net_msg_t *msg, int size);
void net_packet_free(struct net_packet_t *pkt);
void net_packetizer(struct net_t *net, struct net_msg_t *msg, int packet_size);
int net_depacketizer (struct net_t *net, struct net_node_t *node, struct net_packet_t *pkt);
/*
struct net_packet_t *net_packet_create(struct net_t *net, struct net_msg_t *msg, int size);

void net_packet_free(struct net_packet_t *pkt);
*/
//void net_packet_packetizer(struct net_t *net, struct net_msg_t *msg);


#endif /* NETWORK_PACKET_H */

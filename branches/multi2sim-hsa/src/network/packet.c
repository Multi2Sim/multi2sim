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
#include <lib/esim/trace.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>

#include "net-system.h"
#include "message.h"
#include "network.h"
#include "node.h"
#include "packet.h"
#include "buffer.h"


struct net_packet_t *net_packet_create(struct net_t *net, struct net_msg_t *msg, int size)
{
	struct net_packet_t *pkt;

	pkt= xcalloc(1, sizeof(struct net_packet_t));
	pkt->net = net;
	pkt->msg = msg;
	pkt->size = size;
	if (size < 1)
		panic("%s: bad size", __FUNCTION__);

	return pkt;
}

void net_packet_free (struct net_packet_t *pkt)
{
	free(pkt);
}

/*
 * We pass net->packet_size for network simulation cases. What we want is as follows:
 * 1) if net simulation is on-going and we have a packet size != 0, we go with packet size
 * 2) if net simulation is on-going and we have a packet_size == 0, we go with default msg size
 * 3) if actual simulation and packet_size != 0 we always send the packet size
 * 4) if actual sim and packet_sze == 0 we send 1 packet equal the size of the msg ***
 */
void net_packetizer(struct net_t *net, struct net_msg_t *msg, int packet_size)
{
	int packet_count;

	packet_count = (msg->size - 1) / packet_size + 1 ;

	assert(packet_count > 0);

	for (int i = 0; i < packet_count ; i++)
	{
		struct net_packet_t *packet;

		packet = net_packet_create(net, msg, packet_size); // rp net->packet_size with packet_size
		packet->session_id = i;
		DOUBLE_LINKED_LIST_INSERT_TAIL(msg, packet, packet);

		net_trace("net.new_packet net=\"%s\" name=\"P-%lld:%d\" size=%d state=\"%s:packetizer\"\n",
				net->name, msg->id, packet->session_id, packet->size, msg->src_node->name);
		net_trace("net.packet_msg net=\"%s\" name=\"P-%lld:%d\" message=\"M-%lld\"\n",
						net->name, msg->id, packet->session_id, msg->id);
	}
}

int net_depacketizer (struct net_t *net, struct net_node_t *node, struct net_packet_t *pkt)
{
	struct net_msg_t *msg = pkt->msg;
	int count =msg->packet_list_max;

	struct net_buffer_t *buffer = pkt->buffer;
	assert(buffer->node == node);
	if (!list_count(buffer->msg_list))
		panic("%s: empty buffer", __FUNCTION__);
	if (list_get(buffer->msg_list, 0) != pkt)
		panic("%s: message not at input buffer head", __FUNCTION__);
	DOUBLE_LINKED_LIST_INSERT_TAIL(msg, packet, pkt);
	net_buffer_extract(buffer, pkt);

	net_trace("net.packet net=\"%s\" name=\"P-%lld:%d\" state=\"%s:depacketizer\" stg=\"DC\"\n",
			net->name, msg->id, pkt->session_id, node->name);
	for (int i = 0; i < msg->packet_list_count; i++)
	{
		/* All the packets are received */
		count--;
		if (count == 0)
			return 1;
	}
	return 0;
}

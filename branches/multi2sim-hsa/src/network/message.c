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

#include "buffer.h"
#include "bus.h"
#include "link.h"
#include "message.h"
#include "net-system.h"
#include "network.h"
#include "node.h"
#include "routing-table.h"
#include "packet.h"


/* 
 * Message
 */

struct net_msg_t *net_msg_create(struct net_t *net,
		struct net_node_t *src_node, struct net_node_t *dst_node, int size)
{
	struct net_msg_t *msg;
	long long cycle;

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	/* Initialize */
	msg = xcalloc(1, sizeof(struct net_msg_t));
	msg->net = net;
	msg->src_node = src_node;
	msg->dst_node = dst_node;
	msg->size = size;
	msg->id = ++net->msg_id_counter;
	msg->send_cycle = cycle;
	if (size < 1)
		panic("%s: bad size", __FUNCTION__);

	/* Return */
	return msg;
}


void net_msg_free(struct net_msg_t *msg)
{
	while (msg->packet_list_head)
	{
		struct net_packet_t *pkt;
		pkt = msg->packet_list_head;
		DOUBLE_LINKED_LIST_REMOVE(msg, packet, pkt);
		net_packet_free(pkt);
	}
	free(msg);
}




/* 
 * Event-driven Simulation
 */

struct net_stack_t *net_stack_create(struct net_t *net,
		int retevent, void *retstack)
{
	struct net_stack_t *stack;

	/* Initialize */
	stack = xcalloc(1, sizeof(struct net_stack_t));
	stack->net = net;
	stack->ret_event = retevent;
	stack->ret_stack = retstack;

	/* Return */
	return stack;
}


void net_stack_return(struct net_stack_t *stack)
{
	int retevent = stack->ret_event;
	struct net_stack_t *retstack = stack->ret_stack;

	free(stack);
	esim_schedule_event(retevent, retstack, 0);
}


void net_event_handler(int event, void *data)
{
	struct net_stack_t *stack = data;

	struct net_t *net = stack->net;
	struct net_routing_table_t *routing_table = net->routing_table;
	struct net_packet_t *pkt= stack->packet;

	struct net_node_t *src_node = pkt->msg->src_node;
	struct net_node_t *dst_node = pkt->msg->dst_node;

	struct net_node_t *node = pkt->node;
	struct net_buffer_t *buffer = pkt->buffer;

	long long cycle;

	/* Get current cycle */
	cycle = esim_domain_cycle(net_domain_index);

	if ((net_snap_period != 0) &&
	                (net->last_recorded_cycle < (cycle/net_snap_period )))
	        net_bandwidth_snapshot(net, cycle);

	if (event == EV_NET_SEND)
	{
		struct net_routing_table_entry_t *entry;
		struct net_buffer_t *output_buffer;

		/* Get output buffer */
		entry = net_routing_table_lookup(routing_table, src_node,
				dst_node);
		output_buffer = entry->output_buffer;
		if (!output_buffer)
			fatal("%s: no route from %s to %s.\n%s", net->name,
					src_node->name, dst_node->name,
					net_err_no_route);

		if (pkt->msg->size > output_buffer->size)
			panic("%s: message does not fit in buffer.\n%s",
					__FUNCTION__, net_err_can_send);

		if (output_buffer->count + pkt->size > output_buffer->size)
			panic("%s: output buffer full.\n%s", __FUNCTION__,
					net_err_can_send);

		/* Insert in output buffer (1 cycle latency) */
		net_buffer_insert(output_buffer, pkt);
		output_buffer->write_busy = cycle;
		pkt->node = src_node;
		pkt->buffer = output_buffer;
		pkt->busy = cycle;

		/* Schedule next event */
		esim_schedule_event(EV_NET_OUTPUT_BUFFER, stack, 1);

	}

	else if (event == EV_NET_OUTPUT_BUFFER)
	{
		struct net_buffer_t *input_buffer;
		int lat;

		/* Debug */
		net_debug("msg "
				"a=\"obuf\" "
				"net=\"%s\" "
				"msg-->pkt=%lld-->%d "
				"node=\"%s\" "
				"buf=\"%s\"\n",
				net->name,
				pkt->msg->id,
				pkt->session_id,
				node->name,
				buffer->name);

		/* If message is not at buffer head, process later */
		assert(list_count(buffer->msg_list));

		if (list_get(buffer->msg_list, 0) != pkt)
		{
			net_buffer_wait(buffer, event, stack);
			net_debug("msg "
					"a=\"stall\" "
					"net=\"%s\" "
					"msg-->packet=%lld:%d "
					"why=\"not output buffer head\"\n",
					net->name,
					pkt->msg->id,
					pkt->session_id);
			return;
		}

		if (buffer->read_busy >= cycle)
		{
			esim_schedule_event(event, stack,
					buffer->read_busy - cycle + 1);
			net_debug("msg "
					"a=\"stall\" "
					"net=\"%s\" "
					"msg-->pkt=%lld:%d "
					"why=\"output buffer busy\" \n",
					net->name,
					pkt->msg->id,
					pkt->session_id);
			return;
		}

		/* If link is busy, wait */
		if (buffer->kind == net_buffer_link)
		{
			struct net_link_t *link;

			assert(buffer->link);
			link = buffer->link;
			if (link->busy >= cycle)
			{
				esim_schedule_event(event, stack,
						link->busy - cycle + 1);
				net_debug("msg "
						"a=\"stall\" "
						"net=\"%s\" "
						"msg-->pkt=%lld:%d "
						"why=\"link busy\"\n",
						net->name,
						pkt->msg->id,
						pkt->session_id);

				net_trace("net.packet "
						"net=\"%s\" "
						"name=\"P-%lld:%d\" "
						"state=\"%s:%s:link_busy\" "
						"stg=\"LB\"\n",
						net->name, pkt->msg->id,
						pkt->session_id,
						node->name,
						buffer->name);
				return;
			}

			/* If buffer contain the message but doesn't have the 
			 * shared link in control, wait */
			if (link->virtual_channel > 1)
			{
				struct net_buffer_t *temp_buffer;

				temp_buffer = net_link_arbitrator_vc(link, node);
				if (temp_buffer != buffer)
				{
					net_debug("msg "
							"a=\"stall\" "
							"net=\"%s\" "
							"msg-->pkt=%lld:%d "
							"why=\"arbitrator sched\"\n",
							net->name,
							pkt->msg->id,
							pkt->session_id);
					esim_schedule_event(event, stack, 1);

					net_trace("net.packet "
							"net=\"%s\" "
							"name=\"P-%lld:%d\" "
							"state=\"%s:%s:VC_arbitration_fail\" "
							"stg=\"VCA\"\n",
							net->name, pkt->msg->id,
							pkt->session_id,
							node->name,
							buffer->name);
					return;
				}
			}

			/* If destination input buffer is busy, wait */
			assert(buffer == link->src_buffer);
			input_buffer = link->dst_buffer;
			if (input_buffer->write_busy >= cycle)
			{
				net_debug("msg "
						"a=\"stall\" "
						"net=\"%s\" "
						"msg-->pkt=%lld:%d "
						"why=\"input buffer busy\"\n",
						net->name, pkt->msg->id,
						pkt->session_id);
				net_trace("net.packet "
						"net=\"%s\" "
						"name=\"P-%lld:%d\" "
						"state=\"%s:%s:Dest_buffer_busy\" "
						"stg=\"DBB\"\n",
						net->name, pkt->msg->id,
						pkt->session_id,
						node->name,
						buffer->name);

				esim_schedule_event(event, stack,
						input_buffer->write_busy - cycle + 1);
				return;
			}

			/* If destination input buffer is full, wait */
			if (pkt->size > input_buffer->size)
				fatal("%s: packet does not fit in buffer.\n%s",
						net->name, net_err_large_message);
			if (input_buffer->count + pkt->size >
			input_buffer->size)
			{
				net_debug("msg "
						"a=\"stall\" "
						"net=\"%s\" "
						"msg-->pkt=%lld:%d "
						"why=\"input buffer full\"\n",
						net->name, pkt->msg->id, pkt->session_id);
				net_trace("net.packet "
						"net=\"%s\" "
						"name=\"P-%lld:%d\" "
						"state=\"%s:%s:Dest_buffer_full\" "
						"stg=\"DBF\"\n",
						net->name, pkt->msg->id,
						pkt->session_id,
						node->name,
						buffer->name);

				net_buffer_wait(input_buffer, event, stack);
				return;
			}

			/* Calculate latency and occupy resources */
			lat = (pkt->size - 1) / link->bandwidth + 1;
			assert(lat > 0);
			buffer->read_busy = cycle + lat - 1;
			link->busy = cycle + lat - 1;
			input_buffer->write_busy = cycle + lat - 1;

			/* Transfer message to next input buffer */
			assert(pkt->busy < cycle);
			net_buffer_extract(buffer, pkt);
			net_buffer_insert(input_buffer, pkt);
			pkt->node = input_buffer->node;
			pkt->buffer = input_buffer;
			pkt->busy = cycle + lat - 1;

			/* Stats */
			link->busy_cycles += lat;
			link->transferred_bytes += pkt->size;
			link->transferred_msgs++;

			net->topology_util_bw += pkt->size;

			node->bytes_sent += pkt->size;
			node->msgs_sent++;
			input_buffer->node->bytes_received += pkt->size;
			input_buffer->node->msgs_received++;
			net_trace("net.link_transfer net=\"%s\" link=\"%s\" "
					"transB=%lld last_size=%d busy=%lld\n",
					net->name, link->name,
					link->transferred_bytes,
					pkt->size, link->busy);
		}
		else if (buffer->kind == net_buffer_bus)
		{
			struct net_bus_t *bus, *updated_bus;
			struct net_node_t *bus_node;

			assert(!buffer->link);
			assert(buffer->bus);
			bus = buffer->bus;
			bus_node = bus->node;

			/* before initiating bus transfer we have to figure out what is the
			 * next input buffer since it is not clear from the
			 * output buffer */
			int input_buffer_detection = 0;
			struct net_routing_table_entry_t *entry;

			entry = net_routing_table_lookup(routing_table,
					pkt->node, pkt->msg->dst_node);

			for (int i = 0; i < list_count(bus_node->dst_buffer_list); i++)
			{
				input_buffer = list_get(bus_node->dst_buffer_list, i);
				if (entry->next_node == input_buffer->node)
				{
					input_buffer_detection = 1;
					break;
				}
			}
			if (input_buffer_detection == 0)
				fatal("%s: Something went wrong so there is no appropriate input"
						"buffer for the route between %s and %s \n", net->name,
						pkt->node->name,entry->next_node->name);

			/* 1. Check the destination buffer is busy or not */
			if (input_buffer->write_busy >= cycle)
			{
				esim_schedule_event(event, stack,
						input_buffer->write_busy - cycle + 1);
				net_debug("msg "
						"a=\"stall\" "
						"net=\"%s\" "
						"msg-->pkt=%lld:%d "
						"why=\"input busy\"\n",
						net->name,
						pkt->msg->id,
						pkt->session_id);

				net_trace("net.packet "
						"net=\"%s\" "
						"name=\"P-%lld:%d\" "
						"state=\"%s:%s:Dest_buffer_busy\" "
						"stg=\"DBB\"\n",
						net->name, pkt->msg->id,
						pkt->session_id,
						node->name,
						buffer->name);

				return;
			}

			/* 2. Check the destination buffer is full or not */
			if (pkt->size > input_buffer->size)
				fatal("%s: packet  does not fit in buffer.\n%s",
						net->name, net_err_large_message);

			if (input_buffer->count + pkt->size > input_buffer->size)
			{
				net_buffer_wait(input_buffer, event, stack);
				net_debug("msg "
						"a=\"stall\" "
						"net=\"%s\" "
						"msg-->pkt=%lld:%d "
						"why=\"input full\"\n",
						net->name, pkt->msg->id,
						pkt->session_id);

				net_trace("net.packet "
						"net=\"%s\" "
						"name=\"P-%lld:%d\" "
						"state=\"%s:%s:Dest_buffer_full\" "
						"stg=\"DBF\"\n",
						net->name, pkt->msg->id,
						pkt->session_id,
						node->name,
						buffer->name);

				return;
			}

			/* 3. Make sure if any bus is available; return one
			 * that is available the fastest */
			updated_bus = net_bus_arbitration(bus_node, buffer);
			if (updated_bus == NULL)
			{
				esim_schedule_event(event, stack, 1);
				net_debug("msg "
						"a=\"stall\" "
						"net=\"%s\" "
						"msg-->pkt=%lld:%d "
						"why=\"bus arbiter\"\n",
						net->name, pkt->msg->id,
						pkt->session_id);
				net_trace("net.packet "
						"net=\"%s\" "
						"name=\"P-%lld:%d\" "
						"state=\"%s:%s:BUS_arbit_fail\" "
						"stg=\"BA\"\n",
						net->name, pkt->msg->id,
						pkt->session_id,
						node->name,
						buffer->name);

				return;
			}

			/* 4. assign the bus to the buffer. update the
			 * necessary data ; before here, the bus is not
			 * assign to anything and is not updated so it can be 
			 * assign to other buffers as well. If this certain
			 * buffer wins that specific bus_lane the appropriate
			 * fields will be updated. Contains: bus_lane
			 * cin_buffer and cout_buffer and busy time as well as
			 * buffer data itself */
			assert(updated_bus);
			buffer->bus = updated_bus;
			input_buffer->bus = updated_bus;
			bus = buffer->bus;
			assert(bus);


			/* Calculate latency and occupy resources */
			/* Wire delay is introduced when the packet is on transit */
			lat = bus->fix_delay + ((pkt->size - 1) / bus->bandwidth + 1) ;
			assert(lat > 0);
			buffer->read_busy = cycle + lat - 1;
			bus->busy = cycle + lat - 1;
			input_buffer->write_busy = cycle + lat - 1 ;

			/* Transfer message to next input buffer */
			assert(pkt->busy < cycle);
			net_buffer_extract(buffer, pkt);
			net_buffer_insert(input_buffer, pkt);
			pkt->node = input_buffer->node;
			pkt->buffer = input_buffer;
			pkt->busy = cycle + lat - 1;

			/* Stats */
			bus->busy_cycles += lat;
			bus->transferred_bytes += pkt->size;
			bus->transferred_msgs++;

                        net->topology_util_bw += pkt->size;

			node->bytes_sent += pkt->size;
			node->msgs_sent++;
			input_buffer->node->bytes_received += pkt->size;
			input_buffer->node->msgs_received++;
			net_trace("net.bus_transfer net=\"%s\" node=\"%s\" "
					"lane_index=%d transB=%lld last_size=%d busy=%lld\n",
					net->name, bus->node->name, bus->index,
					bus->transferred_bytes, pkt->size, bus->busy);
		}

		else if (buffer->kind == net_buffer_photonic)
		{
			struct net_bus_t *bus, *updated_bus;
			struct net_node_t *bus_node;

			assert(!buffer->link);
			assert(buffer->bus);
			bus = buffer->bus;
			bus_node = bus->node;

			/* before 1 and 2 we have to figure out what is the
			 * next input buffer since it is not clear from the
			 * output buffer */
			int input_buffer_detection = 0;
			struct net_routing_table_entry_t *entry;

			entry = net_routing_table_lookup(routing_table,
					pkt->node, pkt->msg->dst_node);

			for (int i = 0; i < list_count(bus_node->dst_buffer_list); i++)
			{
				input_buffer = list_get(bus_node->dst_buffer_list, i);
				if (entry->next_node == input_buffer->node)
				{
					input_buffer_detection = 1;
					break;
				}
			}
			if (input_buffer_detection == 0)
				fatal("%s: Something went wrong so there is no appropriate input"
						"buffer for the route between %s and %s \n", net->name,
						pkt->node->name,entry->next_node->name);

			/* 1. Check the destination buffer is busy or not */
			if (input_buffer->write_busy > cycle)
			{
				esim_schedule_event(event, stack,
						input_buffer->write_busy - cycle + 1);
				net_debug("msg "
						"a=\"stall\" "
						"net=\"%s\" "
						"msg-->pkt=%lld:%d "
						"why=\"input busy\"\n",
						net->name,
						pkt->msg->id,
						pkt->session_id);

				net_trace("net.packet "
						"net=\"%s\" "
						"name=\"P-%lld:%d\" "
						"state=\"%s:%s:Dest_buffer_busy\" "
						"stg=\"DBB\"\n",
						net->name, pkt->msg->id,
						pkt->session_id,
						node->name,
						buffer->name);

				return;
			}

			/* 2. Check the destination buffer is full or not */
			if (pkt->size > input_buffer->size)
				fatal("%s: message does not fit in buffer.\n%s",
						net->name, net_err_large_message);

			if (input_buffer->count + pkt->size > input_buffer->size)
			{
				net_buffer_wait(input_buffer, event, stack);
				net_debug("msg "
						"a=\"stall\" "
						"net=\"%s\" "
						"msg-->pkt=%lld:%d "
						"why=\"input full\"\n",
						net->name, pkt->msg->id,
						pkt->session_id);

				net_trace("net.packet "
						"net=\"%s\" "
						"name=\"P-%lld:%d\" "
						"state=\"%s:%s:Dest_buffer_full\" "
						"stg=\"DBF\"\n",
						net->name, pkt->msg->id,
						pkt->session_id,
						node->name,
						buffer->name);

				return;
			}


			/* 3. Make sure if any bus is available; return one
			 * that is available the fastest */
			updated_bus = net_photo_link_arbitration(bus_node, buffer);
			if (updated_bus == NULL)
			{
				esim_schedule_event(event, stack, 1);
				net_debug("msg "
						"a=\"stall\" "
						"net=\"%s\" "
						"msg-->pkt=%lld:%d "
						"why=\"bus arbiter\"\n",
						net->name, pkt->msg->id,
						pkt->session_id);

				net_trace("net.packet "
						"net=\"%s\" "
						"name=\"P-%lld:%d\" "
						"state=\"%s:%s:photonic_arbitration\" "
						"stg=\"BA\"\n",
						net->name, pkt->msg->id,
						pkt->session_id,
						node->name,
						buffer->name);

				return;
			}

			/* 4. assign the bus to the buffer. update the
			 * necessary data ; before here, the bus is not
			 * assign to anything and is not updated so it can be
			 * assign to other buffers as well. If this certain
			 * buffer wins that specific bus_lane the appropriate
			 * fields will be updated. Contains: bus_lane
			 * cin_buffer and cout_buffer and busy time as well as
			 * buffer data itself */
			assert(updated_bus);
			buffer->bus = updated_bus;
			input_buffer->bus = updated_bus;
			bus = buffer->bus;
			assert(bus);

			/* Calculate latency and occupy resources */
			lat = (pkt->size - 1) / bus->bandwidth + 1;
			assert(lat > 0);
			buffer->read_busy = cycle + lat - 1;
			bus->busy = cycle + lat - 1;
			input_buffer->write_busy = cycle + lat - 1;

			/* Transfer message to next input buffer */
			assert(pkt->busy < cycle);
			net_buffer_extract(buffer, pkt);
			net_buffer_insert(input_buffer, pkt);
			pkt->node = input_buffer->node;
			pkt->buffer = input_buffer;
			pkt->busy = cycle + lat - 1;

			/* Stats */
			bus->busy_cycles += lat;
			bus->transferred_bytes += pkt->size;
			bus->transferred_msgs++;

                        net->topology_util_bw += pkt->size;

			node->bytes_sent += pkt->size;
			node->msgs_sent++;
			input_buffer->node->bytes_received += pkt->size;
			input_buffer->node->msgs_received++;
			net_trace("net.photonic_transfer net=\"%s\" node=\"%s\" "
					"lane_index=%d transB=%lld last_size=%d busy=%lld\n",
					net->name, bus->node->name, bus->index,
					bus->transferred_bytes,pkt->size, bus->busy);
			net_debug("msg "
					"a=\"success photonic transmission\" "
					"net=\"%s\" "
					"msg-->pkt=%lld:%d "
					"through = \" %d\"\n",
					net->name, pkt->msg->id,
					pkt->session_id, updated_bus->index);
		}

		/* Schedule next event */
		esim_schedule_event(EV_NET_INPUT_BUFFER, stack, lat);
	}

	else if (event == EV_NET_INPUT_BUFFER)
	{
		struct net_routing_table_entry_t *entry;
		struct net_buffer_t *output_buffer;

		int lat;

		/* Debug */
		net_debug("msg "
				"a=\"ibuf\" "
				"net=\"%s\" "
				"msg-->pkt=%lld:%d "
				"node=\"%s\" "
				"buf=\"%s\"\n",
				net->name,
				pkt->msg->id,
				pkt->session_id,
				node->name,
				buffer->name);

		/* If message is not at buffer head, process later */
		assert(list_count(buffer->msg_list));
		if (list_get(buffer->msg_list, 0) != pkt)
		{
			net_debug("msg "
					"a=\"stall\" "
					"net=\"%s\" "
					"msg-->pkt=%lld:%d"
					"why=\"not-head\"\n",
					net->name, pkt->msg->id,
					pkt->session_id);
			net_buffer_wait(buffer, event, stack);
			return;
		}

		/* If this is the destination node, finish */
		if (node == pkt->msg->dst_node)
		{
			esim_schedule_event(EV_NET_RECEIVE, stack, 0);
			return;
		}

		/* If source input buffer is busy, wait */
		if (buffer->read_busy >= cycle)
		{
			net_debug("pkt"
					"a=\"stall\" "
					"net=\"%s\" "
					"msg-->pkt=%lld:%d "
					"why=\"src-busy\"\n",
					net->name,
					pkt->msg->id,
					pkt->session_id);

			esim_schedule_event(event, stack,
					buffer->read_busy - cycle + 1);
			return;
		}

		/* Get output buffer */
		entry = net_routing_table_lookup(routing_table, node,
				dst_node);
		output_buffer = entry->output_buffer;
		if (!output_buffer)
			fatal("%s: no route from %s to %s.\n%s", net->name,
					node->name, dst_node->name, net_err_no_route);

		/* If destination output buffer is busy, wait */
		if (output_buffer->write_busy >= cycle)
		{
			net_debug("pkt "
					"a=\"stall\" "
					"net=\"%s\" "
					"msg-->pkt=%lld:%d "
					"why=\"dst-busy\"\n",
					net->name,
					pkt->msg->id,
					pkt->session_id);
			net_trace("net.packet "
					"net=\"%s\" "
					"name=\"P-%lld:%d\" "
					"state=\"%s:%s:Dest_buffer_busy\" "
					"stg=\"DBB\"\n",
					net->name, pkt->msg->id,
					pkt->session_id,
					node->name,
					buffer->name);

			esim_schedule_event(event, stack,
					output_buffer->write_busy - cycle + 1);
			return;
		}

		/* If destination output buffer is full, wait */
		if (pkt->size > output_buffer->size)
			fatal("%s: packet does not fit in buffer.\n%s",
					net->name, net_err_large_message);


		if (output_buffer->count + pkt->size > output_buffer->size)
		{
			net_debug("pkt "
					"a=\"stall\" "
					"net=\"%s\" "
					"msg-->pkt=%lld:%d "
					"why=\"dst-full\"\n",
					net->name,
					pkt->msg->id,
					pkt->session_id);

			net_trace("net.packet "
					"net=\"%s\" "
					"name=\"P-%lld:%d\" "
					"state=\"%s:%s:Dest_buffer_full\" "
					"stg=\"DBF\"\n",
					net->name, pkt->msg->id,
					pkt->session_id,
					node->name,
					buffer->name);


			net_buffer_wait(output_buffer, event, stack);
			return;
		}

		/* If scheduler says that it is not our turn, try later */
		if (net_node_schedule(node, output_buffer) != buffer)
		{
			net_debug("pkt "
					"a=\"stall\" "
					"net=\"%s\" "
					"msg-->pkt=%lld:%d "
					"why=\"sched\"\n",
					net->name,
					pkt->msg->id,
					pkt->session_id);

			net_trace("net.packet "
					"net=\"%s\" "
					"name=\"P-%lld:%d\" "
					"state=\"%s:%s:switch_arbit_fail\" "
					"stg=\"SA\"\n",
					net->name, pkt->msg->id,
					pkt->session_id,
					node->name,
					buffer->name);

			esim_schedule_event(event, stack, 1);
			return;
		}

		/* Calculate latency and occupy resources */
		assert(node->kind != net_node_end);
		assert(node->bandwidth > 0);
		lat = (pkt->size - 1) / node->bandwidth + 1;
		assert(lat > 0);
		buffer->read_busy = cycle + lat - 1;
		output_buffer->write_busy = cycle + lat - 1;

		/* Transfer message to next output buffer */
		assert(pkt->busy < cycle);
		net_buffer_extract(buffer, pkt);
		net_buffer_insert(output_buffer, pkt);
		pkt->buffer = output_buffer;
		pkt->busy = cycle + lat - 1;

		/* Schedule next event */
		esim_schedule_event(EV_NET_OUTPUT_BUFFER, stack, lat);
	}

	else if (event == EV_NET_RECEIVE)
	{
		assert (pkt);
		struct net_msg_t *msg = pkt->msg;
		/* Debug */
		net_debug("pkt "
				"a=\"receive\" "
				"net=\"%s\" "
				"msg-->pkt=%lld:%d "
				"node=\"%s\"\n",
				net->name,
				pkt->msg->id,
				pkt->session_id,
				dst_node->name);

		if (net_depacketizer(net, node, pkt) == 1)
		{
			if (pkt->msg->packet_list_count > 1)
				net_trace("net.msg net=\"%s\" name=\"M-%lld\" "
						"state=\"%s:depacketize\"\n",
						net->name, msg->id, node->name);

			if (stack->ret_event == ESIM_EV_NONE)
			{
				assert (msg);
				net_receive(net, node, msg);

			}
			/* Finish */
			net_stack_return(stack);
		}
		else
			/* Freeing packet stack, not the message */
			free(stack);
	}

	else
	{
		panic("%s: unknown event", __FUNCTION__);
	}
}

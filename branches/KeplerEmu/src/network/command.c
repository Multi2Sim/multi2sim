/*
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
#include <lib/util/string.h>
#include <lib/mhandle/mhandle.h>
#include <lib/util/list.h>
#include <lib/util/debug.h>

#include "net-system.h"
#include "command.h"
#include "network.h"
#include "node.h"
#include "buffer.h"

int EV_NET_COMMAND;
int EV_NET_COMMAND_RCV;
static void net_command_expect(struct list_t *token_list, char *command_line)
{
	if (!list_count(token_list))
		fatal("%s: unexpected end of line.\n\t> %s",
				__FUNCTION__, command_line);
}

static void net_command_get_string(struct list_t *token_list,
		char *command_line, char *buf, int size)
{
	net_command_expect(token_list, command_line);
	snprintf(buf, size, "%s", str_token_list_first(token_list));
	str_token_list_shift(token_list);
}

static long long net_command_get_llint(struct list_t *token_list,
		char *command_line, char *expected)
{
	int err;
	long long command_cycle;

	/* Read cycle */
	net_command_expect(token_list, command_line);
	command_cycle = str_to_llint(str_token_list_first(token_list), &err);
	if (err || command_cycle < 1)
		fatal("%s: %s: invalid %s , integer >= 1 expected.\n\t> %s",
				__FUNCTION__, str_token_list_first(token_list),
				command_line, expected);

	/* Shift token and return */
	str_token_list_shift(token_list);
	return command_cycle;
}

static long long net_command_get_def(struct list_t *token_list, char *command_line,
		char *expected, long long def)
{
	if (!list_count(token_list))
	{
		return def;
	}
	else
		return net_command_get_llint(token_list, command_line,
				expected);

}


static struct net_node_t * net_command_get_node(struct net_t *net,
		struct list_t *token_list, char *command_line,
		enum net_node_kind_t expected_kind)
{
	struct net_node_t *node;
	char *node_name;

	/* Getting the end node name */
	net_command_expect(token_list, command_line);
	node_name = str_token_list_first(token_list);

	node = net_get_node_by_name(net, node_name);

	if (!node)
	{
		fatal("%s: %s: invalid node name.\n\t> %s",
				__FUNCTION__, node_name, command_line);
	}

	else if (expected_kind != net_node_invalid && node->kind != expected_kind)
	{
		fatal("%s: %s: Unexpected node type \n\t > %s",
				__FUNCTION__, node_name, command_line);
	}

	/* Return module */
	str_token_list_shift(token_list);
	return node;
}

static struct net_buffer_t * net_command_get_buffer(struct net_node_t * node,
		struct list_t *token_list,char *command_line)
{
	struct net_buffer_t *buffer;
	char *buffer_name;

	net_command_expect(token_list, command_line);
	buffer_name = str_token_list_first(token_list);

	buffer = net_get_buffer_by_name(node, buffer_name);
	if (!buffer)
	{
		fatal("%s: %s: invalid buffer name in node %s.\n\t> %s",
			__FUNCTION__, buffer_name, node->name,
			command_line);
	}
	return buffer;

}


void net_command_handler(int event, void *data)
{

	struct net_stack_t *stack= data;
	struct net_t *net = stack->net;

	char out_msg[MAX_STRING_SIZE];
	char msg_detail[MAX_STRING_SIZE];

	char *msg_str = out_msg;
	int msg_size = sizeof out_msg;

	char *msg_detail_str = msg_detail;
	int msg_detail_size = sizeof msg_detail;

	int test_failed;

	test_failed = 0;
	*msg_str = '\0';
	*msg_detail_str = '\0';

	long long cycle = esim_domain_cycle(net_domain_index);

	if (event == EV_NET_COMMAND)
	{
		char *command_line = stack->command;
		char command[MAX_STRING_SIZE];

		struct list_t *token_list;

		/* Split command in tokens, skip command */
		token_list = str_token_list_create(command_line, " ");
		assert(list_count(token_list));

		long long command_cycle = net_command_get_llint(token_list,
				command_line,"cycle value");

		if (command_cycle > cycle)
		{
			str_token_list_free(token_list);
			esim_schedule_event(event, stack, command_cycle - cycle);
			return;
		}

		net_command_get_string(token_list, command_line, command,
				sizeof command );

		if (!strcasecmp(command, "Send"))
		{
			int msg_size;
			long long int msg_id;

			struct net_node_t *src_node;
			struct net_node_t *dst_node;


			/* Getting the Source Node from the command */
			src_node = net_command_get_node(net,token_list,
					command_line, net_node_end);

			/* Getting the Source Node from the command */
			dst_node = net_command_get_node(net, token_list,
					command_line, net_node_end);

			/* Getting the Message Size; or Default values */
			msg_size = (int) net_command_get_def(token_list,
					command_line,"Message size value",
					(long long) net_msg_size);

			/* Getting the Message ID; or default value */
			msg_id = net_command_get_def(token_list, command_line,
					"Message id value",
					net->msg_id_counter + 1);

			if (msg_id != net->msg_id_counter + 1)
			{
				fatal("%s: The message id is out of order. "
					"\n\t\"You are out o' order. "
					"The whole Trial is out o' order \" \n"
					"\tBear with us. We are being "
					"cute here \n\t> %s",
					__FUNCTION__, command_line);
			}

			stack->msg = net_send_ev(net, src_node, dst_node, msg_size,
					EV_NET_COMMAND_RCV, stack);
			fprintf(stderr, "\n Message %lld sent at %lld \n\n", msg_id,
					cycle);

		}

		else if (!strcasecmp(command, "Receive"))
		{
			long long msg_id;
			struct net_node_t *dst_node;
			struct net_msg_t *msg;

			dst_node = net_command_get_node(net, token_list,
					command_line, net_node_end);

			msg_id = net_command_get_llint(token_list, command_line,
					"Message id value");

			/* Output */
			str_printf(&msg_str, &msg_size,
				"CHECK: Cycle %lld: Receive in node %s for message"
				" %lld ",cycle, dst_node->name, msg_id);

			msg = net_msg_table_get(net, msg_id);
			if (!msg)
			{
				test_failed = 1;
				str_printf(&msg_detail_str, &msg_detail_size,
					"\t Message is either invalid, "
					"not yet send or already received\n");
			}

			/* Checking : the node indicated in the receive command
			 * is the destination node for the message*/
			else
			{
				if (dst_node != msg->dst_node)
					warning("%s: The node %s in the receive "
						"command is \n\tnot intended node "
						"%s in the send command\n > %s"
						,__FUNCTION__, dst_node->name,
						msg->dst_node->name, command_line);

				if (msg->node != dst_node)
				{
					test_failed = 1;
					str_printf(&msg_detail_str, &msg_detail_size,
						"\t Message expected to be "
						"in node %s, but found in %s\n",
						dst_node->name,	msg->node->name);
				}

				else if (msg != list_get(msg->buffer->msg_list, 0))
				{
					test_failed = 1;
					str_printf(&msg_detail_str, &msg_detail_size,
						"\tMessage expected to be "
						"ready for receive in node %s \n\tbut "
						"not in the buffer head\n",
						dst_node->name);
				}
			}

			/* Output */
			fprintf(stderr, ">>> %s - %s\n", out_msg, test_failed ?
				"failed" : "passed");
			fprintf(stderr, "%s", msg_detail);
			net_stack_return(stack);

		}

		else if (!strcasecmp(command, "InBufferCheck"))
		{
			long long int msg_id;
			struct net_msg_t * msg;
			struct net_node_t * node;
			struct net_buffer_t *buffer;

			node = net_command_get_node(net, token_list,
					command_line, 0);

			msg_id = net_command_get_llint(token_list, command_line,
					"invalid message id");
			/* Output */
			str_printf(&msg_str, &msg_size,
				"CHECK: Cycle %lld: message %lld is in input buffer of "
				"node %s", cycle, msg_id, node->name);


			msg = net_msg_table_get(net, msg_id);
			if (!msg)
			{
				test_failed = 1;
				str_printf(&msg_detail_str, &msg_detail_size,
					"\t Message is either invalid, "
					"not yet send or already received\n");
			}
			else
			{
				buffer = msg->buffer;
				if (msg->node != node)
				{
					test_failed = 1;
					str_printf(&msg_detail_str, &msg_detail_size,
						"\tMessage expected to be in node %s"
						"but found in node %s \n",
						node->name, msg->node->name);
				}
				else if (buffer != list_get(node->input_buffer_list,
						buffer->index))
				{
					test_failed = 1;
					str_printf(&msg_detail_str, &msg_detail_size,
						"\t Message is not in any of input "
						"buffers of the node %s\n",
						node->name);
				}
			}
			/* Output */
			fprintf(stderr, ">>> %s - %s\n", out_msg, test_failed ?
				"failed" : "passed");
			fprintf(stderr, "%s", msg_detail);
			net_stack_return(stack);
		}

		else if (!strcasecmp(command, "OutBufferCheck"))
		{
			long long int msg_id;
			struct net_msg_t * msg;
			struct net_node_t * node;
			struct net_buffer_t *buffer;

			node = net_command_get_node(net, token_list,
					command_line, 0);

			msg_id = net_command_get_llint(token_list, command_line,
					"invalid message id");
			/* Output */
			str_printf(&msg_str, &msg_size,
				"CHECK: Cycle %lld: message %lld is in one of output buffers"
				" of node %s", cycle, msg_id, node->name);


			msg = net_msg_table_get(net, msg_id);
			if (!msg)
			{
				test_failed = 1;
				str_printf(&msg_detail_str, &msg_detail_size,
					"\t Message is either invalid, "
					"not yet send or already received\n");
			}
			else
			{
				buffer = msg->buffer;
				if (msg->node != node)
				{
					test_failed = 1;
					str_printf(&msg_detail_str, &msg_detail_size,
						"\tMessage expected to be in node %s"
						"but found in node %s\n",
						node->name, msg->node->name);
				}
				else if (buffer != list_get(node->output_buffer_list,
						buffer->index))
				{
					test_failed = 1;
					str_printf(&msg_detail_str, &msg_detail_size,
						"\t Message is not in any of output"
						"buffers of the node %s\n", node->name);
				}
			}
			/* Output */
			fprintf(stderr, ">>> %s - %s\n", out_msg, test_failed ?
				"failed" : "passed");
			fprintf(stderr, "%s", msg_detail);
			net_stack_return(stack);		}

		else if (!strcasecmp(command, "NodeCheck"))
		{

			long long int msg_id;
			struct net_msg_t * msg;
			struct net_node_t * node;

			node = net_command_get_node(net, token_list,
					command_line, 0);

			msg_id = net_command_get_llint(token_list, command_line,
					"invalid message id");
			/* Output */
			str_printf(&msg_str, &msg_size,
				"CHECK: Cycle %lld: message %lld is in node %s", cycle,
				msg_id, node->name);


			msg = net_msg_table_get(net, msg_id);
			if (!msg)
			{
				test_failed = 1;
				str_printf(&msg_detail_str, &msg_detail_size,
					"\t Message is either invalid, "
					"not yet send or already received\n");
			}
			else
			{
				if (msg->node != node)
				{
					test_failed = 1;
					str_printf(&msg_detail_str, &msg_detail_size,
						"\tMessage is not in the node %s\n",
						node->name);
				}
			}
			/* Output */
			fprintf(stderr, ">>> %s - %s\n", out_msg, test_failed ?
				"failed" : "passed");
			fprintf(stderr, "%s", msg_detail);
			net_stack_return(stack);
		}

		else if (!strcasecmp(command, "ExactPosCheck"))
		{
			long long int msg_id;
			struct net_msg_t * msg;
			struct net_node_t * node;
			struct net_buffer_t *buffer;

			node = net_command_get_node(net, token_list,
				command_line, 0);

			buffer = net_command_get_buffer(node, token_list,
					command_line);

			msg_id = net_command_get_llint(token_list, command_line,
				"invalid message id");
			/* Output */
			str_printf(&msg_str, &msg_size,
				"CHECK: Cycle %lld: message %lld is in buffer %s"
				" of node %s", cycle, msg_id, buffer->name, node->name);

			/* Checks */
			msg = net_msg_table_get(net, msg_id);
			if (!msg)
			{
				test_failed = 1;
				str_printf(&msg_detail_str, &msg_detail_size,
					"\t Message is either invalid, "
					"not yet send or already received\n");
			}
			else
			{
				if (msg->buffer != buffer || msg->node != node)
				{
					test_failed = 1;
					str_printf(&msg_detail_str, &msg_detail_size,
						"Message expected to be in buffer %s"
						"(node %s) \n\tbut found in buffer %s"
						"(node %s)\n",buffer->name, node->name,
						msg->buffer->name, msg->node->name);
				}
			}
			/* Output */
			fprintf(stderr, ">>> %s - %s\n", out_msg, test_failed ?
				"failed" : "passed");
			fprintf(stderr, "%s", msg_detail);
			net_stack_return(stack);
		}
		else
			fatal("%s: %s: invalid command.\n\t> %s",
					__FUNCTION__, command, command_line);
		free(command_line);
		str_token_list_free(token_list);
	}

	else if (event == EV_NET_COMMAND_RCV)
	{
		struct net_msg_t *msg;
		struct net_node_t *dst_node;
		msg = stack->msg;
		dst_node = msg->dst_node;

		assert(dst_node == msg->node);

		fprintf(stderr, "\n Message %lld received at %lld \n\n", msg->id, cycle);
		net_receive(net, dst_node, msg);
		net_stack_return(stack);
	}
}

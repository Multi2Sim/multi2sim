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

#ifndef NET_COMMAND_H
#define NET_COMMAND_H

/*
 * Event to handle network commands that injects message to the network,
 * checks them and receives them.
 */

extern int EV_NET_COMMAND;
extern int EV_NET_COMMAND_RCV;
extern int EV_NET_COMMAND_SEND;

/* Stack */
struct net_command_stack_t
{
	/* Local variables */
	struct net_t *net;
	struct net_msg_t *msg;
	char *command;

	/* Return event */
	int ret_event;
	struct net_command_stack_t *ret_stack;
};


struct net_command_stack_t *net_command_stack_create(struct net_t *net,
	int retevent, void *retstack);
void net_command_stack_return(struct net_command_stack_t *stack);


void net_command_handler(int event, void *data);


#endif


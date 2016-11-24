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
#include <gtk/gtk.h>
#include <stdio.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>
#include <visual/common/cycle-bar.h>
#include <visual/common/state.h>
#include <visual/common/trace.h>
#include <visual/memory/mod.h>

#include "buffer.h"
#include "link.h"
#include "net.h"
#include "net-message.h"
#include "net-packet.h"
#include "net-system.h"
#include "node.h"

/*
 * Public Functions
 */

struct vi_net_system_t *vi_net_system;

static char *err_vi_net_system_trace_version =
		"\tThe network system trace file has been created with an incompatible version\n"
		"\tof Multi2Sim. Please rerun the simulation with the same Multi2Sim\n"
		"\tversion used to visualize the trace.\n";

#define VI_NET_SYSTEM_TRACE_VERSION_MAJOR	1
#define VI_NET_SYSTEM_TRACE_VERSION_MINOR	10

static void vi_net_system_read_checkpoint  (struct vi_net_system_t *net_system, FILE *f);
static void vi_net_system_write_checkpoint (struct vi_net_system_t *net_system, FILE *f);

static void vi_net_system_new_message       (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line);
static void vi_net_system_msg_set_access    (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line);
static void vi_net_system_message           (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line);
static void vi_net_system_end_message	    (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line);
static void vi_net_system_link_transfer     (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line);
static void vi_net_system_new_packet	    (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line);
static void vi_net_system_packet_set_message(struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line);
static void vi_net_system_end_packet	    (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line);
static void vi_net_system_packet_extract    (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line);
static void vi_net_system_packet_insert     (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line);
static void vi_net_system_packet	    (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line);

void vi_net_system_init(void)
{
	struct vi_trace_line_t *trace_line;

	/* State file */
	vi_state_new_category("Network hierarchy",
			(vi_state_read_checkpoint_func_t) vi_net_system_read_checkpoint,
			(vi_state_write_checkpoint_func_t) vi_net_system_write_checkpoint,
			vi_net_system);

	/* Commands */
	vi_state_new_command("net.new_msg",
			(vi_state_process_trace_line_func_t) vi_net_system_new_message,
			vi_net_system);

	vi_state_new_command("net.msg_access",
			(vi_state_process_trace_line_func_t) vi_net_system_msg_set_access,
			vi_net_system);

	vi_state_new_command("net.msg",
			(vi_state_process_trace_line_func_t) vi_net_system_message,
			vi_net_system);

	vi_state_new_command("net.end_msg",
			(vi_state_process_trace_line_func_t) vi_net_system_end_message,
			vi_net_system);

	vi_state_new_command("net.link_transfer",
			(vi_state_process_trace_line_func_t) vi_net_system_link_transfer,
			vi_net_system);

	vi_state_new_command("net.new_packet",
			(vi_state_process_trace_line_func_t) vi_net_system_new_packet,
			vi_net_system);

	vi_state_new_command("net.packet_msg",
			(vi_state_process_trace_line_func_t) vi_net_system_packet_set_message,
			vi_net_system);

	vi_state_new_command("net.packet_extract",
			(vi_state_process_trace_line_func_t) vi_net_system_packet_extract,
			vi_net_system);

	vi_state_new_command("net.packet_insert",
			(vi_state_process_trace_line_func_t) vi_net_system_packet_insert,
			vi_net_system);
	vi_state_new_command("net.end_packet",
			(vi_state_process_trace_line_func_t) vi_net_system_end_packet,
			vi_net_system);

	vi_state_new_command("net.packet",
			(vi_state_process_trace_line_func_t) vi_net_system_packet,
			vi_net_system);



	/* Initialize */
	vi_net_system = xcalloc(1, sizeof(struct vi_net_system_t));
	vi_net_system->net_table = hash_table_create(0, FALSE);
	vi_net_system->level_list = list_create();

	/* Parse header in state file */
	VI_STATE_FOR_EACH_HEADER(trace_line)
	{
		char *command;

		/* Get command */
		command = vi_trace_line_get_command(trace_line);
		assert(strcmp(command, "c"));

		if (!strcmp(command, "net.init"))
		{
			char *version;

			int version_major = 0;
			int version_minor = 0;

			vi_net_system->active = 1;

			/* Check version compatibility */
			version = vi_trace_line_get_symbol(trace_line, "version");
			if (version)
				sscanf(version, "%d.%d", &version_major, &version_minor);
			if (version_major != VI_NET_SYSTEM_TRACE_VERSION_MAJOR ||
					version_minor > VI_NET_SYSTEM_TRACE_VERSION_MINOR)
				fatal("incompatible network system trace version.\n"
						"\tTrace generation v. %d.%d / Trace consumer v. %d.%d\n%s",
						version_major, version_minor, VI_NET_SYSTEM_TRACE_VERSION_MAJOR,
						VI_NET_SYSTEM_TRACE_VERSION_MINOR, err_vi_net_system_trace_version);
		}
		else if (!strcmp(command, "net.create"))
		{
			struct vi_net_t *net;

			/* Create module */
			net = vi_net_create(trace_line);

			/* Add to network system hash table of networks */
			hash_table_insert(vi_net_system->net_table, net->name, net);

		}
		else if (!strcmp(command, "net.node"))
		{
			struct vi_net_node_t *node;

			node = vi_net_node_assign(trace_line);
			assert(node);
		}
		else if (!strcmp(command, "net.link"))
		{
			struct vi_net_link_t *link;

			link = vi_net_link_create(trace_line);
			assert(link);
		}

		else if (!strcmp(command, "net.input_buffer"))
		{
			struct vi_net_buffer_t *buffer;

			buffer = vi_net_buffer_create(trace_line, vi_buffer_dir_input);
			assert(buffer);
		}
		else if (!strcmp(command, "net.output_buffer"))
		{
			struct vi_net_buffer_t *buffer;

			buffer = vi_net_buffer_create(trace_line, vi_buffer_dir_output);
			assert(buffer);
		}
	}
}

void vi_net_system_done(void)
{
	int i;
	struct vi_net_t *net;

	char *net_name;
	/* Free networks */
	HASH_TABLE_FOR_EACH(vi_net_system->net_table, net_name, net)
	{
		vi_net_free(net);
	}
	hash_table_free(vi_net_system->net_table);

	/* Free levels */
	LIST_FOR_EACH(vi_net_system->level_list, i)
	{
		struct list_t *list = list_get(vi_net_system->level_list, i);
		if (list)
			list_free(list);
	}

	list_free(vi_net_system->level_list);

	/* Rest */
	free(vi_net_system);
}

void vi_net_system_level_assign(void)
{
	int mod_level;
	char *net_name;
	struct vi_net_t *net;

	HASH_TABLE_FOR_EACH(vi_net_system->net_table, net_name, net)
	{
		struct list_t *net_level;
		struct vi_mod_t *mod;

		hash_table_find_first(net->high_mods,(void **) &(mod));

		/* To make sure you are not dependent to mods */
		if (mod)
			mod_level = mod->level;
		else
			mod_level = 1;

		while (vi_net_system->level_list->count < mod_level)
			list_add(vi_net_system->level_list, list_create());
		net_level = list_get(vi_net_system->level_list, mod_level - 1);
		list_add(net_level, net);
	}
}

void vi_net_system_coordination_config(void)
{
	char *net_name;
	struct vi_net_t *net;

	HASH_TABLE_FOR_EACH(vi_net_system->net_table, net_name, net)
	{
		vi_net_graph_visual_calculation(net);
	}
}


static void vi_net_system_read_checkpoint(struct vi_net_system_t *net_system, FILE *f)
{
	char net_name[MAX_STRING_SIZE];

	struct vi_net_t *net;

	int i;

	/* Read modules */
	for (i = 0; i < hash_table_count(vi_net_system->net_table); i++)
	{
		/* Get module */
		str_read_from_file(f, net_name, sizeof net_name);
		net = hash_table_get(vi_net_system->net_table, net_name);
		if (!net)
			panic("%s: %s: invalid module name", __FUNCTION__, net_name);

		/* Read module checkpoint */
		vi_net_read_checkpoint(net, f);
	}
}
static void vi_net_system_write_checkpoint(struct vi_net_system_t *net_system, FILE *f)
{
	struct vi_net_t *net;

	char *net_name;

	/* Write modules */
	HASH_TABLE_FOR_EACH(vi_net_system->net_table, net_name,net)
	{
		str_write_to_file(f, net->name);
		vi_net_write_checkpoint(net, f);
	}
}

static void vi_net_system_new_message(struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line)
{
	char *net_name;
	char *name;
	char *state;

	struct vi_net_t *net;
	struct vi_net_message_t *message;

	unsigned int size;

	/* Read fields */
	net_name =  vi_trace_line_get_symbol(trace_line, "net");
	net = hash_table_get(vi_net_system->net_table, net_name);
	if (!net)
		panic("%s: invalid network name '%s'", __FUNCTION__, net_name);

	name = vi_trace_line_get_symbol(trace_line, "name");
	state = vi_trace_line_get_symbol(trace_line, "state");
	size = vi_trace_line_get_symbol_int(trace_line, "size");

	/* Create new access */
	message = vi_net_message_create(net->name, name, size);
	vi_net_message_set_state(message, state);

	/* Add access to list */
	hash_table_insert(net->message_table, message->name, message);
}

static void vi_net_system_new_packet(struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line)
{
	char *net_name;
	char *name;
	char *state;

	struct vi_net_t *net;
	struct vi_net_packet_t *packet;

	unsigned int size;

	/* Read fields */
	net_name =  vi_trace_line_get_symbol(trace_line, "net");
	net = hash_table_get(vi_net_system->net_table, net_name);
	if (!net)
		panic("%s: invalid network name '%s'", __FUNCTION__, net_name);

	name = vi_trace_line_get_symbol(trace_line, "name");
	state = vi_trace_line_get_symbol(trace_line, "state");
	size = vi_trace_line_get_symbol_int(trace_line, "size");

	/* Create new access */
	packet = vi_net_packet_create(net->name, name, size);
	vi_net_packet_set_state(packet , state);

	/* Add access to list */
	hash_table_insert(net->packet_table, packet->name, packet);
}

static void vi_net_system_msg_set_access(struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line)
{
	char *net_name;
	char *message_name;
	char *access;

	struct vi_net_t *net;
	struct vi_net_message_t *message;

	/* Read fields */
	net_name =  vi_trace_line_get_symbol(trace_line, "net");
	net = hash_table_get(vi_net_system->net_table, net_name);
	if (!net)
		panic("%s: invalid network name '%s'", __FUNCTION__, net_name);

	message_name = vi_trace_line_get_symbol(trace_line, "name");
	access = vi_trace_line_get_symbol(trace_line, "access");

	/* Create new access */
	message = hash_table_get(net->message_table, message_name);
	if (!message)
		panic("%s: invalid message name '%s'", __FUNCTION__, net_name);

	message->access_name = str_set(message->access_name, access);
}

static void vi_net_system_packet_set_message(struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line)
{
	char *net_name;
	char *packet_name;
	char *message_name;

	struct vi_net_t *net;
	struct vi_net_packet_t *packet;
	struct vi_net_message_t *message;

	/* Read fields */
	net_name =  vi_trace_line_get_symbol(trace_line, "net");
	net = hash_table_get(vi_net_system->net_table, net_name);
	if (!net)
		panic("%s: invalid network name '%s'", __FUNCTION__, net_name);

	message_name = vi_trace_line_get_symbol(trace_line, "message");
	message = hash_table_get(net->message_table, message_name);
	if (!message)
		panic("%s: invalid message: wrong association of packet and message '%s'", __FUNCTION__, message_name);

	packet_name = vi_trace_line_get_symbol(trace_line, "name");

	/* Create new access */
	packet = hash_table_get(net->packet_table, packet_name);
	if (!packet)
		panic("%s: invalid message name '%s'", __FUNCTION__, net_name);

	packet->message_name = str_set(packet->message_name, message_name);
}
static void vi_net_system_message(struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line)
{
	struct vi_net_t *net;
	struct vi_net_message_t *message;

	char *net_name;
	char *name;
	char *state;

	/* Read fields */
	net_name =  vi_trace_line_get_symbol(trace_line, "net");
	net = hash_table_get(vi_net_system->net_table, net_name);
	if (!net)
		panic("%s: invalid network name '%s'", __FUNCTION__, net_name);

	name = vi_trace_line_get_symbol(trace_line, "name");
	state = vi_trace_line_get_symbol(trace_line, "state");

	/* Find access */
	message = hash_table_get(net->message_table, name);
	if (!message)
		panic("%s: %s: access not found", __FUNCTION__, name);

	/* Update access */
	vi_net_message_set_state(message, state);
}

static void vi_net_system_packet(struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line)
{
	struct vi_net_t *net;
	struct vi_net_packet_t *packet;

	char *net_name;
	char *name;
	char *state;
	enum vi_net_packet_stage stage;

	/* Read fields */
	net_name =  vi_trace_line_get_symbol(trace_line, "net");
	net = hash_table_get(vi_net_system->net_table, net_name);
	if (!net)
		panic("%s: invalid network name '%s'", __FUNCTION__, net_name);

	name = vi_trace_line_get_symbol(trace_line, "name");
	state = vi_trace_line_get_symbol(trace_line, "state");

	/* Find access */
	packet = hash_table_get(net->packet_table, name);
	if (!packet)
		panic("%s: %s: access not found", __FUNCTION__, name);

	stage = str_map_string(&vi_packet_stage_map, vi_trace_line_get_symbol(trace_line, "stg"));
	packet->stage = stage;

	/* Update access */
	vi_net_packet_set_state(packet, state);
}

static void vi_net_system_end_message (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line)
{
	struct vi_net_message_t *message;
	struct vi_net_t *net;

	char *name;
	char *net_name;

	/* Read fields */
	net_name =  vi_trace_line_get_symbol(trace_line, "net");
	net = hash_table_get(vi_net_system->net_table, net_name);
	if (!net)
		panic("%s: invalid network name '%s'", __FUNCTION__, net_name);

	name = vi_trace_line_get_symbol(trace_line, "name");

	/* Find access */
	message = hash_table_remove(net->message_table, name);
	if (!message)
		panic("%s: message not found", __FUNCTION__);

	/* Free access */
	vi_net_message_free(message);
}

static void vi_net_system_end_packet (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line)
{
	struct vi_net_packet_t *packet;
	struct vi_net_t *net;

	char *name;
	char *net_name;

	/* Read fields */
	net_name =  vi_trace_line_get_symbol(trace_line, "net");
	net = hash_table_get(vi_net_system->net_table, net_name);
	if (!net)
		panic("%s: invalid network name '%s'", __FUNCTION__, net_name);

	name = vi_trace_line_get_symbol(trace_line, "name");

	/* Find access */
	packet = hash_table_remove(net->packet_table, name);
	if (!packet)
		panic("%s: packet not found", __FUNCTION__);

	/* Free access */
	vi_net_packet_free(packet);
}

static void vi_net_system_link_transfer(struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line)
{
	struct vi_net_t *net;
	struct vi_net_link_t *link;

	char *net_name;
	char *link_name;

	int transferred_bytes;
	int last_packet_size;
	long long busy_cycle;

	/* Read Fields */
	net_name =  vi_trace_line_get_symbol(trace_line, "net");
	net = hash_table_get(vi_net_system->net_table, net_name);
	if (!net)
		panic("%s: invalid network name '%s'", __FUNCTION__, net_name);

	link_name = vi_trace_line_get_symbol(trace_line, "link");

	link = hash_table_get(net->link_table, link_name);
	if (!link)
		panic("%s: Link not found", __FUNCTION__);

	transferred_bytes = vi_trace_line_get_symbol_int(trace_line, "transB");

	last_packet_size = vi_trace_line_get_symbol_int(trace_line, "last_size");

	busy_cycle = vi_trace_line_get_symbol_int(trace_line, "busy");

	link->transferred_bytes = transferred_bytes;
	link->last_packet_size = last_packet_size;
	link->busy_cycle = busy_cycle;
}

static void vi_net_system_packet_insert (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line)
{
	struct vi_net_t *net;
	char *net_name;

	struct vi_net_node_t *node;
	char *node_name;

	char* buffer_name;

	int buffer_occupancy;

	struct vi_net_packet_t *packet;
	char *packet_name;

	net_name =  vi_trace_line_get_symbol(trace_line, "net");
	net = hash_table_get(vi_net_system->net_table, net_name);
	if (!net)
		panic("%s: invalid network name '%s'", __FUNCTION__, net_name);

	node_name = vi_trace_line_get_symbol(trace_line, "node");
	node = hash_table_get(net->node_table, node_name);
	if (!net)
		panic("%s: invalid node name '%s'", __FUNCTION__, node_name);

	packet_name = vi_trace_line_get_symbol(trace_line, "name");
	packet = hash_table_get(net->packet_table, packet_name);
	if (!packet)
		panic("%s: invalid packet name '%s'", __FUNCTION__, packet_name);

	buffer_name = vi_trace_line_get_symbol(trace_line, "buffer");
	buffer_occupancy = vi_trace_line_get_symbol_int(trace_line, "occpncy");

	vi_node_insert_packet(node, buffer_name, buffer_occupancy, packet);
}

static void vi_net_system_packet_extract (struct vi_net_system_t *net_system, struct vi_trace_line_t *trace_line)
{
	struct vi_net_t *net;
	char *net_name;

	struct vi_net_node_t *node;
	char *node_name;

	char* buffer_name;

	int buffer_occupancy;

	struct vi_net_packet_t *packet;
	char *packet_name;

	net_name =  vi_trace_line_get_symbol(trace_line, "net");
	net = hash_table_get(vi_net_system->net_table, net_name);
	if (!net)
		panic("%s: invalid network name '%s'", __FUNCTION__, net_name);

	node_name = vi_trace_line_get_symbol(trace_line, "node");
	node = hash_table_get(net->node_table, node_name);
	if (!net)
		panic("%s: invalid node name '%s'", __FUNCTION__, node_name);

	packet_name = vi_trace_line_get_symbol(trace_line, "name");
	packet = hash_table_get(net->packet_table, packet_name);
	if (!packet)
		panic("%s: invalid packet name '%s'", __FUNCTION__, packet_name);

	buffer_name = vi_trace_line_get_symbol(trace_line, "buffer");
	buffer_occupancy = vi_trace_line_get_symbol_int(trace_line, "occpncy");

	vi_node_extract_packet(node, buffer_name, buffer_occupancy, packet);
}

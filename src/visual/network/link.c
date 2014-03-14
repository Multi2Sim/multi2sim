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
#include <stdio.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/hash-table.h>
#include <lib/util/misc.h>
#include <lib/mhandle/mhandle.h>
#include <visual/common/state.h>
#include <visual/common/trace.h>


#include "link.h"
#include "net.h"
#include "net-system.h"

struct vi_net_link_t *vi_net_link_create(struct vi_trace_line_t *trace_line)
{
	struct vi_net_link_t *link;

	/* Allocate link */
	link = xcalloc(1, sizeof(struct vi_net_link_t));

	/* Get the corresponding network */
	struct vi_net_t *net;
	char *net_name;
	net_name = vi_trace_line_get_symbol(trace_line, "net_name");
	net = hash_table_get(vi_net_system->net_table, net_name);

	assert(net);

	/* Assign link name */
	char *link_name;
	link_name = vi_trace_line_get_symbol(trace_line, "link_name");
	link->name = xstrdup(link_name);

	/* Get Source node and Destination node of Link */
	char *src_node_name;
	char *dst_node_name;
	struct vi_net_node_t *src, *dst;

	src_node_name = vi_trace_line_get_symbol(trace_line, "src_node");
	src = hash_table_get(net->node_table, src_node_name);

	dst_node_name = vi_trace_line_get_symbol(trace_line, "dst_node");
	dst = hash_table_get(net->node_table, dst_node_name);

	assert(src && dst && (src != dst));

	link->src_node = src;
	link->dst_node = dst;

	/* Get number of Virtual Channels */
	int vc_num;
	vc_num = vi_trace_line_get_symbol_int(trace_line, "vc_num");
	link->vc_number = vc_num;

	hash_table_insert(net->link_table,link->name, link);

	/* Return Link */
	return link;
}

void vi_net_link_free(struct vi_net_link_t *link)
{
	if (link->name)
		free(link->name);
	free(link);
}

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

#ifndef VISUAL_NETWORK_LINK_H
#define VISUAL_NETWORK_LINK_H

struct vi_net_link_t
{
	int   vc_number;
	char *name;

	struct vi_net_node_t *src_node;
	struct vi_net_node_t *dst_node;
};


struct vi_net_link_t *vi_net_link_create  (struct vi_trace_line_t * trace_line);
void                  vi_net_link_free    (struct vi_net_link_t *link);

#endif

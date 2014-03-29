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

#ifndef VISUAL_NETWORK_NET_SYSTEM_H
#define VISUAL_NETWORK_NET_SYSTEM_H

struct hash_table_t;

struct vi_net_system_t
{
	int active;

	struct hash_table_t *net_table;
	struct list_t 	    *level_list;
};

extern struct vi_net_system_t *vi_net_system;

void vi_net_system_init(void);
void vi_net_system_level_assign(void);
void vi_net_system_coordination_config(void);
void vi_net_system_done(void);

#endif

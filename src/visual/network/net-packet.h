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


#ifndef VISUAL_NETWORK_PACKET_H
#define VISUAL_NETWORK_PACKET_H

enum vi_net_packet_stage
{
	vi_packet_dont_care = 0,
	vi_packet_dest_buf_busy,
	vi_packet_dest_buf_full,
	vi_packet_link_busy,
	vi_packet_bus_arbit,
	vi_packet_sw_arbit,
	vi_packet_vc_arbit,

	vi_packet_stage_count
};

struct vi_net_packet_t
{
        char *name;
        char *state;
        char *net_name;
        char *message_name;

        int size;
        /* An access is considered to be in a module as long as the access
         * is currently in any block of the module. For an access in a module,
         * this field gives the number of accesses (objects with same
         * 'name' field) located in different blocks of the module. */
        int num_links;

        long long creation_cycle;
        long long state_update_cycle;

        enum vi_net_packet_stage stage;
};

struct str_map_t vi_packet_stage_map;
struct str_map_t vi_packet_stage_color_map;

struct vi_net_packet_t *vi_net_packet_create(char *net_name, char *name, int size);
struct vi_net_packet_t *vi_net_packet_duplicate(struct vi_net_packet_t *packet);
void vi_net_packet_free(struct vi_net_packet_t *packet);

void vi_net_packet_set_state(struct vi_net_packet_t *packet, char *state);

void vi_net_packet_read_checkpoint(struct vi_net_packet_t *packet, FILE *f);
void vi_net_packet_write_checkpoint(struct vi_net_packet_t *packet, FILE *f);

void vi_net_packet_get_name_short(struct vi_net_packet_t *packet, char *buf, int size);
void vi_net_packet_get_desc(struct vi_net_packet_t *packet, char *buf, int size);


#endif

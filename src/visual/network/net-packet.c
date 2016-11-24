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

#include <lib/mhandle/mhandle.h>

#include <lib/util/debug.h>
#include <lib/util/string.h>
#include <visual/common/state.h>
#include "net-packet.h"

struct str_map_t vi_packet_stage_map =
{
	vi_packet_stage_count - 1,
	{
		{ "DC", vi_packet_dont_care },
		{ "DBB", vi_packet_dest_buf_busy },
		{ "DBF", vi_packet_dest_buf_full },
		{ "LB", vi_packet_link_busy },
		{ "BA", vi_packet_bus_arbit },
		{ "SA", vi_packet_sw_arbit },
		{ "VCA", vi_packet_vc_arbit }
	}
};

struct str_map_t vi_packet_stage_color_map =
{
	vi_packet_stage_count - 1,
	{
		{ "#FFFFFF", vi_packet_dont_care},
		{ "#CCFFCC", vi_packet_dest_buf_busy },
		{ "#CCFF99", vi_packet_dest_buf_full },
		{ "#CCFF66", vi_packet_link_busy },
		{ "#CCCC66", vi_packet_bus_arbit },
		{ "#CC9933", vi_packet_sw_arbit },
		{ "#FF0000", vi_packet_vc_arbit }
	}
};

void vi_net_packet_free(struct vi_net_packet_t *packet)
{
	if (packet->message_name)
		str_free(packet->message_name);
	if (packet->net_name)
		str_free(packet->net_name);
	if (packet->name)
	{
		str_free(packet->name);
	}
	if (packet->state)
		str_free(packet->state);
	free(packet);
}

struct vi_net_packet_t *vi_net_packet_create(char * net_name, char *name, int size)
{
	struct vi_net_packet_t *packet;

	/* Initialize */
	packet = xcalloc(1, sizeof(struct vi_net_packet_t));
	packet->name = str_set(packet->name, name);
	packet->net_name = str_set(packet->net_name, net_name);
	packet->size = size;
	packet->creation_cycle = vi_state_get_current_cycle();
	packet->stage = vi_packet_dont_care;

	/* Return */
	return packet;
}

void vi_net_packet_set_state(struct vi_net_packet_t *packet, char *state)
{
	packet->state = str_set(packet->state, state);
	packet->state_update_cycle = vi_state_get_current_cycle();
}

void vi_net_packet_write_checkpoint(struct vi_net_packet_t *packet, FILE *f)
{
	int count;

	/* Write name */
	str_write_to_file(f, packet->name);

	/* Write name */
	str_write_to_file(f, packet->net_name);

	/* Write state */
	str_write_to_file(f, packet->state);

	/* Write number of links */
	count = fwrite(&packet->num_links, 1, sizeof packet->num_links, f);
	if (count != sizeof packet->num_links)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write creation cycle */
	count = fwrite(&packet->creation_cycle, 1, sizeof packet->creation_cycle, f);
	if (count != sizeof packet->creation_cycle)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write state update cycle */
	count = fwrite(&packet->state_update_cycle, 1, sizeof packet->state_update_cycle, f);
	if (count != sizeof packet->state_update_cycle)
		panic("%s: cannot write checkpoint", __FUNCTION__);

	/* Write stage - state */
	count = fwrite(&packet->stage, 1, sizeof packet->stage, f);
	if (count != sizeof packet->stage)
		panic("%s: cannot write checkpoint", __FUNCTION__);

}

void vi_net_packet_read_checkpoint(struct vi_net_packet_t *packet, FILE *f)
{
	char name[MAX_STRING_SIZE];
	char net_name[MAX_STRING_SIZE];
	char state[MAX_STRING_SIZE];

	int count;

	/* Read message name */
	str_read_from_file(f, name, sizeof name);
	packet->name = str_set(packet->name, name);

	/* Read Network Name */
	str_read_from_file(f, net_name, sizeof net_name);
	packet->net_name = str_set(packet->net_name, net_name);

	/* Read state */
	str_read_from_file(f, state, sizeof state);
	packet->state = str_set(packet->state, state);

	/* Read number of links */
	count = fread(&packet->num_links, 1, sizeof packet->num_links, f);
	if (count != sizeof packet->num_links)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read creation cycle */
	count = fread(&packet->creation_cycle, 1, sizeof packet->creation_cycle, f);
	if (count != sizeof packet->creation_cycle)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read state update cycle */
	count = fread(&packet->state_update_cycle, 1, sizeof packet->state_update_cycle, f);
	if (count != sizeof packet->state_update_cycle)
		panic("%s: cannot read checkpoint", __FUNCTION__);

	/* Read state update cycle */
	count = fread(&packet->stage, 1, sizeof packet->stage, f);
	if (count != sizeof packet->stage)
		panic("%s: cannot read checkpoint", __FUNCTION__);

}

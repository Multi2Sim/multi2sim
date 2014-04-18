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

#include "net-message.h"

void vi_net_message_free(struct vi_net_message_t *message)
{
        str_free(message->name);
        str_free(message->state);
        free(message);
}

struct vi_net_message_t *vi_net_message_create(char *name)
{
        struct vi_net_message_t *message;

        /* Initialize */
        message = xcalloc(1, sizeof(struct vi_net_message_t));
        message->name = str_set(message->name, name);
        message->creation_cycle = vi_state_get_current_cycle();

        /* Return */
        return message;
}
void vi_net_message_read_checkpoint(struct vi_net_message_t *message, FILE *f)
{
        char name[MAX_STRING_SIZE];
        char state[MAX_STRING_SIZE];

        int count;

        /* Read message name */
        str_read_from_file(f, name, sizeof name);
        message->name = str_set(message->name, name);

        /* Read state */
        str_read_from_file(f, state, sizeof state);
        message->state = str_set(message->state, state);

        /* Read number of links */
        count = fread(&message->num_links, 1, sizeof message->num_links, f);
        if (count != sizeof message->num_links)
                panic("%s: cannot read checkpoint", __FUNCTION__);

        /* Read creation cycle */
        count = fread(&message->creation_cycle, 1, sizeof message->creation_cycle, f);
        if (count != sizeof message->creation_cycle)
                panic("%s: cannot read checkpoint", __FUNCTION__);

        /* Read state update cycle */
        count = fread(&message->state_update_cycle, 1, sizeof message->state_update_cycle, f);
        if (count != sizeof message->state_update_cycle)
                panic("%s: cannot read checkpoint", __FUNCTION__);
}

void vi_net_message_write_checkpoint(struct vi_net_message_t *message, FILE *f)
{
        int count;

        /* Write name */
        str_write_to_file(f, message->name);

        /* Write state */
        str_write_to_file(f, message->state);

        /* Write number of links */
        count = fwrite(&message->num_links, 1, sizeof message->num_links, f);
        if (count != sizeof message->num_links)
                panic("%s: cannot write checkpoint", __FUNCTION__);

        /* Write creation cycle */
        count = fwrite(&message->creation_cycle, 1, sizeof message->creation_cycle, f);
        if (count != sizeof message->creation_cycle)
                panic("%s: cannot write checkpoint", __FUNCTION__);

        /* Write state update cycle */
        count = fwrite(&message->state_update_cycle, 1, sizeof message->state_update_cycle, f);
        if (count != sizeof message->state_update_cycle)
                panic("%s: cannot write checkpoint", __FUNCTION__);
}


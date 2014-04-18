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


#ifndef VISUAL_NETWORK_MESSAGE_H
#define VISUAL_NETWORK_MESSAGE_H

struct vi_net_message_t
{
        char *name;
        char *state;

        /* An access is considered to be in a module as long as the access
         * is currently in any block of the module. For an access in a module,
         * this field gives the number of accesses (objects with same
         * 'name' field) located in different blocks of the module. */
        int num_links;

        long long creation_cycle;
        long long state_update_cycle;
};

struct vi_net_message_t *vi_net_message_create(char *name);
void vi_net_message_free(struct vi_net_message_t *message);


void vi_net_message_read_checkpoint(struct vi_net_message_t *message, FILE *f);
void vi_net_message_write_checkpoint(struct vi_net_message_t *message, FILE *f);


#endif

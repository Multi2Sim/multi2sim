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


#ifndef VISUAL_MEMORY_MOD_ACCESS_H
#define VISUAL_MEMORY_MOD_ACCESS_H


struct vi_mod_access_t
{
	char *name;
	char *state;

	unsigned int address;

	/* An access is considered to be in a module as long as the access
	 * is currently in any block of the module. For an access in a module,
	 * this field gives the number of accesses (objects with same
	 * 'name' field) located in different blocks of the module. */
	int num_links;

	long long creation_cycle;
	long long state_update_cycle;
};

struct vi_mod_access_t *vi_mod_access_create(char *name, unsigned int address);
void vi_mod_access_free(struct vi_mod_access_t *access);

void vi_mod_access_set_state(struct vi_mod_access_t *access, char *state);

void vi_mod_access_read_checkpoint(struct vi_mod_access_t *access, FILE *f);
void vi_mod_access_write_checkpoint(struct vi_mod_access_t *access, FILE *f);

void vi_mod_access_get_name_short(char *access_name, char *buf, int size);
void vi_mod_access_get_name_long(char *access_name, char *buf, int size);
void vi_mod_access_get_desc(char *access_name, char *buf, int size);


#endif


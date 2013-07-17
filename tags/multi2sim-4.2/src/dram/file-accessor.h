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

#ifndef DRAM_FILE_ACCESSOR_H
#define DRAM_FILE_ACCESSOR_H

#include <stdio.h>


/*
 * request_file_accessor
 */

struct request_file_accessor
{
	FILE *f;

	struct dram_request_t *cached_request;
	long long cached_request_cycle;
	long long request_id_counter;
};

struct request_file_accessor *request_file_accessor_create(void);
void request_file_accessor_free(struct request_file_accessor *accessor);

struct dram_request_t *request_file_accessor_get(struct request_file_accessor *accessor);

/*
ref - refresh
pre - precharge
act - activate
wr - write
rd - read
nop - no operation
*/


/*
 * command_file_accessor
 */

struct command_file_accessor
{
	FILE *f;

	struct dram_command_t *cached_command;
	long long cached_command_cycle;
	long long command_id_counter;
};

struct command_file_accessor *command_file_accessor_create(void);
void command_file_accessor_free(struct command_file_accessor *accessor);

struct dram_command_t *command_file_accessor_get(struct command_file_accessor *accessor, struct dram_t *dram);


#endif

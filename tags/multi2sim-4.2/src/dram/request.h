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

#ifndef DRAM_REQUEST_H
#define DRAM_REQUEST_H


/*
 * Local variable
 */

extern int EV_DRAM_REQUEST;

enum dram_request_type_t
{
	request_type_invalid,
	request_type_read,
	request_type_write,
	request_type_refresh
};


/*
 * Request
 */

struct request_stack_t
{
	struct dram_system_t *system;
	char *request_line;
};

struct dram_request_t
{
	long long id;
	long long cycle;
	unsigned int addr;
	enum dram_request_type_t type;
	struct dram_system_t *system;
};

struct request_stack_t *dram_request_stack_create(void);
void dram_request_stack_free(struct request_stack_t *);

struct dram_request_t *dram_request_create(void);
void dram_request_free(struct dram_request_t *request);
void dram_request_dump(struct dram_request_t *request, FILE *f);
void dram_request_handler (int event, void *data);


#endif

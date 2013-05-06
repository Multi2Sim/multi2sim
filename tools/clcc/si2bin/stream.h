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

#ifndef TOOLS_CLCC_SI2BIN_STREAM_H
#define TOOLS_CLCC_SI2BIN_STREAM_H

#include <stdio.h>


/*
 * Stream
 */

/* Forward declarations */
struct si2bin_inst_t;

struct si2bin_stream_t
{
	void *buf;

	int size;
	int offset;
};


struct si2bin_stream_t *si2bin_stream_create(int size);
void si2bin_stream_free(struct si2bin_stream_t *stream);

void si2bin_stream_add_inst(struct si2bin_stream_t *stream,
		struct si2bin_inst_t *inst);




/*
 * Global
 */

/* Global output stream */
extern struct si2bin_stream_t *si2bin_out_stream;

void si2bin_stream_init(void);
void si2bin_stream_done(void);


#endif


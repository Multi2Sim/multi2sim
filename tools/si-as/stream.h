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

#ifndef TOOLS_SI_AS_LABEL_H
#define TOOLS_SI_AS_LABEL_H

#include <stdio.h>


/*
 * Stream
 */

/* Forward declarations */
struct si_dis_inst_t;

struct si_stream_t
{
	void *buf;

	int size;
	int offset;
};


struct si_stream_t *si_stream_create(int size);
void si_stream_free(struct si_stream_t *stream);

void si_stream_add_inst(struct si_stream_t *stream,
		struct si_dis_inst_t *inst);




/*
 * Global
 */

/* Global output stream */
extern struct si_stream_t *si_out_stream;

void si_stream_init(void);
void si_stream_done(void);


#endif


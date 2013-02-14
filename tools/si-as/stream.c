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

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>

#include "dis-inst.h"
#include "task.h"
#include "stream.h"


/* Global output stream */
struct si_stream_t *stream;


struct si_stream_t *si_stream_create(char *fileName)
{
	struct si_stream_t *stream;
	
	/* Allocate */
	stream = xcalloc(1, sizeof(struct si_stream_t));
	
	/* Initialize */
	stream->out_file = fopen(fileName, "wr");
	stream->offset = 0;
	
	return stream;
}


void si_stream_free(struct si_stream_t *stream)
{
	free(stream);
}


void si_stream_add_inst(struct si_stream_t *stream, struct si_dis_inst_t *inst)
{
	unsigned long long inst_bytes;
	int size = 0;
	
	size = si_dis_inst_code_gen(inst, &inst_bytes);
	
	if (size == 0)
	{
		printf("BINARY: Could not decode!\n");
	} else 
	{
		if (size == 4)
			fprintf(stdout, "BINARY: %lx\n", (unsigned long) inst_bytes);
		else if (size == 8)
			fprintf(stdout, "BINARY: %016llx\n", inst_bytes);
		stream->offset += size;
		fwrite(&inst_bytes , size , 1, stream->out_file);
	}
}

long si_stream_get_offset(struct si_stream_t *stream)
{
	return (stream->offset);
}
void si_stream_resolve_task(struct si_stream_t *stream, struct si_task_t *task)
{
	/* Go to task's offset and replace bits 0-16 with label_tabel's offset - task's offset */
}

void si_stream_close(struct si_stream_t *stream)
{
	fclose(stream->out_file);
}


#include <lib/util/debug.h>

#include "task.h"
#include "inst.h"
#include "stream.h"


struct si_stream_t *si_stream_create(char *fileName)
{
	struct si_stream_t *stream;
	
	/* Allocate */
	stream = calloc(1, sizeof(struct si_stream_t));
	if (!stream)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Initialize */
	stream->out_file = fopen(fileName, "wr");
	stream->offset = 0;
	
	return stream;
}

void si_stream_add_inst(struct si_stream_t *stream, struct si_inst_t *inst)
{
	unsigned long long inst_bytes;
	int size = 0;
	
	size = si_inst_code_gen(inst, &inst_bytes);
	
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


#include <stdio.h>

/* Forward declarations */
struct si_task_t;
struct si_dis_inst_t;

struct si_stream_t
{
	FILE *out_file;
	long offset;
};

/* A global variable called 'stream' */
extern struct si_stream_t *stream;

struct si_stream_t *si_stream_create(char *fileName);
void si_stream_free(struct si_stream_t *stream);

void si_stream_add_inst(struct si_stream_t *stream, struct si_dis_inst_t *inst);
long si_stream_get_offset(struct si_stream_t *stream);
void si_stream_resolve_task(struct si_stream_t *stream, struct si_task_t *task);
void si_stream_close(struct si_stream_t *stream);

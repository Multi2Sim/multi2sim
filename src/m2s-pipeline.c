#include <curses.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <time.h>
#include <curses.h>

/*
 * Constants
 */

#define MAX_CTX 10
#define STATE_INTERVAL 100
#define KBRD_BUF_SIZE 10




/*
 * Global Variables
 */

struct ctx_t {
	WINDOW *wnd;

	/* Trace and state files */
	char *trace_file_name;
	FILE *trace_file;
	char *state_file_name;
	FILE *state_file;

	/* Array of state positions in state file */
	long *state_array;
	int state_array_size, state_array_count;

	long leftcycle;
	long topseq;
	long lastcycle;
	
	int maxy, maxx;
	int topy, topx;
	int rows, cols;
	int col_width, mop_header_width, uop_header_width, total_width;

};

WINDOW *mainwnd;

struct ctx_t *ctx_array[MAX_CTX];
int ctx_count;
int active_ctx = 0;
int show_mops = 0;
char kbrd_buf[KBRD_BUF_SIZE];



/*
 * Auxiliary functions
 */

/* Error message */
void error(char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	endwin();
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	exit(1);
}

/* Fill 'buf' with spaces or truncate to 'len' */
void buf_fill(char *buf, int len, char c)
{
	buf[len] = '\0';
	while (strlen(buf) < len) {
		buf[strlen(buf) + 1] = 0;
		buf[strlen(buf)] = c;
	}
}

/* Advance 'buf' while characters in 'set' are found */
void buf_skip(char **pbuf, char *set)
{
	int i, found;
	for (;;) {
		if (**pbuf == '\0' || **pbuf == '\n')
			break;
		for (i = found = 0; set[i] && !found; i++)
			found = **pbuf == set[i];
		if (!found)
			break;
		(*pbuf)++;
	}
}

/* Copy buffer characters until a character in 'set' is found */
void buf_copy(char **pbuf, char *set, char *dest, int size)
{
	int i, found;
	size--;
	for (;;) {
		if (**pbuf == '\0' || **pbuf == '\n')
			break;
		for (i = found = 0; set[i] && !found; i++)
			found = **pbuf == set[i];
		if (found)
			break;
		if (size) {
			*dest = **pbuf;
			dest++;
			size--;
		}
		(*pbuf)++;
	}
	*dest = 0;
}



/*
 * Functions to analyze commands
 */

struct command_field_t {
	struct command_field_t *next;
	char name[20];
	char value[100];
};

struct command_t {
	char name[20];
	long trace_file_pos;
	struct command_field_t *fields;
};

/* Read a line from 'trace_file' and decode it */
struct command_t *command_read(struct ctx_t *ctx)
{
	struct command_t *cmd;
	struct command_field_t *fld, *prevfld;
	char string[500];
	char *buf;

	cmd = calloc(1, sizeof(struct command_t));
	cmd->trace_file_pos = ftell(ctx->trace_file);

	buf = fgets(string, sizeof(string), ctx->trace_file);
	if (!buf || feof(ctx->trace_file)) {
		free(cmd);
		return NULL;
	}
	if (*buf && buf[strlen(buf) - 1] == '\n')
		buf[strlen(buf) - 1] = '\0';

	buf_skip(&buf, " \t");
	buf_copy(&buf, " \t", cmd->name, sizeof(cmd->name));
	buf_skip(&buf, " \t");

	prevfld = NULL;
	while (*buf != '\0' && *buf != '\n') {
		fld = calloc(1, sizeof(struct command_field_t));
		buf_copy(&buf, "= \t", fld->name, sizeof(fld->name));
		buf_skip(&buf, "= \t");
		if (*buf == '"') {
			buf++;
			buf_copy(&buf, "\"", fld->value, sizeof(fld->value));
			buf_skip(&buf, "\" \t,");
		} else if (*buf == '\'') {
			buf++;
			buf_copy(&buf, "'", fld->value, sizeof(fld->value));
			buf_skip(&buf, "' \t,");
		} else {
			buf_copy(&buf, " \t,", fld->value, sizeof(fld->value));
			buf_skip(&buf, " \t,");
		}

		if (prevfld)
			prevfld->next = fld;
		else
			cmd->fields = fld;
		prevfld = fld;
	}
	return cmd;
}

void command_free(struct command_t *cmd)
{
	struct command_field_t *nfield;
	while (cmd->fields) {
		nfield = cmd->fields->next;
		free(cmd->fields);
		cmd->fields = nfield;
	}
	free(cmd);
}

void command_dump(struct command_t *cmd, FILE *f)
{
	struct command_field_t *fld;
	fprintf(f, "%s ", cmd->name);
	for (fld = cmd->fields; fld; fld = fld->next)
		fprintf(f, "%s=\"%s\"%s", fld->name, fld->value,
			fld->next ? ", " : "");
	fprintf(f, "\n");
}

int command_has_field(struct command_t *cmd, char *name)
{
	struct command_field_t *fld;
	for (fld = cmd->fields; fld; fld = fld->next)
		if (!strcmp(fld->name, name))
			return 1;
	return 0;
}

char *command_get_field(struct command_t *cmd, char *name)
{
	struct command_field_t *fld;
	for (fld = cmd->fields; fld; fld = fld->next)
		if (!strcmp(fld->name, name))
			return fld->value;
	return "";
}




/*
 * Functions for Pipeline State
 */

struct state_uop_t {
	struct state_uop_t *next;

	char name[40];
	long seq;
	int spec;

	char mop_name[40];
	long mop_seq;
	int mop_count;
	int mop_index;

	int in_rob;
	int in_lsq;
	int in_iq;

	int stg_dispatch;
	int stg_issue;
	int stg_writeback;
	int stg_commit;

	int destroyed;
	int squashed;
	int ready;
	int issued;
	int completed;
};

struct state_t {
	long cycle;
	long state_file_pos;
	long trace_file_pos;
	struct state_uop_t *uops;
};

struct state_t *state_create(void)
{
	struct state_t *st;
	st = calloc(1, sizeof(struct state_t));
	return st;
}

void state_free(struct state_t *st)
{
	struct state_uop_t *nuop;
	while (st->uops) {
		nuop = st->uops->next;
		free(st->uops);
		st->uops = nuop;
	}
	free(st);
}

/* Return and uop by its seq number */
struct state_uop_t *state_get_uop(struct state_t *st, long seq)
{
	struct state_uop_t *uop;
	for (uop = st->uops; uop; uop = uop->next)
		if (uop->seq == seq)
			return uop;
	return NULL;
}

#define STATE_UOP_UPDATE_STRING(X) \
	if (command_has_field(cmd, #X)) \
		strncpy(uop->X, command_get_field(cmd, #X), sizeof(uop->X))
#define STATE_UOP_UPDATE_INT(X) \
	if (command_has_field(cmd, #X)) \
		uop->X = atoi(command_get_field(cmd, #X));
#define STATE_UOP_UPDATE_LONG(X) \
	if (command_has_field(cmd, #X)) \
		uop->X = atol(command_get_field(cmd, #X));

void state_uop_update(struct state_uop_t *uop, struct command_t *cmd)
{
	STATE_UOP_UPDATE_STRING(name);
	STATE_UOP_UPDATE_LONG(seq);
	STATE_UOP_UPDATE_INT(spec);

	STATE_UOP_UPDATE_STRING(mop_name);
	STATE_UOP_UPDATE_LONG(mop_seq);
	STATE_UOP_UPDATE_INT(mop_count);
	STATE_UOP_UPDATE_INT(mop_index);

	STATE_UOP_UPDATE_INT(in_rob);
	STATE_UOP_UPDATE_INT(in_iq);
	STATE_UOP_UPDATE_INT(in_lsq);

	STATE_UOP_UPDATE_INT(ready);
	STATE_UOP_UPDATE_INT(issued);
	STATE_UOP_UPDATE_INT(completed);

	STATE_UOP_UPDATE_INT(stg_dispatch);
	STATE_UOP_UPDATE_INT(stg_issue);
	STATE_UOP_UPDATE_INT(stg_writeback);
	STATE_UOP_UPDATE_INT(stg_commit);
}

void state_dump(struct state_t *st, FILE *f)
{
	struct state_uop_t *uop;
	fprintf(f, "State in cycle %ld\n", st->cycle);
	fprintf(f, "  trace_file_pos: %ld\n", st->trace_file_pos);
	fprintf(f, "  uops: ");
	for (uop = st->uops; uop; uop = uop->next) {
		fprintf(f, "%ld", uop->seq);
		if (uop->next)
			fprintf(f, ", ");
	}
	fprintf(f, "\n");
}

void state_write(struct state_t *st, FILE *f)
{
	long uop_count_pos;
	long end_pos;
	int uop_count = 0;
	struct state_uop_t *uop;

	/* Dump state */
	st->state_file_pos = ftell(f);
	fwrite(st, sizeof(struct state_t), 1, f);

	/* Save space for uop_count */
	uop_count_pos = ftell(f);
	fwrite(&uop_count, sizeof(uop_count), 1, f);

	/* Dump uops */
	for (uop = st->uops; uop; uop = uop->next) {
		fwrite(uop, sizeof(struct state_uop_t), 1, f);
		uop_count++;
	}
	end_pos = ftell(f);

	/* Write uop_count */
	fseek(f, uop_count_pos, SEEK_SET);
	fwrite(&uop_count, sizeof(uop_count), 1, f);
	fseek(f, end_pos, SEEK_SET);
}

struct state_t *state_read(FILE *f)
{
	struct state_t *st;
	int uop_count = 0, i;
	struct state_uop_t *uop;
	int bytes_read;

	st = state_create();
	bytes_read = fread(st, sizeof(struct state_t), 1, f);
	st->uops = NULL;
	bytes_read += fread(&uop_count, sizeof(uop_count), 1, f);
	for (i = 0; i < uop_count; i++) {
		uop = calloc(1, sizeof(struct state_uop_t));
		bytes_read += fread(uop, sizeof(struct state_uop_t), 1, f);
		uop->next = st->uops;
		st->uops = uop;
	}
	return st;
}

/* Process the command at the current position in the
 * trace file. */
void state_process_command(struct state_t *st, struct command_t *cmd)
{
	if (!strcmp(cmd->name, "uop")) {
		char *action = command_get_field(cmd, "action");
		if (!strcmp(action, "create")) {
			struct state_uop_t *uop;
			long seq;
			seq = atol(command_get_field(cmd, "seq"));
			uop = state_get_uop(st, seq);
			if (uop)
				error("uop seq %ld duplicated", seq);
			
			uop = calloc(1, sizeof(struct state_uop_t));
			uop->next = st->uops;
			st->uops = uop;
			state_uop_update(uop, cmd);
		} else if (!strcmp(action, "update")) {
			struct state_uop_t *uop;
			long seq;
			seq = atol(command_get_field(cmd, "seq"));
			uop = state_get_uop(st, seq);
			if (uop)
				state_uop_update(uop, cmd);
		} else if (!strcmp(action, "destroy")) {
			struct state_uop_t *uop;
			long seq;
			seq = atol(command_get_field(cmd, "seq"));
			uop = state_get_uop(st, seq);
			if (uop)
				uop->destroyed = 1;
		} else if (!strcmp(action, "squash")) {
			struct state_uop_t *uop;
			long seq;
			seq = atol(command_get_field(cmd, "seq"));
			uop = state_get_uop(st, seq);
			if (uop)
				uop->squashed = 1;
		}
	}
}

/* Advance state one cycle */
int state_process_cycle(struct ctx_t *ctx, struct state_t *st)
{
	struct command_t *cmd;
	struct state_uop_t *uop, *puop, *nuop;
	long cycle;

	/* Read clk command */
	fseek(ctx->trace_file, st->trace_file_pos, SEEK_SET);
	cmd = command_read(ctx);
	if (!cmd)
		return 0;
	if (strcmp(cmd->name, "clk"))
		error("'clk' command expected");

	/* Get next cycle number */
	cycle = atol(command_get_field(cmd, "c"));
	command_free(cmd);
	if (cycle <= st->cycle)
		error("'clk' commands disordered: %ld %ld", cycle, st->cycle);
	
	/* Before starting cycle, free all destroyed uops. Clear their current
	 * stage flags. */
	puop = NULL;
	for (uop = st->uops; uop; uop = nuop) {
		nuop = uop->next;
		if (uop->destroyed || uop->squashed) {
			free(uop);
			if (puop)
				puop->next = nuop;
			else
				st->uops = nuop;
			continue;
		}
		uop->stg_dispatch = 0;
		uop->stg_issue = 0;
		uop->stg_writeback = 0;
		uop->stg_commit = 0;
		puop = uop;
	}

	/* Advance cycle. */
	st->cycle++;
	if (cycle > st->cycle) {
		fseek(ctx->trace_file, st->trace_file_pos, SEEK_SET);
		return 1;
	}

	/* Process commands for current cycle */
	while (!feof(ctx->trace_file)) {
		cmd = command_read(ctx);
		if (!cmd)
			break;
		if (!strcmp(cmd->name, "clk")) {
			command_free(cmd);
			fseek(ctx->trace_file, st->trace_file_pos, SEEK_SET);
			break;
		}
		state_process_command(st, cmd);
		st->trace_file_pos = ftell(ctx->trace_file);
		command_free(cmd);
	}
	return 1;
}

/* Return the state in the given cycle */
struct state_t *state_goto(struct ctx_t *ctx, long cycle)
{
	struct state_t *st;
	long index;

	/* Later than last cycle */
	if (cycle > ctx->lastcycle) {
		st = state_create();
		st->cycle = cycle;
		fseek(ctx->trace_file, 0, SEEK_END);
		st->trace_file_pos = ftell(ctx->trace_file);
		return st;
	}
	
	/* Cycle 0 */
	if (!cycle) {
		st = state_create();
		st->cycle = -1;
		st->trace_file_pos = 0;
		state_process_cycle(ctx, st);
		return st;
	}

	/* Find index in state_array */
	index = cycle / STATE_INTERVAL;
	if (index >= ctx->state_array_count)
		index = ctx->state_array_count - 1;
	assert(index >= 0);
	
	/* Read initial state from state file */
	fseek(ctx->state_file, ctx->state_array[index], SEEK_SET);
	st = state_read(ctx->state_file);
	assert(st->cycle <= cycle);
	while (st->cycle < cycle)
		state_process_cycle(ctx, st);
	return st;
}




/*
 * Main Program
 */

struct ctx_t *ctx_create(char *trace_file_name, int height, int width, int y, int x)
{
	struct ctx_t *ctx;
	char buf[500];
	struct state_t *st;
	clock_t clk;

	ctx = calloc(1, sizeof(struct ctx_t));

	/* Files */
	ctx->trace_file_name = strdup(trace_file_name);
	ctx->trace_file = fopen(trace_file_name, "rt");
	if (!ctx->trace_file)
		error("%s: cannot open file", trace_file_name);
	sprintf(buf, "%s.status", trace_file_name);
	ctx->state_file_name = strdup(buf);
	ctx->state_file = fopen(ctx->state_file_name, "w+b");
	if (!ctx->state_file)
		error("%s: cannot create state file", ctx->state_file_name);
	
	/* State array */
	ctx->state_array_size = 1024;
	ctx->state_array = calloc(ctx->state_array_size, sizeof(long));

	/* Generate state file */
	st = state_goto(ctx, 0);
	clk = clock() - CLOCKS_PER_SEC;
	for (;;) {
		
		/* Info */
		if (clock() - clk > CLOCKS_PER_SEC / 4) {
			clk = clock();
			werase(mainwnd);
			mvwprintw(mainwnd, LINES - 1, 0, "Generating status file... %s - %ld cycles, %d states",
				ctx->trace_file_name, st->cycle, ctx->state_array_count);
			wrefresh(mainwnd);
		}
		
		/* Dump current state */
		if (st->cycle % STATE_INTERVAL == 0) {
			if (ctx->state_array_count == ctx->state_array_size) {
				ctx->state_array_size = ctx->state_array_size * 2;
				ctx->state_array = realloc(ctx->state_array, ctx->state_array_size * sizeof(long));
				if (!ctx->state_array)
					error("out of memory");
			}
			ctx->state_array[ctx->state_array_count++] = ftell(ctx->state_file);
			state_write(st, ctx->state_file);
		}
		
		/* Next cycle */
		if (!state_process_cycle(ctx, st))
			break;
	}
	ctx->lastcycle = st->cycle;

	/* Window */
	ctx->wnd = newwin(height, width, y, x);
	nodelay(ctx->wnd, FALSE);
	keypad(ctx->wnd, TRUE);
	wrefresh(ctx->wnd);
	
	return ctx;
}

void ctx_free(struct ctx_t *ctx)
{
	fclose(ctx->trace_file);
	fclose(ctx->state_file);
	remove(ctx->state_file_name);
	
	free(ctx->trace_file_name);
	free(ctx->state_file_name);
	free(ctx->state_array);
	free(ctx);
}

void ctx_update_topseq(struct ctx_t *ctx)
{
	struct state_t *st;
	struct state_uop_t *uop;
	long cycle;

	/* Calculate topseq - seq number of uop at left-top corner */
	st = state_goto(ctx, ctx->leftcycle);
	ctx->topseq = 0;
	if (ctx->leftcycle > ctx->lastcycle)
		return;
	for (cycle = ctx->leftcycle; cycle < ctx->leftcycle + ctx->cols; cycle++) {
		for (uop = st->uops; uop; uop = uop->next)
			if (uop->seq > ctx->topseq + ctx->rows - 1)
				ctx->topseq = uop->seq - ctx->rows + 1;
		state_process_cycle(ctx, st);
	}
	state_free(st);
}

void ctx_update_display(struct ctx_t *ctx)
{
	struct state_t *st;
	struct state_uop_t *uop;
	int i, x, y, color;
	char buf[500];
	long cycle;

	/* Refresh table parameters */
	getmaxyx(ctx->wnd, ctx->maxy, ctx->maxx);
	ctx->col_width = 4;
	ctx->mop_header_width = show_mops ? 40 : 0;
	ctx->uop_header_width = 30;
	ctx->topy = 0;
	ctx->topx = 1;
	ctx->rows = ctx->maxy - ctx->topy - 2;
	ctx->cols = (ctx->maxx - ctx->topx - ctx->mop_header_width - ctx->uop_header_width) / ctx->col_width;
	ctx->total_width = ctx->mop_header_width + ctx->uop_header_width + ctx->col_width * ctx->cols;
	
	/* Erase window */
	werase(ctx->wnd);
	
	/* Write column headers */
	for (i = 0; i < ctx->cols; i++)
		mvwprintw(ctx->wnd, ctx->topy, ctx->topx + ctx->mop_header_width + ctx->uop_header_width
			+ i * ctx->col_width, "%03ld ", (ctx->leftcycle + i) % 1000);

	/* Draw uops */
	st = state_goto(ctx, ctx->leftcycle);
	for (cycle = ctx->leftcycle; cycle < ctx->leftcycle + ctx->cols; cycle++) {
		for (uop = st->uops; uop; uop = uop->next) {
		
			/* Uop in range */
			if (uop->seq < ctx->topseq || uop->seq > ctx->topseq + ctx->rows - 1)
				continue;
			y = ctx->topy + uop->seq - ctx->topseq + 1;

			/* Header */
			if (uop->spec)
				wattron(ctx->wnd, COLOR_PAIR(1));
			if (show_mops) {
				wattron(ctx->wnd, A_BOLD);
				strcpy(buf, uop->mop_name);
				buf[ctx->mop_header_width - 1] = '\0';
				mvwprintw(ctx->wnd, y, ctx->topx, buf);
				wattroff(ctx->wnd, A_BOLD);
			}
			if (!uop->mop_index)
				wattron(ctx->wnd, A_BOLD);
			sprintf(buf, "%03ld %s", uop->seq % 1000, uop->name);
			buf[ctx->uop_header_width - 1] = '\0';
			mvwprintw(ctx->wnd, y, ctx->topx + ctx->mop_header_width, buf);
			wattroff(ctx->wnd, A_BOLD);
			wattroff(ctx->wnd, COLOR_PAIR(1));

			/* State */
			buf[0] = '\0';
			if (uop->stg_dispatch)
				strcpy(buf, "Di ");
			if (uop->stg_issue)
				strcpy(buf, "I  ");
			if (uop->stg_writeback)
				strcpy(buf, "Wb ");
			if (uop->stg_commit)
				strcpy(buf, "C  ");

			/* Select color */
			color = 1;
			if (uop->ready)
				color = 2;
			if (uop->issued)
				color = 3;
			if (uop->completed)
				color = 4;
			wattron(ctx->wnd, COLOR_PAIR(color));

			/* Print */
			buf_fill(buf, 3, color == 2 ? '_' : '.');
			x = ctx->topx + ctx->mop_header_width + ctx->uop_header_width +
				(cycle - ctx->leftcycle) * ctx->col_width;
			mvwprintw(ctx->wnd, y, x, buf);
			wattroff(ctx->wnd, COLOR_PAIR(color));

			/* Additional character */
			strcpy(buf, " ");
			if (uop->squashed)
				strcpy(buf, "X");
			mvwprintw(ctx->wnd, y, x + ctx->col_width - 1, buf);
		}
		if (!state_process_cycle(ctx, st))
			break;
	}

	/* Status bar */
	color = ctx == ctx_array[active_ctx] ? 5 : 8;
	wattron(ctx->wnd, COLOR_PAIR(color));
	sprintf(buf, "cycle=%ld, seq=%ld, lastcycle=%ld", ctx->leftcycle, ctx->topseq, ctx->lastcycle);
	if (kbrd_buf[0]) {
		strcat(buf, " - (");
		strcat(buf, kbrd_buf);
		strcat(buf, ")");
	}
	buf_fill(buf, ctx->maxx, ' ');
	mvwprintw(ctx->wnd, ctx->maxy - 1, 0, buf);
	wattroff(ctx->wnd, COLOR_PAIR(color));

	/* Refresh window */
	wrefresh(ctx->wnd);
}


/* Draw window */
void update_display(void)
{
	int i;
	char buf[500];
	for (i = 0; i < ctx_count; i++)
		ctx_update_display(ctx_array[i]);
	wattron(mainwnd, COLOR_PAIR(9));
	sprintf(buf, "Keypad-move table; s-swap window; h-help; q-quit");
	buf_fill(buf, COLS, ' ');
	mvwprintw(mainwnd, LINES - 1, 0, buf);
	wattroff(mainwnd, COLOR_PAIR(9));
}


/* Help window */
void show_help(void)
{
	char buf[500];
	werase(mainwnd);

	/* Help */
	mvwprintw(mainwnd, 0, 0, "Keyboard commands:\n");
	wprintw(mainwnd, "\n");

	wprintw(mainwnd, "  n       Next cycle\n");
	wprintw(mainwnd, "  N       Next cycle in all windows\n");
	wprintw(mainwnd, "  b       Previous cycle\n");
	wprintw(mainwnd, "  B       Previous cycle in all windows\n");
	wprintw(mainwnd, "PgDown    Forward chronogram\n");
	wprintw(mainwnd, " PgUp     Rewind chronogram\n");
	wprintw(mainwnd, "Arrows    Scroll chronogram\n");
	wprintw(mainwnd, " Home     Go to first cycle\n");
	wprintw(mainwnd, "  End     Go to last cycle\n");
	wprintw(mainwnd, "\n");

	wprintw(mainwnd, "  s       Synchronize all windows (go to same cycle)\n");
	wprintw(mainwnd, "  w       Switch current window\n");
	wprintw(mainwnd, "  m       Show/hide macroinstructions\n");
	wprintw(mainwnd, "  q       Quit\n");
	wprintw(mainwnd, "\n");

	wprintw(mainwnd, " 0-9      Write cycle or uop sequence number\n");
	wprintw(mainwnd, "  g       Go to cycle\n");

	/* Status bar */
	sprintf(buf, "Press any key to continue...");
	buf_fill(buf, COLS, ' ');
	wattron(mainwnd, COLOR_PAIR(8));
	mvwprintw(mainwnd, LINES - 1, 0, buf);
	wattroff(mainwnd, COLOR_PAIR(8));
	wrefresh(mainwnd);
	wgetch(mainwnd);
}


#define MAX_CTX 10
int main(int argc, char **argv)
{
	struct ctx_t *ctx;
	int done = 0;
	int i, y;

	/* Syntax */
	if (argc < 2)
		error("syntax: %s <file1> [<file2> ...]", argv[0]);
	
	/* Screen */
	mainwnd = initscr();
	if (!has_colors())
		error("your terminal does not support colors\n");

	/* Window */
	refresh();
	use_default_colors();
	start_color();
	cbreak();
	noecho();
	nonl();
	curs_set(0);
	intrflush(stdscr, FALSE);
	nodelay(mainwnd, FALSE);
	keypad(mainwnd, TRUE);
	wrefresh(mainwnd);
	
	/* Colors */
	init_pair(1, COLOR_RED, -1);
	init_pair(2, COLOR_YELLOW, -1);
	init_pair(3, COLOR_CYAN, -1);
	init_pair(4, COLOR_GREEN, -1);
	init_pair(5, COLOR_WHITE, COLOR_GREEN);
	init_pair(8, COLOR_WHITE, COLOR_BLACK);
	init_pair(9, COLOR_WHITE, COLOR_BLUE);
	
	/* Create contexts */
	ctx_count = argc - 1;
	if (ctx_count > MAX_CTX)
		error("maximum number of contexts is %d", MAX_CTX);
	y = 0;
	for (i = 0; i < ctx_count; i++) {
		int height = (LINES - 1) / ctx_count + (i < (LINES - 1) % ctx_count ? 1 : 0);
		ctx_array[i] = ctx_create(argv[i + 1], height, COLS, y, 0);
		y += height;
	}

	/* Loop */
	do {
		int ch;

		update_display();
		ch = wgetch(mainwnd);
		ctx = ctx_array[active_ctx];
		switch (ch) {
		case 'n':
			ctx->leftcycle++;
			ctx_update_topseq(ctx);
			break;

		case 'N':
			for (i = 0; i < ctx_count; i++) {
				ctx = ctx_array[i];
				ctx->leftcycle++;
				ctx_update_topseq(ctx);
			}
			break;

		case 'b':
			if (ctx->leftcycle > 0)
				ctx->leftcycle--;
			ctx_update_topseq(ctx);
			break;

		case 'B':
			for (i = 0; i < ctx_count; i++) {
				ctx = ctx_array[i];
				if (ctx->leftcycle > 0)
					ctx->leftcycle--;
				ctx_update_topseq(ctx);
			}
			break;

		case 's':
			for (i = 0; i < ctx_count; i++) {
				if (i == active_ctx)
					continue;
				ctx_array[i]->leftcycle = ctx->leftcycle;
				ctx_update_topseq(ctx_array[i]);
			}
			break;

		case 'q':
			done = 1;
			break;

		case 'w':
			active_ctx = (active_ctx + 1) % ctx_count;
			break;

		case KEY_RIGHT:
			ctx->leftcycle++;
			break;

		case KEY_LEFT:
			ctx->leftcycle--;
			if (ctx->leftcycle < 0)
				ctx->leftcycle = 0;
			break;

		case KEY_UP:
			ctx->topseq--;
			if (ctx->topseq < 0)
				ctx->topseq = 0;
			break;

		case KEY_DOWN:
			ctx->topseq++;
			break;

		case KEY_NPAGE:
			ctx->leftcycle += ctx->cols - 1;
			ctx_update_topseq(ctx);
			break;

		case KEY_PPAGE:
			ctx->leftcycle -= ctx->cols - 1;
			if (ctx->leftcycle < 0)
				ctx->leftcycle = 0;
			ctx_update_topseq(ctx);
			break;

		case KEY_HOME:
		case 'g':
			if (kbrd_buf[0]) {
				ctx->leftcycle = atol(kbrd_buf);
				ctx_update_topseq(ctx);
				kbrd_buf[0] = 0;
			} else {
				ctx->leftcycle = 0;
				ctx->topseq = 0;
			}
			break;

		case KEY_END:
		case 'G':
			kbrd_buf[0] = 0;
			ctx->leftcycle = ctx->lastcycle - ctx->cols + 1;
			if (ctx->leftcycle < 0)
				ctx->leftcycle = 0;
			ctx_update_topseq(ctx);
			break;

		case 'm':
			show_mops = !show_mops;
			break;

		case 'h':
			show_help();
			break;

		case KEY_BACKSPACE:
			if (kbrd_buf[0])
				kbrd_buf[strlen(kbrd_buf) - 1] = '\0';
			break;

		case KEY_RESIZE:
			y = 0;
			for (i = 0; i < ctx_count; i++) {
				int height = (LINES - 1) / ctx_count + (i < (LINES - 1) % ctx_count ? 1 : 0);
				wresize(ctx_array[i]->wnd, height, COLS);
				mvwin(ctx_array[i]->wnd, y, 0);
				y += height;
			}
			break;

		default:
			if (ch >= '0' && ch <= '9' && strlen(kbrd_buf) < KBRD_BUF_SIZE - 1) {
				kbrd_buf[strlen(kbrd_buf) + 1] = '\0';
				kbrd_buf[strlen(kbrd_buf)] = ch;
			}

		}
	} while (!done);

	/* Free contexts */
	for (i = 0; i < ctx_count; i++)
		ctx_free(ctx_array[i]);
	endwin();
	return 0;
}


/*
 *  Multi2Sim
 *  Copyright (C) 2007  Rafael Ubal Tena (raurte@gap.upv.es)
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

#include <m2s.h>



#define PTRACE_MAX_THREADS	8

static FILE *ptrace_outfd;
static FILE *ptrace_pcfd[PTRACE_MAX_THREADS];
static char *ptrace_pc = "";
static char *ptrace_file = "";


/* start ptrace */
void ptrace_init() {
	ptrace_outfd = open_write(ptrace_file);
	if (!ptrace_outfd && ptrace_file[0])
		fatal("%s: cannot open trace file", ptrace_file);
}


/* stop ptrace */
void ptrace_done() {
	int i;
	close_file(ptrace_outfd);
	for (i = 0; i < PTRACE_MAX_THREADS; i++)
		close_file(ptrace_pcfd[i]);
}


void ptrace_reg_options()
{
	opt_reg_string("-ptrace:file", "ptrace output file (stdout|stderr|<fname>)",
		&ptrace_file);
	opt_reg_string("-ptrace:pc", "trace file for pc sequence of ctx 0",
		&ptrace_pc);
}


void ptrace_new_uop(struct uop_t *uop)
{
	char status[10];

	if (!ptrace_outfd)
		return;
	
	/* uop status string */
	sprintf(status, "%s%s",
		uop->mispred ? "m" : "-",
		uop->specmode ? "s" : "-");
	
	fprintf(ptrace_outfd, "+ %llu %d 0x%x %s ",
		(long long) uop->seq, uop->ctx->pid, uop->eip, status);
	uop_dump(uop, ptrace_outfd);
	fprintf(ptrace_outfd, "\n");
	
	if (ptrace_outfd == stderr || ptrace_outfd == stdout)
		fflush(ptrace_outfd);
}


static struct string_map_t ptrace_stage_map = {
	7, {
		{ "IF",  ptrace_fetch },
		{ "DI",  ptrace_dispatch },
		{ "I",   ptrace_issue },
		{ "EX",  ptrace_execution },
		{ "M",   ptrace_memory },
		{ "WB",  ptrace_writeback },
		{ "C",   ptrace_commit }
	}
};


void ptrace_new_stage(struct uop_t *uop, enum ptrace_stage_enum stage)
{
	if (!ptrace_outfd)
		return;
	fprintf(ptrace_outfd, "* %llu %s\n", (long long) uop->seq,
		map_value(&ptrace_stage_map, stage));
	if (ptrace_outfd == stderr || ptrace_outfd == stdout)
		fflush(ptrace_outfd);
}


void ptrace_end_uop(struct uop_t *uop)
{
	if (!ptrace_outfd)
		return;
	fprintf(ptrace_outfd, "- %llu\n", (long long) uop->seq);
	if (ptrace_outfd == stderr || ptrace_outfd == stdout)
		fflush(ptrace_outfd);
}


void ptrace_new_cycle()
{
	if (!ptrace_outfd)
		return;
		
	fprintf(ptrace_outfd, "@ %lld\n", (long long) sim_cycle);
	if (ptrace_outfd == stderr || ptrace_outfd == stdout)
		fflush(ptrace_outfd);
}


/*
 *  Multi2Sim-Ptrace
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "table.h"

#define TABLE_TITLE "Trace (seq, status, pc, inst)"


void usage() {
	printf("Syntax: ptrace [options]\n"
		"\t-pc <offs>   Start trace from pc <offs> (hex format)\n"
		"\t-c <n>       Start trace from cycle <n>\n"
		"\t-i <inst>    Start trace after finding inst <i>\n"
		"\t-step <n>    Show state each <n> cycles (default 1)\n"
		"\t-width <w>   Table width\n"
		"\t-height <h>  Table height\n");
	exit(1);
}


static uint32_t start_pc = 0;
static uint64_t start_cycle = 0;
static int step = 1;
static int height = 30;
static int width = 30;
static char *start_inst = NULL;
static int start_inst_len = 0;
static int active = 0;
static FILE *kbrd;


void check_args(int argc, char **argv)
{
	int i;
	
	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-pc"))
		{
			if (i + 1 == argc)
				usage();
			sscanf(argv[i + 1], "0x%x", &start_pc);
			++i;
		}
		else if (!strcmp(argv[i], "-c"))
		{
			if (i + 1 == argc)
				usage();
			sscanf(argv[i + 1], "%lld", (long long *) &start_cycle);
			++i;
		}
		else if (!strcmp(argv[i], "-i"))
		{
			if (i + 1 == argc)
				usage();
			start_inst = argv[i + 1];
			start_inst_len = strlen(start_inst);
			++i;
		}
		else if (!strcmp(argv[i], "-step")) {
			if (i + 1 == argc)
				usage();
			sscanf(argv[i + 1], "%d", &step);
			++i;
		}
		else if (!strcmp(argv[i], "-width")) {
			if (i + 1 == argc)
				usage();
			sscanf(argv[i + 1], "%d", &width);
			++i;
		}
		else if (!strcmp(argv[i], "-height")) {
			if (i + 1 == argc)
				usage();
			sscanf(argv[i + 1], "%d", &height);
			++i;
		}
		else
			usage();
	}
}


void stop()
{
	char aux[128];
	int done = 0;
	
	do {
		printf("(Enter=NextCycle, Ctrl+C=End, n=NextPC/inst): ");
		fgets(aux, 10, kbrd);
		aux[strlen(aux) - 1] = '\0';
		if (!aux[0])
			done = 1;
		else if (!strcmp(aux, "n"))
		{
			if (!start_inst && !start_pc) {
				printf("\tNeither '-pc' nor '-i' specified/n");
				continue;
			}
			active = 0;
			done = 1;
		}
		else
			printf("\tCommand not known: %s\n", aux);
	} while (!done);
}


int main(int argc, char **argv)
{
	char s[128], name[128], stg[128], aux[128], *ylbl, status[128];
	int ptrace_start = 0, thread_id;
	uint64_t cycle, seq;
	uint32_t pc;
	struct table_t *table;

	check_args(argc, argv);
	table = table_create("", width, height, 2, 45);
	kbrd = fopen("/dev/tty", "rt");
	if (!start_pc && !start_cycle && !start_inst)
		active = 1;

	while (1) {
	
		/* Read stdin */
		fgets(s, 128, stdin);
		if (feof(stdin))
			break;
		if (*s == '@')
			ptrace_start = 1;
		if (!ptrace_start)
			continue;

		/* Action depending on command */
		switch (*s) {
			
		case '@':
			/* Trace activated */
			sscanf(s, "@ %Ld", (long long *) &cycle);
			if (start_cycle && cycle >= start_cycle)
				active = 1;
			
			/* Print table */
			if ((cycle % step == step - 1) && cycle && active) {
				table_print(table, stdout);
				stop();
			}
			sprintf(aux, "%lld", (long long) (cycle % 100));
			table_setxlbl(table, cycle, aux);

			/* Title */
			sprintf(aux, "%s - cycle=%lld", TABLE_TITLE, (long long) cycle);
			table_set_title(table, aux);
			break;
				
		case '+':
			sscanf(s, "+ %Ld %d %x %s %[^\n]s", (long long *) &seq,
				&thread_id, &pc, status, name);
			sprintf(aux, "%04lld %s %x %s", (long long) seq, status, pc & 0xffffff, name);
			table_setylbl(table, seq, aux);

			/* Activate trace for PC */
			if (start_pc && pc == start_pc)
				active = 1;

			/* Activate trace for inst */
			if (start_inst && !strncmp(start_inst, name, start_inst_len) &&
					(!name[start_inst_len] || name[start_inst_len] == ' '))
				active = 1;
			break;

		case '*':
			sscanf(s, "* %Ld %s", (long long *) &seq, stg);
			table_setcell(table, cycle, seq, stg);
			break;

		case '-':
			sscanf(s, "- %Ld", (long long *) &seq);
			ylbl = table_ylbl(table, seq);
			if (ylbl)
				strncpy(ylbl, "----", 4);
			break;

		default:
			if (active)
				printf("%s", s);
			
		}
	}

	table_print(table, stdout);
	fclose(kbrd);
	free(table);
	return 0;
}


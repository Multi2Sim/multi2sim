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


/* Stats */
static uint64_t max_cycles = 0;
static uint64_t max_inst = 0;
static uint64_t max_time = 0;
static char *ctxconfig = "";
static char *configfile = "";
static char *sim_title = "";

static char *syscall_debug_file = "";
static char *loader_debug_file = "";
static char *isa_call_debug_file = "";
static char *isa_inst_debug_file = "";



/* Variables */
static int sigint_received = FALSE;
uint64_t sim_cycle;






static void sim_reg_options()
{
	opt_reg_string("-title", "simulation title", &sim_title);
	opt_reg_string("-ctxconfig", "context configuration file", &ctxconfig);

	opt_reg_string("-syscall_debug", "debug information for system calls", &syscall_debug_file);	
	opt_reg_string("-loader_debug", "debug information from program loader", &loader_debug_file);	
	opt_reg_string("-call_debug", "debug information about procedure calls", &isa_call_debug_file);	
	opt_reg_string("-inst_debug", "debug information about executed instructions", &isa_inst_debug_file);

	opt_reg_uint64("-max_cycles", "cycle to stop program (0=no stop)", &max_cycles);
	opt_reg_uint64("-max_inst", "max number of retireed instructions (0=no max)", &max_inst);
	opt_reg_uint64("-max_time", "max running time (in seconds)", &max_time);
}


/* Signal handlers */
static void sim_signal_handler(int signum)
{
	switch (signum) {
	
	case SIGINT:
		if (sigint_received)
			abort();
		sigint_received = TRUE;
		p_dump(stderr);
		fprintf(stderr, "SIGINT received\n");
		break;
		
	case SIGABRT:
		signal(SIGABRT, SIG_DFL);
		p_dump(stderr);
		exit(1);
		break;
	}
}


int main(int argc, char **argv)
{
	struct list_t *uop_list;
	//struct bpred_t *bpred;
	struct uop_t *uop;
	uint32_t eip, neip;
	struct ctx_t *ctx;

	/* Options & stats */
	opt_init();
	sim_reg_options();
	//bpred_reg_options();
	opt_check_options(&argc, argv);
	if (*configfile)
		opt_check_config(configfile);

	/* Initial information */
	fprintf(stderr, "\nMulti2Sim - Branch Predictor Simulator (m2s-bpred)\n");
	fprintf(stderr, "Last compilation: %s %s\n\n", __DATE__, __TIME__);
	opt_print_options(stderr);

	/* Initialization */
	ke_init();
	uop_init();

	/* Debug */
	debug_init();
	debug_assign_file(syscall_debug_category, syscall_debug_file);
	debug_assign_file(ld_debug_category, loader_debug_file);
	debug_assign_file(isa_call_debug_category, isa_call_debug_file);
	debug_assign_file(isa_inst_debug_category, isa_inst_debug_file);

	uop_list = list_create(10);
	//bpred = bpred_create();
	//strcpy(bpred->name, "bpred");

	/* Load programs from configuration file and command line. */
	if (*ctxconfig)
		ld_load_prog_from_ctxconfig(ctxconfig);
	if (argc > 1)
		ld_load_prog_from_cmdline(argc - 1, argv + 1);
	if (!ke->context_list)
		fatal("no context loaded");

	/* Exhaustive simulation */
	signal(SIGINT, &sim_signal_handler);
	signal(SIGABRT, &sim_signal_handler);
	while (ke->context_list) {

		/* Next cycle */
		sim_cycle++;

		/* Execute one instruction */
		ctx = ke->context_list;
		eip = ctx->regs->eip;
		ctx_execute_inst(ctx);
		neip = ctx->regs->eip;
		while (ke->finished_list)
			ctx_free(ke->finished_list);

		/* Decode it and check branch predictor */
		uop_decode(&isa_inst, uop_list);
		while (list_count(uop_list)) {
			uop = list_dequeue(uop_list);
			if (uop->flags & FCTRL) {
				uop->eip = eip;
				uop->neip = neip;
				//uop->pred_neip = bpred_lookup(bpred, uop);
				//bpred_update(bpred, uop);
			}
			uop_free_if_not_queued(uop);
		}

		/* Halt execution */
		if (sigint_received)
			break;
		if (max_cycles && sim_cycle >= max_cycles)
			break;
		if (max_inst && p->committed >= max_inst)
			break;
		if (max_time && clock() > max_time * CLOCKS_PER_SEC)
			break;
	}

	/* Finalization */
	fprintf(stderr, "\n");
	list_free(uop_list);
	//bpred_free(bpred);

	opt_done();
	uop_done();
	ke_done();
	debug_done();
	fprintf(stderr, "\n");
	mhandle_done();
	return 0;
}

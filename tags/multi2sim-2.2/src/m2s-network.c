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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <options.h>
#include <esim.h>
#include <network.h>


/* Options */
static char *configfile = "";
static uint64_t max_cycles = 0;
static uint64_t max_time = 0;
static uint32_t bus_width = 1;
static uint32_t node_count = 8;
static double send_rate = 0.01;


/* Variables */
static struct net_t *net;
static int sigint_received = 0;
static double *last_injection;



static void sim_reg_options()
{
	opt_reg_string("-config", "m2s-network configuration file", &configfile);
	opt_reg_uint64("-max_cycles", "cycle to stop program (0=no stop)", &max_cycles);
	opt_reg_uint64("-max_time", "max running time (in seconds)", &max_time);
	opt_reg_uint32("-bus:width", "bus width in bytes", &bus_width);
	opt_reg_uint32("-node_count", "number of nodes", &node_count);
	opt_reg_double("-send_rate", "packet per cycle and node injected",
		&send_rate);
}


/* Signal handlers */
static void sim_signal_handler(int signum)
{
	switch (signum) {
	
	case SIGINT:
		if (sigint_received)
			abort();
		sigint_received = 1;
		fprintf(stderr, "SIGINT received\n");
		break;
	}
}



double exponential(double lambda)
{
	double r = (double) (random() & 0xfffffff) / (double) 0xfffffff;
	return -log(1.0 - r) / lambda;
}


static void sim_inject_packets()
{
	int node, dst;
	for (node = 0; node < node_count; node++) {
		while (last_injection[node] < (double) esim_cycle) {
			do {
				dst = random() % node_count;
			} while (dst == node);
			net_send(net, node, dst, 1);
			last_injection[node] += exponential(send_rate);
		}
	}
}


int main(int argc, char **argv)
{
	/* Initial information */
	fprintf(stderr, "\nMulti2Sim. Network Simulator (m2s-network)\n");
	fprintf(stderr, "Last compilation: %s %s\n\n", __DATE__, __TIME__);

	/* Options */
	opt_init();
	sim_reg_options();
	opt_check_options(&argc, argv);
	opt_check_config(configfile);
	opt_print_options(stderr);

	/* Initialize */
	esim_init();
	net_init();
	last_injection = calloc(node_count, sizeof(double));

	/* Create network */
	net = net_create_bus();

	net_init_nodes(net, 7);
	net_add_node(net, net_node_kind_cache, NULL);
	net_add_node(net, net_node_kind_cache, NULL);
	net_add_node(net, net_node_kind_cache, NULL);
	net_add_node(net, net_node_kind_cache, NULL);
	node_count = 4;

/*	net_add_node(net, net_node_kind_switch, NULL);
	net_add_node(net, net_node_kind_switch, NULL);
	net_add_node(net, net_node_kind_switch, NULL);

	net_add_con(net, 0, 4);
	net_add_con(net, 1, 4);
	net_add_con(net, 2, 5);
	net_add_con(net, 3, 5);
	net_add_con(net, 4, 6);
	net_add_con(net, 5, 6);

	net_calculate_routes(net);
	net_dump_routes(net, stdout);*/
	

	/* Exhaustive simulation */
	signal(SIGINT, &sim_signal_handler);
	while (!sigint_received) {
		
		/* Packet injection */
		sim_inject_packets();

		/* Process network event */
		esim_process_events();

		/* Stop simulation */
		if (esim_cycle >= max_cycles && max_cycles)
			break;
		if (max_time && clock() > max_time * CLOCKS_PER_SEC)
			break;

	}

	/* Stats */
	fprintf(stderr, "\n");
	fprintf(stderr, "cycles  %lld  # Simulation cycles\n",
		(long long) esim_cycle);
	fprintf(stderr, "packets  %lld  # Number of packets delivered\n",
		(long long) net->delivered);
	fprintf(stderr, "throughput  %.4f  # Packets delivered per cycle and node\n",
		(double) net->delivered / esim_cycle / node_count);
	fprintf(stderr, "latency  %.2f  # Average latency for packets\n",
		(double) net->deliver_time / net->delivered);
	fprintf(stderr, "time  %.1f  # Simulation time\n",
		(double) clock() / CLOCKS_PER_SEC);
	fprintf(stderr, "cps  %.0f  # Cycles simulated per second\n",
		clock() ? (double) esim_cycle / clock() * CLOCKS_PER_SEC : 0.0);
	fprintf(stderr, "\n");

	/* Finalization */
	free(last_injection);
	net_free(net);
	opt_done();
	net_done();
	esim_done();
	mhandle_done();
	return 0;
}

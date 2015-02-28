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

#ifndef NETWORK_NET_SYSTEM_H
#define NETWORK_NET_SYSTEM_H

/* Error messages */
extern char *net_err_end_nodes;
extern char *net_err_no_route;
extern char *net_err_large_message;
extern char *net_err_node_name_duplicate;
extern char *net_err_config;
extern char *net_err_can_send;
extern char *net_err_cycle;
extern char *net_err_route_step;

extern struct hash_table_t *net_table;

/* Debug */
#define net_debug(...) debug(net_debug_category, __VA_ARGS__)
extern int net_debug_category;

/* Trace */
#define net_tracing() trace_status(net_trace_category)
#define net_trace(...) trace(net_trace_category, __VA_ARGS__)
#define net_trace_header(...) trace_header(net_trace_category, __VA_ARGS__)

#define NET_SYSTEM_TRACE_VERSION_MAJOR          1
#define NET_SYSTEM_TRACE_VERSION_MINOR          10

extern int net_trace_category;

/* Configuration parameters */
extern char *net_report_file_name;
extern char *net_visual_file_name;
extern char *net_sim_network_name;
extern char *net_route_file_name;

extern char *net_traffic_pattern;

extern long long net_max_cycles;
extern double net_injection_rate;
extern int net_msg_size;
extern int net_snap_period;

/* Frequency and frequency domain */
extern int net_frequency;
extern int net_domain_index;

/* Functions */
void net_init(void);
void net_done(void);

void net_load(char *file_name);

struct net_t *net_find(char *name);
struct net_t *net_find_first(void);
struct net_t *net_find_next(void);

void net_sim(char *debug_file_name);

#endif

/*
 *  Multi2Sim Tools
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


#ifndef VISUAL_COMMON_STATE_H
#define VISUAL_COMMON_STATE_H


struct vi_trace_line_t;
typedef void (*vi_state_write_checkpoint_func_t)(void *user_data, FILE *f);
typedef void (*vi_state_read_checkpoint_func_t)(void *user_data, FILE *f);
typedef void (*vi_state_process_trace_line_func_t)(void *user_data, struct vi_trace_line_t *trace_line);

#define VI_STATE_FOR_EACH_HEADER(trace_line) \
	for ((trace_line) = vi_state_header_first(); \
	(trace_line); (trace_line) = vi_state_header_next())

void vi_state_init(char *trace_file_name);
void vi_state_done(void);

long long vi_state_get_num_cycles(void);
long long vi_state_get_current_cycle(void);

void vi_state_create_checkpoints(void);

void vi_state_new_category(char *name,
	vi_state_read_checkpoint_func_t read_checkpoint_func,
	vi_state_write_checkpoint_func_t write_checkpoint_func,
	void *user_data);
void vi_state_new_command(char *command_name,
	vi_state_process_trace_line_func_t process_trace_line_func,
	void *user_data);

struct vi_trace_line_t *vi_state_header_first(void);
struct vi_trace_line_t *vi_state_header_next(void);

struct vi_trace_line_t *vi_state_trace_line_first(long long cycle);
struct vi_trace_line_t *vi_state_trace_line_next(void);

void vi_state_go_to_cycle(long long cycle);


#endif


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

#ifndef ARCH_COMMON_ARCH_H
#define ARCH_COMMON_ARCH_H

#include <stdio.h>



/*
 * Architecture Object
 */


struct config_t;
struct arch_t;

extern struct str_map_t arch_sim_kind_map;

enum arch_sim_kind_t
{
	arch_sim_kind_invalid = 0,
	arch_sim_kind_functional,
	arch_sim_kind_detailed
};

typedef void (*arch_callback_func_t)(struct arch_t *arch, void *user_data);

typedef void (*arch_emu_init_func_t)(struct arch_t *arch);
typedef void (*arch_emu_done_func_t)(void);
typedef void (*arch_emu_dump_func_t)(FILE *f);
typedef void (*arch_emu_dump_summary_func_t)(FILE *f);

typedef void (*arch_timing_init_func_t)(void);
typedef void (*arch_timing_done_func_t)(void);
typedef void (*arch_timing_dump_func_t)(FILE *f);
typedef void (*arch_timing_dump_summary_func_t)(FILE *f);

typedef enum arch_sim_kind_t (*arch_run_func_t)(void);

typedef void (*arch_mem_config_default_func_t)(struct config_t *config);
typedef void (*arch_mem_config_parse_entry_func_t)(struct config_t *config, char *section);
typedef void (*arch_mem_config_check_func_t)(struct config_t *config);


struct arch_t
{
	/* Name of architecture (x86, ARM, etc.) */
	char *name;

	/* Prefix used in command-line options and in code variables and functions.
	 * E.g., 'evg' for Evergreen, or 'si' for Southern Islands. */
	char *prefix;

	/* Simulation kind - must be assigned externally */
	enum arch_sim_kind_t sim_kind;

	/* Call-back functions for emulator */
	arch_emu_init_func_t emu_init_func;
	arch_emu_done_func_t emu_done_func;
	arch_emu_dump_func_t emu_dump_func;
	arch_emu_dump_summary_func_t emu_dump_summary_func;
	arch_run_func_t emu_run_func;

	/* Call-back functions for timing simulator */
	arch_timing_init_func_t timing_init_func;
	arch_timing_done_func_t timing_done_func;
	arch_timing_dump_func_t timing_dump_func;
	arch_timing_dump_summary_func_t timing_dump_summary_func;
	arch_run_func_t timing_run_func;

	/* Function to run one iteration of the simulation loop. This is set to
	 * either 'emu_run_func' or 'timing_run_func', depending on the value of
	 * 'sim_kind'. */
	arch_run_func_t run_func;

	/* Call-back functions used in by the memory hierarchy */
	arch_mem_config_default_func_t mem_config_default_func;
	arch_mem_config_parse_entry_func_t mem_config_parse_entry_func;
	arch_mem_config_check_func_t mem_config_check_func;

	/* List of entry modules to the memory hierarchy. Each element of this list
	 * is of type 'mod_t'. */
	struct linked_list_t *mem_entry_mod_list;

	/* Timer for activity */
	struct m2s_timer_t *timer;

	/* Counters */
	long long cycle;
	long long inst_count;
};


struct arch_t *arch_create(char *name, char *prefix);
void arch_free(struct arch_t *arch);
void arch_dump(struct arch_t *arch, FILE *f);

void arch_dump_summary(struct arch_t *arch, FILE *f);




/*
 * Architecture List
 */



/*
 * Global
 */

void arch_init(void);
void arch_done(void);

void arch_register(char *name, char *prefix,
		enum arch_sim_kind_t sim_kind,
		arch_emu_init_func_t emu_init_func,
		arch_emu_done_func_t emu_done_func,
		arch_emu_dump_func_t emu_dump_func,
		arch_emu_dump_summary_func_t emu_dump_summary_func,
		arch_run_func_t emu_run_func,
		arch_timing_init_func_t timing_init_func,
		arch_timing_done_func_t timing_done_func,
		arch_timing_dump_func_t timing_dump_func,
		arch_timing_dump_summary_func_t timing_dump_summary_func,
		arch_run_func_t timing_run_func);

void arch_for_each(arch_callback_func_t callback_func, void *user_data);

struct arch_t *arch_get(char *name);
void arch_get_names(char *str, int size);

/* Return number of architectures performing timing simulation */
int arch_get_sim_kind_detailed_count(void);

/* Run one iteration (functional or timing) for each registered architecture,
 * and return the type of useful simulation as follows:
 *   - sim_arch_kind_detailed: at least one architecture performed detailed
 *     simulation.
 *   - sim_arch_kind_functional: no architecture performed detailed simulation,
 *     but at least one performed useful functional simulation.
 *   - sim_arch_kind_invalid: no architecture performed useful simulation, which
 *     means the the simulator main loop can finish.
 */
enum arch_sim_kind_t arch_run_all(void);

#endif

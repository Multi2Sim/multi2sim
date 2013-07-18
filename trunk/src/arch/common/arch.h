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

#include <lib/util/class.h>


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

typedef void (*arch_emu_init_func_t)(void);
typedef void (*arch_emu_done_func_t)(void);

typedef void (*arch_timing_read_config_func_t)(void);
typedef void (*arch_timing_init_func_t)(void);
typedef void (*arch_timing_done_func_t)(void);


struct arch_t
{
	/* Name of architecture (x86, ARM, etc.) */
	char *name;

	/* Prefix used in command-line options and in code variables and functions.
	 * E.g., 'evg' for Evergreen, or 'si' for Southern Islands. */
	char *prefix;

	/* Simulation kind - must be assigned externally */
	enum arch_sim_kind_t sim_kind;

	/* Flag indicating whether a simulation loop for this architecture
	 * performed a useful work. This flag is updated with the return value
	 * of functions 'emu_run_func' and 'timing_run_func'. */
	int active;

	/* Cycle in the local frequency domain when last timing simulation
	 * iteration was performed. If the main loop intends to run a new
	 * iteration, but the current cycle has not changed yet for this
	 * frequency domain, the iteration will be skipped. */
	long long last_timing_cycle;

	/* Call-back functions for emulator */
	arch_emu_init_func_t emu_init_func;
	arch_emu_done_func_t emu_done_func;

	/* Call-back functions for timing simulator */
	arch_timing_read_config_func_t timing_read_config_func;
	arch_timing_init_func_t timing_init_func;
	arch_timing_done_func_t timing_done_func;

	/* Disassembler */
	Asm *as;

	/* Emulator */
	Emu *emu;

	/* Timing simulator */
	Timing *timing;

	/* List of entry modules to the memory hierarchy. Each element of this list
	 * is of type 'mod_t'. */
	struct linked_list_t *mem_entry_mod_list;
};


struct arch_t *arch_create(char *name, char *prefix);
void arch_free(struct arch_t *arch);
void arch_dump(struct arch_t *arch, FILE *f);

void arch_set_asm(struct arch_t *arch, Asm *as);
void arch_set_emu(struct arch_t *arch, Emu *emu);
void arch_set_timing(struct arch_t *arch, Timing *timing);

void arch_dump_summary(struct arch_t *arch, FILE *f);



/*
 * Global Variables
 */

extern struct arch_t *arch_arm;
extern struct arch_t *arch_evergreen;
extern struct arch_t *arch_fermi;
extern struct arch_t *arch_mips;
extern struct arch_t *arch_southern_islands;
extern struct arch_t *arch_x86;




/*
 * Public Functions
 */

void arch_init(void);
void arch_done(void);

struct arch_t *arch_register(char *name, char *prefix,
		enum arch_sim_kind_t sim_kind,
		arch_emu_init_func_t emu_init_func,
		arch_emu_done_func_t emu_done_func,
		arch_timing_read_config_func_t timing_read_config_func,
		arch_timing_init_func_t timing_init_func,
		arch_timing_done_func_t timing_done_func);

void arch_for_each(arch_callback_func_t callback_func, void *user_data);

struct arch_t *arch_get(char *name);
void arch_get_names(char *str, int size);

/* Return number of architectures performing timing simulation */
int arch_get_sim_kind_detailed_count(void);

/* Run one simulation loop iteration (functional or timing) for each
 * registered architecture. This function is called in every iteration of the
 * main loop working at the highest frequency. It an architecture with timing
 * simulation and lower frequency should actually not run its iteration, it
 * will not do it.
 * This function returns the number of architectures doing an effective
 * emulation ('num_emu_active') and timing simulation ('num_timing_active').
 * These values are used in the main loop to decide whether to exit and
 * finish simulation, or whether to call the event-driven simulation.
 */
void arch_run(int *num_emu_active_ptr, int *num_timing_active_ptr);

#endif

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


struct config_t;

extern struct str_map_t arch_sim_kind_map;

enum arch_sim_kind_t
{
	arch_sim_kind_invalid = 0,
	arch_sim_kind_functional,
	arch_sim_kind_detailed
};

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

	/* Callback functions used in by the memory hierarchy */
	arch_mem_config_default_func_t mem_config_default_func;
	arch_mem_config_parse_entry_func_t mem_config_parse_entry_func;
	arch_mem_config_check_func_t mem_config_check_func;

	/* List of entry modules to the memory hierarchy. Each element of this list
	 * is of type 'mod_t'. */
	struct linked_list_t *mem_entry_mod_list;
};


struct arch_t *arch_create(char *name, char *prefix);
void arch_free(struct arch_t *arch);
void arch_dump(struct arch_t *arch, FILE *f);


#endif

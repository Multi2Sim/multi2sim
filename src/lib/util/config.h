/*
 *  Libstruct
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

#ifndef LIB_UTIL_CONFIG_H
#define LIB_UTIL_CONFIG_H

struct config_t;

/* Creation and destruction */
struct config_t *config_create(char *file_name);
void config_free(struct config_t *config);

/* Get the file name */
char *config_get_file_name(struct config_t *config);

/* Load/save configuration */
void config_load(struct config_t *config);
void config_save(struct config_t *config);

/* Ask for section of variable existence */
int config_section_exists(struct config_t *config, char *section);
int config_var_exists(struct config_t *config, char *section, char *var);

/* Remove a variable/section;
 * Return value: non-0=ok, 0=variable/section does not exist */
int config_section_remove(struct config_t *config, char *section);
int config_var_remove(struct config_t *config, char *section, char *var);

/* Enumeration of sections */
char *config_section_first(struct config_t *config);
char *config_section_next(struct config_t *config);
#define CONFIG_SECTION_FOR_EACH(config, section) \
	for (section = config_section_first(config); section; \
		section = config_section_next(config))

/* Add variables in a section; if section does not exists, it is created;
 * If variable already exists, replace old value;
 * String values are strdup'ped, so they can be modified in user program */
void config_write_string(struct config_t *config, char *section, char *var, char *value);
void config_write_int(struct config_t *config, char *section, char *var, int value);
void config_write_llint(struct config_t *config, char *section, char *var, long long value);
void config_write_bool(struct config_t *config, char *section, char *var, int value);
void config_write_double(struct config_t *config, char *section, char *var, double value);
void config_write_enum(struct config_t *config, char *section, char *var, int value, char **map, int map_count);
void config_write_ptr(struct config_t *config, char *section, char *var, void *value);

/* Read variables from a section.
 * If a section or variable does not exist, the default value in 'def' is returned.
 * Sections and variables given in 'config_read_XXX' functions are automatically added
 *   into the set of allowed sections/variables for the configuration file. */
char *config_read_string(struct config_t *config, char *section, char *var, char *def);
int config_read_int(struct config_t *config, char *section, char *var, int def);
long long config_read_llint(struct config_t *config, char *section, char *var, long long def);
int config_read_bool(struct config_t *config, char *section, char *var, int def);
double config_read_double(struct config_t *config, char *section, char *var, double def);
int config_read_enum(struct config_t *config, char *section, char *var, int def, char **map, int map_count);
void *config_read_ptr(struct config_t *config, char *section, char *var, void *def);

/* Defining and checking allowed/mandatory sections/keys in file */
void config_section_allow(struct config_t *config, char *section);
void config_section_enforce(struct config_t *config, char *section);
void config_var_allow(struct config_t *config, char *section, char *key);
void config_var_enforce(struct config_t *config, char *section, char *key);
void config_check(struct config_t *config);
void config_section_check(struct config_t *config, char *section);


#endif


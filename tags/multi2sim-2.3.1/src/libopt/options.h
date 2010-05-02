/*
 *  Libopt
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

#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdio.h>
#include <stdint.h>

/* Initialization and destruction */
void opt_init();
void opt_done();


/* Variables registration */
#define opt_reg_int32(name, desc, var) opt_reg_int32_list(name, desc, var, 1, NULL)
#define opt_reg_uint32(name, desc, var) opt_reg_uint32_list(name, desc, var, 1, NULL)
#define opt_reg_int64(name, desc, var) opt_reg_int64_list(name, desc, var, 1, NULL)
#define opt_reg_uint64(name, desc, var) opt_reg_uint64_list(name, desc, var, 1, NULL)
#define opt_reg_float(name, desc, var) opt_reg_float_list(name, desc, var, 1, NULL)
#define opt_reg_double(name, desc, var) opt_reg_double_list(name, desc, var, 1, NULL)
#define opt_reg_string(name, desc, var) opt_reg_string_list(name, desc, var, 1, NULL)
#define opt_reg_bool(name, desc, var) opt_reg_bool_list(name, desc, var, 1, NULL)
#define opt_reg_enum(name, desc, var, map, mapcount) \
	opt_reg_enum_list(name, desc, var, 1, NULL, map, mapcount)


/* Variable lists.
 * The number of elements input by the user is returned in '*nelem'.
 * This number must be equal to 'nvars', except when 'nvars' is 0. In this case,
 * the list has a variable length up to a maximum of 32 items. */
void opt_reg_int32_list(char *name, char *desc, int32_t *vars, int nvars, int *nelem);
void opt_reg_uint32_list(char *name, char *desc, uint32_t *vars, int nvars, int *nelem);
void opt_reg_int64_list(char *name, char *desc, int64_t *vars, int nvars, int *nelem);
void opt_reg_uint64_list(char *name, char *desc, uint64_t *vars, int nvars, int *nelem);
void opt_reg_float_list(char *name, char *desc, float *vars, int nvars, int *nelem);
void opt_reg_double_list(char *name, char *desc, double *vars, int nvars, int *nelem);
void opt_reg_string_list(char *name, char *desc, char **vars, int nvars, int *nelem);
void opt_reg_bool_list(char *name, char *desc, int *vars, int nvars, int *nelem);
void opt_reg_enum_list(char *name, char *desc, int *vars, int nvars, int *nelem,
	char **map, int mapcount);


/* Operations with options database */
void opt_print_options(FILE *f);
void opt_check_options(int *argc, char **argv);
void opt_check_config(char *file);


/* Return the double value of a registered option */
double opt_get_option(char *name);


#endif

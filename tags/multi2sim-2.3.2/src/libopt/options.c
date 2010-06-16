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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <mhandle.h>

#include "options.h"


/* global private options data base */
struct odb_t {
	struct opt_t *options;
	char *header;
	struct opt_note_t *notes;
};

static struct odb_t *odb = NULL;
static char *vtlargv = NULL;


/* option kinds */
enum opt_class_t {
	oc_int32 = 0,
	oc_uint32,
	oc_int64,
	oc_uint64,
	oc_float,
	oc_double,
	oc_string,
	oc_bool,
	oc_enum
};


struct opt_t {
	struct opt_t *next;
	enum opt_class_t oc;
	char *name;
	char *desc;
	void *vars;
	int nvars;
	int *nelem;
	char *format;
	char **map;	/* oc_enum - possible strings */
	int mapcount;	/* oc_enum - number of possible strings */
};


struct opt_note_t {
	struct	opt_note_t *next;
	char	*note;
};


static struct opt_t *new_option(enum opt_class_t oc, char *name, char *desc,
	void *vars, int nvars, int *nelem, char *format)
{
	struct	opt_t *opt, *elt, *prev;

	if (!odb)
		return NULL;
	
	opt = calloc(1, sizeof(struct opt_t));
	opt->oc = oc;
	opt->name = name;
	opt->desc = desc;
	opt->vars = vars;
	opt->nvars = nvars;
	opt->nelem = nelem;
	opt->format = format;
	
	if (opt->name[0] != '-') {
		fprintf(stderr, "error: option '%s' does not start with '-'\n", opt->name);
		abort();
	}
		
	for (prev = NULL, elt = odb->options;
		elt;
		prev = elt, elt = elt->next)
	{
		if (!strcmp(elt->name, opt->name)) {
			fprintf(stderr, "error: option '%s' redefined\n", opt->name);
			abort();
		}
	}
	
	opt->next = NULL;
	if (prev) prev->next = opt;
	else odb->options = opt;
	return opt;
}


/* returns the index of 'val' in the value list */
static int strtoenum(char *val, char **map, int mapcount)
{
	int i;
	char s[100];
	
	/* looks for val in the value list */
	for (i = 0; i < mapcount; i++)
		if (!strcmp(val, map[i]))
			return i;
	
	/* error if not found */
	strcpy(s, "");
	for (i = 0; i < mapcount; i++) {
		if (i)
			strcat(s, "|");
		strcat(s, map[i]);
	}
	fprintf(stderr, "error: value '%s' not valid; possible values are {%s}\n", val, s);
	exit(1);
	return 0; /* avoid warning */
}


/* Creation and destruction */
void opt_init()
{
	odb = calloc(1, sizeof(struct odb_t));
}


void opt_done()
{
	struct opt_t *elt, *next;
	
	if (!odb)
		return;
	
	/* releas options & data base */
	elt = odb->options;
	while (elt) {
		next = elt->next;
		free(elt);
		elt = next;
	}
	free(odb);
	if (vtlargv)
		free(vtlargv);
	odb = NULL;
	vtlargv = NULL;
}


void opt_reg_int32_list(char *name, char *desc, int32_t *vars, int nvars, int *nelem)
{
	new_option(oc_int32, name, desc, vars, nvars, nelem, "%d");
}


void opt_reg_uint32_list(char *name, char *desc, uint32_t *vars, int nvars, int *nelem)
{
	new_option(oc_uint32, name, desc, vars, nvars, nelem, "%u");
}


void opt_reg_int64_list(char *name, char *desc, int64_t *vars, int nvars, int *nelem)
{
	new_option(oc_int64, name, desc, vars, nvars, nelem, "%lld");
}


void opt_reg_uint64_list(char *name, char *desc, uint64_t *vars, int nvars, int *nelem)
{
	new_option(oc_uint64, name, desc, vars, nvars, nelem, "%llu");
}


void opt_reg_float_list(char *name, char *desc, float *vars, int nvars, int *nelem)
{
	new_option(oc_float, name, desc, vars, nvars, nelem, "%f");
}


void opt_reg_double_list(char *name, char *desc, double *vars, int nvars, int *nelem)
{
	new_option(oc_double, name, desc, vars, nvars, nelem, "%f");
}


void opt_reg_string_list(char *name, char *desc, char **vars, int nvars, int *nelem)
{
	new_option(oc_string, name, desc, vars, nvars, nelem, "%s");
}


void opt_reg_bool_list(char *name, char *desc, int *vars, int nvars, int *nelem)
{
	new_option(oc_bool, name, desc, vars, nvars, nelem, NULL);
}


void opt_reg_enum_list(char *name, char *desc, int *vars, int nvars, int *nelem,
	char **map, int mapcount)
{
	struct opt_t *opt;
	opt = new_option(oc_enum, name, desc, vars, nvars, nelem, NULL);
	opt->map = map;
	opt->mapcount = mapcount;
}


void opt_print_options(FILE *f)
{
	struct opt_t *opt;
	char val[128], s[128];
	int i, nelem;
	
	if (!odb)
		return;
	
	/* key */
	fprintf(f, "option                    value        # description\n");
	for (i = 0; i < 60; i++)
		fprintf(f, "-");
	fprintf(f, "\n");
	
	/* options */
	for (opt = odb->options; opt; opt = opt->next) {
		
		nelem = opt->nelem ? *opt->nelem : opt->nvars;
		fprintf(f, "%-25s ", opt->name);
		*val = 0;
		
		switch (opt->oc) {

			case oc_int32: {
				int32_t *vars = opt->vars;
				for (i = 0; i < nelem; i++) {
					sprintf(s, opt->format, vars[i]);
					strcat(val, s);
					if (i < nelem - 1) strcat(val, " ");
				}
				break;
			}
			
			case oc_uint32: {
				uint32_t *vars = opt->vars;
				for (i = 0; i < nelem; i++) {
					sprintf(s, opt->format, vars[i]);
					strcat(val, s);
					if (i < nelem - 1) strcat(val, " ");
				}
				break;
			}
			
			case oc_int64: {
				int64_t *vars = opt->vars;
				for (i = 0; i < nelem; i++) {
					sprintf(s, opt->format, vars[i]);
					strcat(val, s);
					if (i < nelem - 1) strcat(val, " ");
				}
				break;
			}
			
			case oc_uint64: {
				uint64_t *vars = opt->vars;
				for (i = 0; i < nelem; i++) {
					sprintf(s, opt->format, vars[i]);
					strcat(val, s);
					if (i < nelem - 1) strcat(val, " ");
				}
				break;
			}
			
			case oc_float: {
				float *vars = opt->vars;
				for (i = 0; i < nelem; i++) {
					sprintf(s, opt->format, (double) vars[i]);
					strcat(val, s);
					if (i < nelem - 1) strcat(val, " ");
				}
				break;
			}
			
			case oc_double: {
				double *vars = opt->vars;
				for (i = 0; i < nelem; i++) {
					sprintf(s, opt->format, vars[i]);
					strcat(val, s);
					if (i < nelem - 1) strcat(val, " ");
				}
				break;
			}
			
			case oc_string: {
				char **vars = opt->vars;
				for (i = 0; i < nelem; i++) {
					sprintf(s, opt->format, vars[i]);
					strcat(val, s);
					if (i < nelem - 1) strcat(val, " ");
				}
				break;
			}
			
			case oc_bool: {
				int *vars = opt->vars;
				for (i = 0; i < nelem; i++) {
					sprintf(s, "%s", vars[i] ? "t" : "f");
					strcat(val, s);
					if (i < nelem - 1) strcat(val, " ");
				}
				break;
			}
			
			case oc_enum: {
				int *vars = opt->vars;
				for (i = 0; i < nelem; i++) {
					sprintf(s, "%s", opt->map[vars[i]]);
					strcat(val, s);
					if (i < nelem - 1) strcat(val, " ");
				}
				break;
			}
		}
		
		fprintf(f, "%12s # %s\n", val, opt->desc);
	}
	fprintf(f, "\n");
}




/* Read command line until the current argument is not an option.
 * Modify argc and argv */
#define IS_OPT(X) (argv[X][0] == '-')
#define LAST_ARG(X) ((X) == (*argc - 1) || IS_OPT((X) + 1))

void opt_check_options(int *argc, char **argv)
{
	int i, nelem = 0, new_argc;
	struct opt_t *opt = NULL;
	
	/* no args */
	if (!odb || *argc <= 1)
		return;
	
	for (i = 1; i < *argc; i++) {
		
		if (IS_OPT(i)) {
	
			for (opt = odb->options; opt; opt = opt->next)
				if (!strcmp(opt->name, argv[i]))
					break;
			if (!opt) {
				fprintf(stderr, "error: %s: invalid option\n", argv[i]);
				exit(1);
			}
			if (LAST_ARG(i)) {
				fprintf(stderr, "error: %s: option without arguments\n", opt->name);
				exit(1);
			}
				
			/* argument counter */
			nelem = 0;
			if (opt->nelem)
				*opt->nelem = 0;
			continue;
		}
		else
		{
			/* no option */
			if (i == 1)
				break;
		}
		
		/* store argument depending on option kind */
		switch (opt->oc) {
		
			case oc_int32: {
				int32_t *pvar = & ((int32_t *) opt->vars)[nelem];
				if (strlen(argv[i]) >= 2 && argv[i][0] == '0' &&
					(argv[i][1] == 'x' || argv[i][1] == 'X'))
					sscanf(argv[i] + 2, "%x", pvar);
				else
					sscanf(argv[i], "%d", pvar);
				break;
			}
			
			case oc_uint32: {
				uint32_t *pvar = & ((uint32_t *) opt->vars)[nelem];
				if (strlen(argv[i]) >= 2 && argv[i][0] == '0' &&
					(argv[i][1] == 'x' || argv[i][1] == 'X'))
					sscanf(argv[i] + 2, "%x", pvar);
				else
					sscanf(argv[i], "%u", pvar);
				break;
			}
			
			case oc_int64: {
				int64_t *pvar = & ((int64_t *) opt->vars)[nelem];
				if (strlen(argv[i]) >= 2 && argv[i][0] == '0' &&
					(argv[i][1] == 'x' || argv[i][1] == 'X'))
					sscanf(argv[i] + 2, "%llx", (long long unsigned int *) pvar);
				else
					sscanf(argv[i], "%lld", (long long unsigned int *) pvar);
				break;
			}
			
			case oc_uint64: {
				uint64_t *pvar = & ((uint64_t *) opt->vars)[nelem];
				if (strlen(argv[i]) >= 2 && argv[i][0] == '0' &&
					(argv[i][1] == 'x' || argv[i][1] == 'X'))
					sscanf(argv[i] + 2, "%llx", (long long unsigned int *) pvar);
				else
					sscanf(argv[i], "%llu", (long long unsigned int *) pvar);
				break;
			}
			
			case oc_float: {
				float *pvar = & ((float *) opt->vars)[nelem];
				sscanf(argv[i], "%f", pvar);
				break;
			}
			
			case oc_double: {
				double *pvar = & ((double *) opt->vars)[nelem];
				sscanf(argv[i], "%lf", pvar);
				break;
			}
			
			case oc_string: {
				char **pvar = & ((char **) opt->vars)[nelem];
				*pvar = argv[i];
				break;
			}
		
			case oc_bool: {
				int *pvar = & ((int *) opt->vars)[nelem];
				if (!strcasecmp(argv[i], "t"))
					*pvar = 1;
				else if (!strcasecmp(argv[i], "f"))
					*pvar = 0;
				else {
					fprintf(stderr, "error: %s: option can be 't' or 'f'\n", opt->name);
					exit(1);
				}
				break;
			}
			
			case oc_enum: {
				int *pvar = & ((int *) opt->vars)[nelem];
				*pvar = strtoenum(argv[i], opt->map, opt->mapcount);
				break;
			}
		}
		
		/* update args counter */
		nelem++;
		if (opt->nelem)
			*opt->nelem = nelem;
		if (LAST_ARG(i) && nelem < opt->nvars) {
			fprintf(stderr, "error: %s: option requires %d argument(s)\n", opt->name, opt->nvars);
			exit(1);
		}
		if (!LAST_ARG(i) && nelem >= opt->nvars) {
			i++;
			break;
		}
	}
	
	/* update argc and argv */
	new_argc = *argc - i + 1;
	for (i = 1; i < new_argc; i++)
		argv[i] = argv[i + *argc - new_argc];
	*argc = new_argc;
}



/* Read configuration from a file */
void opt_check_config(char *cfg_file)
{
	FILE *f;
	char **argv, *argvcurr, buf[1000], *line, *end;
	int argc, maxargc = 200, len;
	struct stat fs;
	
	if (!odb || !cfg_file || !*cfg_file)
		return;
	if (vtlargv) {
		fprintf(stderr, "error: cannot load more than one config file\n");
		exit(1);
	}
	
	/* open file */
	lstat(cfg_file, &fs);
	f = fopen(cfg_file, "rt");
	if (!f) {
		fprintf(stderr, "error: %s: cannot open config file\n", cfg_file);
		exit(1);
	}
	
	/* read arguments */
	argc = 1;
	argv = (char **) calloc(maxargc, sizeof(char *));
	vtlargv = (char *) malloc(fs.st_size + 1);
	argv[0] = "sim";
	argvcurr = vtlargv;
	while (1) {
	
		/* read a line in the file */
		line = fgets(buf, 1000, f);
		if (!line || feof(f))
			break;

		/* comment */
		if (*line == '#')
			continue;
		
		/* extract arguments */
		while (*line) {
			
			/* erase spaces or end of line */
			while (*line == '\n' || *line == ' ')
				line++;
			if (!*line)
				break;
		
			/* maxarg exceeded? */
			if (argc == maxargc) {
				fprintf(stderr, "error: %s: too many arguments\n", cfg_file);
				exit(1);
			}
			
			/* get argument length */
			end = index(line, ' ');
			len = end ? end - line : strlen(line);
			if (line[len - 1] == '\n') len--;
			
			/* store argument */
			argv[argc] = argvcurr;
			strncpy(argvcurr, line, len);
			argvcurr[len] = 0;
			argvcurr += len + 1;
			line += len;
			argc++;
		}
	}
	
	/* close & analyze arguments */
	fclose(f);
	opt_check_options(&argc, argv);
	free(argv);
}


/* Return the double value of a registered option.
 * If it is a list, return the value of the first element. */
double opt_get_option(char *name)
{
	struct opt_t *opt;
	
	if (!odb)
		return 0.0;
	
	for (opt = odb->options; opt; opt = opt->next) {
		if (!strcmp(opt->name + 1, name)) {
			
			switch (opt->oc) {
				case oc_int32:
					return * (int32_t *) opt->vars;
				case oc_uint32:
					return * (uint32_t *) opt->vars;
				case oc_int64:
					return * (int64_t *) opt->vars;
				case oc_uint64:
					return * (uint64_t *) opt->vars;
				case oc_float:
					return * (float *) opt->vars;
				case oc_double:
					return * (double *) opt->vars;
				default:
					fprintf(stderr, "error: %s: cannot get double value for option\n", opt->name);
					abort();
			}
		}
	}
	fprintf(stderr, "error: %s: option does not exist\n", name);
	abort();
	return 0; /* avoid warning */
}

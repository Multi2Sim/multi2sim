/*
 *  Libstruct
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

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <mhandle.h>
#include "chrono.h"
#include "list.h"


#define CHRONO_STOPPED		0
#define CHRONO_RUNNING		1


struct chrono_t {
	int status;
	double ellapsed;
	double start;
	char *name;
};


static double current_time()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double) tv.tv_sec + (double) tv.tv_usec / 1.0e6;
}


struct chrono_t *chrono_create(char *name)
{
	struct chrono_t *chrono;
	chrono = calloc(1, sizeof(struct chrono_t));
	chrono->name = name;
	return chrono;
}


void chrono_free(struct chrono_t *chrono)
{
	free(chrono);
}


void chrono_start(struct chrono_t *chrono)
{
	if (chrono->status == CHRONO_RUNNING)
		return;
	chrono->status = CHRONO_RUNNING;
	chrono->start = current_time();
}


void chrono_stop(struct chrono_t *chrono)
{
	double now, ellapsed;
	if (chrono->status == CHRONO_STOPPED)
		return;
	chrono->status = CHRONO_STOPPED;
	now = current_time();
	ellapsed = now - chrono->start;
	chrono->ellapsed += ellapsed;
}


void chrono_reset(struct chrono_t *chrono)
{
	chrono->ellapsed = 0.0;
	chrono->start = current_time();
}


double chrono_ellapsed(struct chrono_t *chrono)
{
	return chrono->status == CHRONO_STOPPED ?
		chrono->ellapsed :
		chrono->ellapsed + (current_time() - chrono->start);
}


char *chrono_name(struct chrono_t *chrono)
{
	return chrono->name;
}


struct chrono_list_t
{
	struct list_t *list;
};


struct chrono_list_t *chrono_list_create()
{
	struct chrono_list_t *chrono_list;
	chrono_list = calloc(1, sizeof(struct chrono_list_t));
	if (!chrono_list)
		return NULL;
	chrono_list->list = list_create(10);
	if (!chrono_list->list) {
		free(chrono_list);
		return NULL;
	}
	return chrono_list;
}


void chrono_list_free(struct chrono_list_t *chrono_list)
{
	int i;
	struct chrono_t *chrono;
	for (i = 0; i < list_count(chrono_list->list); i++) {
		chrono = list_get(chrono_list->list, i);
		chrono_free(chrono);
	}
	list_free(chrono_list->list);
	free(chrono_list);
}


int chrono_list_new(struct chrono_list_t *chrono_list, char *name)
{
	struct chrono_t *chrono;
	chrono = chrono_create(name);
	list_add(chrono_list->list, chrono);
	return list_count(chrono_list->list) - 1;
}


void chrono_list_start(struct chrono_list_t *chrono_list, int idx)
{
	struct chrono_t *chrono;
	if (idx < 0 || idx >= list_count(chrono_list->list))
		return;
	chrono = list_get(chrono_list->list, idx);
	chrono_start(chrono);
}


void chrono_list_stop(struct chrono_list_t *chrono_list, int idx)
{
	struct chrono_t *chrono;
	if (idx < 0 || idx >= list_count(chrono_list->list))
		return;
	chrono = list_get(chrono_list->list, idx);
	chrono_stop(chrono);
}


void chrono_list_dump(struct chrono_list_t *chrono_list, FILE *f)
{
	int i;
	double total = 0.0;
	struct chrono_t *chrono;
	for (i = 0; i < list_count(chrono_list->list); i++) {
		chrono = list_get(chrono_list->list, i);
		total += chrono_ellapsed(chrono);
	}
	if (total == 0.0)
		total = 1.0;
	for (i = 0; i < list_count(chrono_list->list); i++) {
		chrono = list_get(chrono_list->list, i);
		fprintf(f, "chronometer %s: ellapsed=%fs, (%.4f)\n",
			chrono_name(chrono),
			chrono_ellapsed(chrono),
			chrono_ellapsed(chrono) / total);
	}
}

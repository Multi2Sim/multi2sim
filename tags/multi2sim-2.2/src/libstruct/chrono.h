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

#ifndef CHRONO_H
#define CHRONO_H

#include <stdio.h>

struct chrono_t;

struct chrono_t *chrono_create();
void chrono_free(struct chrono_t *chrono);

void chrono_start(struct chrono_t *chrono);
void chrono_stop(struct chrono_t *chrono);
void chrono_reset(struct chrono_t *chrono);
double chrono_ellapsed(struct chrono_t *chrono);
char *chrono_name(struct chrono_t *chrono);


struct chrono_list_t;

struct chrono_list_t *chrono_list_create();
void chrono_list_free(struct chrono_list_t *chrono_list);

int chrono_list_new(struct chrono_list_t *chrono_list, char *name);
void chrono_list_start(struct chrono_list_t *chrono_list, int idx);
void chrono_list_stop(struct chrono_list_t *chrono_list, int idx);
void chrono_list_dump(struct chrono_list_t *chrono_list, FILE *f);

#endif

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

#ifndef LIB_UTIL_REPOS_H
#define LIB_UTIL_REPOS_H

#include <stdio.h>

/* Object repository */
struct repos_t;

/* Creation/destruction */
struct repos_t *repos_create(int objsize, char *name);
void repos_free(struct repos_t *repos);
void repos_free_dump(struct repos_t *repos, void(*dump)(void *, FILE *));

/* Functions to create and free repository objects.
 * The first time an object is created, its memory is allocated
 * with malloc(). When it is freed, no call to free() is made;
 * the object is instead inserted into the repository to be
 * fast returned in a subsequent call to repos_create_object.
 * All objects are free()d when a call to repos_free is made */
void *repos_create_object(struct repos_t *repos);
void repos_free_object(struct repos_t *repos, void *obj);

/* Return non-0 if an object is allocated (not freed) in the
 * specified repository. The object must have been created
 * with the repository */
int repos_allocated_object(struct repos_t *repos, void *obj);

#endif

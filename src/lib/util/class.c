/*
 *  Multi2Sim
 *  Copyright (C) 2013  Rafael Ubal (ubal@ece.neu.edu)
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

#include <assert.h>

#include "class.h"
#include "debug.h"


void class_init(struct class_t *class_info, unsigned int id,
		void *parent)
{
	struct class_t *parent_class_info;

	/* Set class identifier */
	class_info->id = id;

	/* Magic */
	class_info->magic[0] = 'C';
	class_info->magic[1] = 'L';
	class_info->magic[2] = 'A';
	class_info->magic[3] = 'S';

	/* Parent */
	if (parent)
	{
		/* Parent instantce is a class */
		parent_class_info = CLASS(parent);

		/* Parent cannot have a child yet */
		if (parent_class_info->child)
			panic("%s: parent class already has a child",
					__FUNCTION__);

		/* Set parent */
		parent_class_info->child = class_info;
		class_info->parent = parent_class_info;
	}
}


struct class_t *class_get(void *p)
{
	struct class_t *class_info;
	int is_class;

	/* Convert */
	class_info = p;
	is_class = class_info->magic[0] == 'C' &&
			class_info->magic[1] == 'L' &&
			class_info->magic[2] == 'A' &&
			class_info->magic[3] == 'S';
	
	/* Not a class */
	if (!is_class)
		panic("%s: not a class instance", __FUNCTION__);

	/* Converted */
	return class_info;
}


void *class_reinterpret_cast(void *p, unsigned int id)
{
	/* NULL is cast to NULL */
	if (!p)
		return NULL;
	
	/* Try to reinterpret */
	p = class_try_reinterpret_cast(p, id);

	/* Invalid cast */
	if (!p)
		panic("%s: invalid cast", __FUNCTION__);
	
	/* Return cast instance */
	return p;
}


void *class_try_reinterpret_cast(void *p, unsigned int id)
{
	struct class_t *class_info;
	struct class_t *tmp_class_info;

	/* NULL is cast to NULL */
	if (!p)
		return NULL;

	/* Get class */
	class_info = CLASS(p);

	/* Search parents */
	for (tmp_class_info = class_info; tmp_class_info;
			tmp_class_info = tmp_class_info->parent)
		if (tmp_class_info->id == id)
			return tmp_class_info;
	
	/* Search children */
	for (tmp_class_info = class_info->child; tmp_class_info;
			tmp_class_info = tmp_class_info->child)
		if (tmp_class_info->id == id)
			return tmp_class_info;

	/* Invalid cast */
	return NULL;
}

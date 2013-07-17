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

#include <lib/mhandle/mhandle.h>

#include "class.h"
#include "debug.h"


/*
 * Class 'Object'
 */

struct class_t ObjectClass;

void ObjectCreate(Object *self)
{
	/* Virtual functions */
	self->Dump = ObjectDump;
}


void ObjectDestroy(Object *self)
{
}


void ObjectDump(Object *self, FILE *f)
{
	struct class_info_t *info;
	char *comma;

	fprintf(f, "Object @%p: ", self);
	info = &self->__info;
	comma = "";
	while (info)
	{
		assert(info->c);
		fprintf(f, "%s%s", comma, info->c->name);
		info = info->child;
		comma = " <- ";
	}
	fprintf(f, "\n");
}




/*
 * Class functions
 */

struct class_t *class_list_head;
struct class_t *class_list_tail;


void class_init(void)
{
	Object o;

	/* Already initialized */
	if (ObjectClass.id)
		return;
	
	/* Initialize object class */
	ObjectClass.name = "Object";
	ObjectClass.id = class_compute_id(ObjectClass.name);
	ObjectClass.size = sizeof(Object);
	ObjectClass.info_offset =  (unsigned int) ((void *) &o.__info
			- (void *) &o);
	assert(ObjectClass.info_offset == 0);
	ObjectClass.parent = NULL;
	ObjectClass.destroy = (void (*)(void *)) ObjectDestroy;

	/* Insert to class list */
	assert(!class_list_head);
	assert(!class_list_tail);
	class_list_head = &ObjectClass;
	class_list_tail = &ObjectClass;
}


void class_register(struct class_t *c)
{
	/* Check that class has not been registered before */
	if (c->id)
		panic("%s: class '%s' registered twice",
			__FUNCTION__, c->name);
	
	/* Compute class ID */
	assert(c->name && *c->name);
	c->id = class_compute_id(c->name);
	assert(c->id);
	
	/* Insert into class list */
	assert(class_list_head);
	assert(class_list_tail);
	class_list_tail->next = c;
	class_list_tail = c;
}


unsigned int class_compute_id(char *name)
{
	unsigned char *str;
	unsigned int prime;
	unsigned int id;

	/* Hash function */
	str = (unsigned char *) name;
	id = 5381;
	prime = 16777619;
	while (*str)
	{
		id = (id ^ *str) * prime;
		str++;
	}

	/* Return calculated hash */
	return id;
}


void *class_new(struct class_t *c)
{
	struct class_info_t *info;
	struct class_info_t *child_info;
	struct class_info_t *parent_info;
	void *p;

	/* Allocate */
	p = xcalloc(1, c->size);

	/* Initialize 'class_info' fields */
	child_info = NULL;
	info = (struct class_info_t *) (p + c->info_offset);
	do
	{
		/* Class must be registered */
		if (!c->id)
			panic("%s: class has not been registered "
				" - use CLASS_REGISTER()",
				__FUNCTION__);

		/* Get parent class info */
		parent_info = c->parent ?
			(struct class_info_t *) (p + c->parent->info_offset)
			: NULL;

		/* Get current class info and initialize */
		info = (struct class_info_t *) (p + c->info_offset);
		info->c = c;
		info->parent = parent_info;
		info->child = child_info;

		/* Move to parent class */
		c = c->parent;
		child_info = info;
		info = parent_info;
	} while (c);

	/* Return */
	return p;
}


void class_delete(void *p)
{
	struct class_info_t *info;

	/* Find the child-most destructor */
	info = &((Object *) p)->__info;
	while (info->child)
		info = info->child;
	
	/* Call destructors in child-to-parent order */
	while (info)
	{
		assert(info->c);
		assert(info->c->destroy);
		info->c->destroy(p);
		info = info->parent;
	}

	/* Free memory */
	free(p);
}


int class_instance_of(void *p, struct class_t *c)
{
	struct class_info_t *info;

	/* NULL pointer always casts successfully */
	assert(c);
	if (!p)
		return 1;

	/* Find child with matching ID */
	info = &((Object *) p)->__info;
	while (info)
	{
		assert(info->c);
		if (info->c->id == c->id)
			return 1;
		info = info->child;
	}

	/* Not found */
	return 0;
}


static void class_dump_parents(struct class_t *c, FILE *f)
{
	char *comma;

	/* No parent */
	if (!c->parent)
	{
		fprintf(f, "-\n");
		return;
	}

	/* Parent list */
	c = c->parent;
	comma = "";
	while (c)
	{
		fprintf(f, "%s%s", comma, c->name);
		c = c->parent;
		comma = " -> ";
	}
	fprintf(f, "\n");
}


struct class_t *class_of(void *p)
{
	struct class_info_t *info;

	/* Find the child-most destructor */
	info = &((Object *) p)->__info;
	assert(info->c);
	while (info->child)
	{
		info = info->child;
		assert(info->c);
	}
	
	/* Return the class */
	return info->c;
}


void class_dump(FILE *f)
{
	struct class_t *c;

	fprintf(f, "\nRegistered classes:\n\n");
	c = class_list_head;
	while (c)
	{
		/* Name */
		fprintf(f, "[ Class %s ]\n", c->name);

		/* Inheritance */
		fprintf(f, "Parents = ");
		class_dump_parents(c, f);

		/* Other fields */
		fprintf(f, "ID = 0x%x\n", c->id);
		fprintf(f, "Size = %u\n", c->size);
		fprintf(f, "InfoOffset = %u\n", c->info_offset);

		/* Next */
		c = c->next;
		fprintf(f, "\n");
	}
}


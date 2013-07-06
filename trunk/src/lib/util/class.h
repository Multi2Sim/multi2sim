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

#ifndef LIB_UTIL_CLASS_H
#define LIB_UTIL_CLASS_H

/* Cast class instance into 'class_t' */
#ifndef NDEBUG
#define CLASS(p) (class_get((p)))
#else
#define CLASS(p) ((struct class_t *) (p))
#endif

/* Cast a class instance into any of its child/parent classes. If the cast is
 * invalid, program execution is aborted with a panic message. */
#define CLASS_REINTERPRET_CAST(instance, id, type) \
	((type *) class_reinterpret_cast((instance), (id)))

/* Return true if the instance can be safely casted into type 'id' using macro
 * CLASS_REINTERPRET_CAST */
#define CLASS_OF(instance, id) \
	(class_try_reinterpret_cast((instance), (id)) != NULL)

/* To be used in the constructor */
#define CLASS_INIT(instance, id, parent) \
	class_init(&(instance)->class_info, (id), (parent))


/* Every structure considering itself a class must have a field of type
 * 'struct class_t' (notice no pointer) as its first field. */
struct class_t
{
	/* Magic word identifying a class instance */
	char magic[4];  /* = [ 'C', 'L', 'A', 'S' ] */

	/* Unique class identifier */
	unsigned int id;

	/* Inheritance information */
	struct class_t *parent;
	struct class_t *child;
};

/* A class constructor must call this function by passing its 'class_t' first
 * field by reference.
 * The value in 'id' is a unique identifier for the class. This identifier
 * should be a 32-bit checksum of the class type name. This value can be
 * calculated using command-line tool 'cksum'. For example:
 *	printf "%08x\n" `cksum <<< "my_class_t"`
 * The value in 'parent' is an initialized class instance that this class will
 * inherit from, or 'NULL'.
 */
void class_init(struct class_t *class_info, unsigned int id,
		void *parent);

/* Return the 'class_t' object at the beginning of any class instance */
struct class_t *class_get(void *p);

/* Given a class instance, reinterpret its type as any of its child or parent
 * classes. A panic message will occur if the reinterpretation is invalid.
 * Should be used only through macro 'CLASS_REINTERPRET_CAST', which will be
 * used in turn only through specific macros related to each class. */
void *class_reinterpret_cast(void *p, unsigned int id);

/* Same as 'class_reinterpret_cast', but the function does not fail if the cast
 * is invalid. Instead, it returns NULL. Should be used only through macro
 * 'CLASS_OF'. */
void *class_try_reinterpret_cast(void *p, unsigned int id);

#endif

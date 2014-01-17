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

#ifndef LIB_CLASS_CLASS_H
#define LIB_CLASS_CLASS_H

#include <assert.h>
#include <stdio.h>


/*
 * Class declarations
 */

#define CLASS(name) typedef struct _##name name;
#include "class.dat"
#undef CLASS



/*
 * Class
 */


struct class_t
{
	/* Name of the class */
	char *name;

	/* Unique identifier of the class, calculated as a hash function of its
	 * name during initialization. */
	unsigned int id;

	/* Total size of the class structure, considering both the user fields
	 * and the '__parent' and '__info' metadata fields. */
	unsigned int size;

	/* Offset where field '__info' can be found, relative to the beginning
	 * of the class data structure. */
	unsigned int info_offset;

	/* Class parent */
	struct class_t *parent;

	/* Class destructor */
	void (*destroy)(void *ptr);

	/* Next class in class list */
	struct class_t *next;
};


struct class_info_t
{
#ifndef NDEBUG
	unsigned int magic;
#endif
	struct class_info_t *parent;
	struct class_info_t *child;
	struct class_t *c;
};




/*
 * Base class 'Object'
 */

typedef struct _Object Object;

struct _Object
{
	/* Class information, first field */
	struct class_info_t __info;

	
	/*** Virtual functions ***/

	void (*Dump)(Object *self, FILE *f);
	Object *(*Clone)(Object *self);
	int (*Compare)(Object *self, Object *o);
	unsigned int (*Hash)(Object *self);
};

extern struct class_t ObjectClass;

static inline int isObject(void *p)
{
	assert(!p || ((Object *) p)->__info.c);
	assert(!p || ((Object *) p)->__info.c->id == ObjectClass.id);
	return 1;
}

static inline Object *asObject(void *p)
{
	assert(isObject(p));
	return (Object *) p;
}

void ObjectCreate(Object *self);
void ObjectDestroy(Object *self);

void ObjectDump(Object *self, FILE *f);




/*
 * Class definition macros
 */

#define CLASS_BEGIN(name, parent) \
	\
	static struct class_t *name##ParentClass \
			__attribute__((unused)) = &parent##Class; \
	extern struct class_t name##Class; \
	\
	struct _##name \
	{ \
		parent __parent; \
		struct class_info_t __info;
		/* ... user fields follow here ... */


#define CLASS_END(name) \
		\
		/* ... user fields end here ... */ \
	}; \
	\
	/* Prototype for destructor */ \
	void name##Destroy(name *self); \
	\
	/* Check if object is instance of class */ \
	static inline int is##name(void *p) \
	{ \
		return class_instance_of(p, &name##Class); \
	} \
	\
	/* Cast to class */ \
	static inline name *as##name(void *p) \
	{ \
		assert(is##name(p)); \
		return (name *) p; \
	}


#define CLASS_REGISTER(_name) \
	do { \
		_name instance; \
		class_init(); \
		_name##Class.name = #_name; \
		_name##Class.size = sizeof(_name); \
		_name##Class.info_offset = (unsigned int) \
				((void *) &instance.__info - \
				(void *) &instance); \
		_name##Class.parent = _name##ParentClass; \
		_name##Class.destroy = (void(*)(void *)) _name##Destroy; \
		class_register(&_name##Class); \
	} while (0)


#define new_ctor(name, ctor, ...) \
	({ \
		name *__p = xcalloc(1, name##Class.size); \
		__class_new(__p, &name##Class, __FILE__, __LINE__, #name); \
		name##ctor(__p, ##__VA_ARGS__); \
		__p; \
	})

#define new(name, ...) new_ctor(name, Create, ##__VA_ARGS__)

#define new_static_ctor(p, name, ctor, ...) \
	{ \
		name *__p = (p); \
		memset(__p, 0, name##Class.size); \
		__class_new(__p, &name##Class, __FILE__, __LINE__, #name); \
		name##ctor(__p, ##__VA_ARGS__); \
	}

#define new_static(p, name, ...) \
	new_static_ctor((p), name, Create, ##__VA_ARGS__)

#define delete(p) \
	{ \
		void *__p = (p); \
		class_delete(__p); \
		free(__p); \
	}

#define delete_static(p) class_delete((p))


extern struct class_t *class_list_head;
extern struct class_t *class_list_tail;

void class_init(void);
void class_register(struct class_t *c);
unsigned int class_compute_id(char *name);


#ifdef NDEBUG

#define __class_new(p, c, file, line, name) class_new((p), c)
void class_new(void *p, struct class_t *c);

#else

#define __class_new(p, c, file, line, name) class_new((p), c, file, line, name)
void class_new(void *p, struct class_t *c, char *file, int line, char *name);

#endif


void class_delete(void *p);
int class_instance_of(void *p, struct class_t *c);

/* Return the last child class of object in 'p' */
struct class_t *class_of(void *p);

/* Dump information about all classes registered */
void class_dump(FILE *f);


#endif


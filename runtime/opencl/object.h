#ifndef __CLRT_OBJECT_H__
#define __CLRT_OBJECT_H__


/* Container for all OpenCL objects */
typedef void (*clrt_object_destroy_func_t)(void *);

enum clrt_object_type_t
{
	CLRT_OBJECT_INVALID,
	CLRT_OBJECT_CONTEXT,
	CLRT_OBJECT_COMMAND_QUEUE,
	CLRT_OBJECT_MEM,
	CLRT_OBJECT_PROGRAM,
	CLRT_OBJECT_KERNEL,
	CLRT_OBJECT_EVENT,
	CLRT_OBJECT_SAMPLER
};

struct clrt_object_t
{
	enum clrt_object_type_t type;

	/* Callback function to destroy data */
	clrt_object_destroy_func_t destroy_func;

	/* Number of references to the data */
	int ref_count;
	pthread_mutex_t ref_mutex;

	/* Object itself */
	void *data;
};

struct clrt_object_context_t
{
	void *target;
	enum clrt_object_type_t type;
	struct clrt_object_t *match;
};

struct clrt_object_t *clrt_object_create(void *data, enum clrt_object_type_t type,
		clrt_object_destroy_func_t destroy);

void clrt_object_free(struct clrt_object_t *object);

struct clrt_object_t *clrt_object_enumerate(struct clrt_object_t *prev,
		enum clrt_object_type_t type);

struct clrt_object_t *clrt_object_find(void *object, enum clrt_object_type_t type);

int clrt_object_verify(void *object, enum clrt_object_type_t type);

int clrt_object_ref_update(void *object, enum clrt_object_type_t type, int change);
int clrt_object_retain(void *object, enum clrt_object_type_t type, int err_code);
int clrt_object_release(void *object, enum clrt_object_type_t type, int err_code);


#endif

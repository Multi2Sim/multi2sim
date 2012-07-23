#include <assert.h>
#include <m2s-clrt.h>

static struct clrt_object_t *head = NULL;
static pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;



/*
 * Private functions
 */

struct clrt_object_t *clrt_object_create(
	void *object, 
	enum clrt_object_type_t type, 
	clrt_destroy_t destroy)
{
	struct clrt_object_t *item;

	item = (struct clrt_object_t *) malloc(sizeof (struct clrt_object_t));
	if (item == NULL)
		fatal("%s: out of memory", __FUNCTION__);
	
	item->object = object;
	item->destroy = destroy;
	item->type = type;
	item->refcount = 1;
	pthread_mutex_init(&item->reflock, NULL);

	pthread_mutex_lock(&list_lock);
	item->next = head;
	head = item;
	pthread_mutex_unlock(&list_lock);
	return item;
}


/* implement locking before using */
struct clrt_object_t *clrt_object_enumerate(
	struct clrt_object_t *prev, 
	enum clrt_object_type_t type)
{
	if (prev == NULL)
		prev = head;
	else
		prev = prev->next;

	while (prev != NULL)
	{
		if (prev->type == type)
			return prev;
		prev = prev->next;
	}
	return NULL;
}


struct clrt_object_t *clrt_object_find(void *object, struct clrt_object_t **prev_item)
{
	struct clrt_object_t *item;
	struct clrt_object_t *prev;

	pthread_mutex_lock(&list_lock);

	item = head;
	prev = NULL;

	while (item != NULL)
	{
		if (item->object == object)
		{
			if (prev_item != NULL)
				*prev_item = prev;
			pthread_mutex_unlock(&list_lock);
			return item;
		}
		prev = item;
		item = item->next;
	}
	pthread_mutex_unlock(&list_lock);
	return NULL;
}


int clrt_change_refcount(void *object, int change, enum clrt_object_type_t type)
{
	struct clrt_object_t *prev;
	struct clrt_object_t *item = clrt_object_find(object, &prev);

	if (item == NULL)
		return CL_INVALID_VALUE;

	pthread_mutex_lock(&item->reflock);

	if (item->type != type)
	{
		pthread_mutex_unlock(&item->reflock);
		return CL_INVALID_VALUE;
	}
	item->refcount += change;

	if (item->refcount <= 0)
	{
		pthread_mutex_unlock(&item->reflock);
		item->destroy(item->object);

		if (prev == NULL)
			head = item->next;
		else
			prev->next = item->next;
		free(item);
	}
	else
		pthread_mutex_unlock(&item->reflock);
	return CL_SUCCESS;
}


/* True means that it has been found in the list
 * False otherwise */
int clrt_object_verify(void *object, enum clrt_object_type_t type)
{
	struct clrt_object_t *item;

	if (object == NULL)
		return 0;

	item = clrt_object_find(object, NULL);
	return item != NULL && item->type == type;
}



int clrt_retain(void *object, enum clrt_object_type_t type, int err_code)
{
	if (clrt_change_refcount(object, 1, type) != CL_SUCCESS)
		return err_code;
	return CL_SUCCESS;
}


int clrt_release(void *object, enum clrt_object_type_t type, int err_code)
{
	if (clrt_change_refcount(object, -1, type) != CL_SUCCESS)
		return err_code;
	return CL_SUCCESS;
}




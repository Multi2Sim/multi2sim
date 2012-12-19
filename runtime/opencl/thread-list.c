#include <stdlib.h>
#include <stdio.h>
#include "thread-list.h"

struct clrt_thread_list_t *clrt_thread_list_create(void)
{
	struct clrt_thread_list_t *list = malloc(sizeof (struct clrt_thread_list_t));
	list->head = NULL;
	pthread_rwlock_init(&list->lock, NULL);

	return list;
}


void clrt_thread_list_free(struct clrt_thread_list_t *list)
{
	struct clrt_thread_list_node_t *prev = NULL;

	pthread_rwlock_destroy(&list->lock);
	
	while (list->head)
	{
		prev = list->head;
		list->head = list->head->next;
		free(prev);
	}
	free(list);	
}


void clrt_thread_list_insert(struct clrt_thread_list_t *list, void *data)
{
	struct clrt_thread_list_node_t *node = malloc(sizeof (struct clrt_thread_list_node_t));
	node->data = data;
	/* prepend the current node to the beginning of the list */
	pthread_rwlock_wrlock(&list->lock);
	node->next = list->head;
	list->head = node;
	pthread_rwlock_unlock(&list->lock);
}


int clrt_thread_list_remove(struct clrt_thread_list_t *list, void *data)
{
	int status = 0; /* not found */
	struct clrt_thread_list_node_t *prev = NULL;
	pthread_rwlock_wrlock(&list->lock);

	struct clrt_thread_list_node_t *cur = list->head;
	while (cur)
	{
		if (cur->data == data)
		{
			if (prev)
				prev->next = cur->next;
			else
				list->head = cur->next;

			free(cur);
			status = 1;
			break;
		}

		prev = cur;
		cur = cur->next;
	}
	pthread_rwlock_unlock(&list->lock);

	return status;
}


void clrt_thread_list_visit(struct clrt_thread_list_t *list, clrt_thread_list_visitor visitor, void *context)
{
	pthread_rwlock_rdlock(&list->lock);

	struct clrt_thread_list_node_t *cur = list->head;

	while (cur && visitor(context, cur->data))
		cur = cur->next;

	pthread_rwlock_unlock(&list->lock);
}

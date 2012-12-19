#ifndef __THREAD_LIST_H__
#define __THREAD_LIST_H__

#include <pthread.h>

struct clrt_thread_list_node_t
{
	struct clrt_thread_list_node_t *next;
	void *data;
};


struct clrt_thread_list_t
{
	struct clrt_thread_list_node_t *head;
	pthread_rwlock_t lock;
};

typedef int (*clrt_thread_list_visitor)(void *context, void *data);

struct clrt_thread_list_t *clrt_thread_list_create(void);
void clrt_thread_list_free(struct clrt_thread_list_t *list);

void clrt_thread_list_insert(struct clrt_thread_list_t *list, void *data);
int clrt_thread_list_remove(struct clrt_thread_list_t *list, void *data);
void clrt_thread_list_visit(struct clrt_thread_list_t *list, clrt_thread_list_visitor visitor, void *context);

#endif

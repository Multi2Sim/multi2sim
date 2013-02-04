#include <string.h>

#include <lib/util/debug.h>
#include <lib/util/list.h>

#include "task.h"


struct list_t *task_list;

struct si_task_t *si_task_create(char *ID, long offset)
{
	struct si_task_t *task;
	
	/* Allocate */
	task = calloc(1, sizeof(struct si_task_t));
	if (!task)
		fatal("%s: out of memory", __FUNCTION__);
	
	/* Initialize the task's offset and ID */
	task->offset = offset;
	task->ID = strdup(ID);
	
	/* Return */
	return task;

}

void si_task_free(struct si_task_t *task)
{
	free(task->ID);
	free(task);
}

void si_task_dump(struct si_task_t *task, FILE *f)
{
	fprintf(f, "\tTask ID: %s\n", task->ID);
	fprintf(f, "\tTask Offset: %ld\n", task->offset);
}

/***********Task List Functions***********/
		
void si_task_list_init(void)
{							  
	task_list = list_create();
}

void si_task_list_add(struct si_task_t *task)
{
	list_add(task_list, task);
}

void si_task_list_done(void)
{
	int index; 
	
	for (index = 0; index < task_list->count; index++)
		si_task_free(list_get(task_list, index));
	
	list_free(task_list);
}

void si_task_list_dump(FILE *f)
{
	int index; 
	
	for (index = 0; index < task_list->count; index++)
	{
		si_task_dump(list_get(task_list, index), stdout);
	}
}

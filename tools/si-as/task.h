#include <stdio.h>

extern struct list_t *task_list;

struct si_task_t
{
	long offset;
	char *ID;
};

/* Returns a pointer to an si_task_t object
 * initialized with task ID = 'ID' and offset 'offset' */
struct si_task_t *si_task_create(char *ID, long offset);

void si_task_free(struct si_task_t *task);
void si_task_dump(struct si_task_t *task, FILE *f);

/***********Task List Functions***********/

void si_task_list_init(void);
void si_task_list_add(struct si_task_t *task);
void si_task_list_done(void);
void si_task_list_dump(FILE *f);

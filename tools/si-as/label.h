#include <stdio.h>

extern struct hash_table_t *label_table;

struct si_label_t
{
	long offset;
	char *ID;
};

/* Returns a pointer to an si_label_table_t object
 * initialized with label ID = 'ID' and offset 'offset' */
struct si_label_t *si_label_create(char *ID, long offset);

void si_label_free(struct si_label_t *label);
void si_label_dump(struct si_label_t *label, FILE *f);

/***********Label Table Functions***********/

void si_label_table_init(void);
void si_label_table_done(void);
void si_label_table_dump(FILE *f);
/* Returns 0 if a failure occured, otherwise creates a label object and inserts it */
int si_label_table_insert(struct si_label_t *label);
/* Returns a pointer to an si_label_t struct corresponding to the key 'ID' */
struct si_label_t *si_label_table_get(char *ID);


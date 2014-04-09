#include <stdlib.h>
#include <stdio.h>

int compar(const void *e1, const void *e2)
{
	return * (int *) e1 - * (int *) e2;
}


#define NELEM 500
int main(int argc, char **argv)
{
	int *elem, i;
	
	/* Create vector */
	printf("Creating a vector of %d elements...\n", NELEM);
	fflush(stdout);
	elem = calloc(NELEM, sizeof(int));
	for (i = 0; i < NELEM; i++)
		elem[i] = random();
	
	printf("Sorting vector...\n");
	fflush(stdout);
	qsort(elem, NELEM, sizeof(int), compar);

	printf("Finished\n");
	return 0;
}


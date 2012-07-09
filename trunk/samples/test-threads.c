#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_THREADS 32


void *child_fn(void *arg)
{
	int idx = (int) arg;
	printf("I'm thread %d\n", idx);
	fflush(stdout);
	return NULL;
}

int main(int argc, char **argv)
{
	int nthreads, i;
	pthread_t child[MAX_THREADS];

	/* Syntax */
	if (argc < 2) {
		fprintf(stderr, "syntax: test-threads <nthreads>\n");
		exit(1);
	}

	/* Get number of threads */
	nthreads = atoi(argv[1]);
	if (nthreads < 1 || nthreads > MAX_THREADS) {
		fprintf(stderr, "error: nthreads must be > 0 and < %d\n", MAX_THREADS);
		exit(1);
	}

	/* Run */
	for (i = 1; i < nthreads; i++)
		pthread_create(&child[i], NULL, child_fn, (int *) i);
	child_fn((int *) 0);
	for (i = 1; i < nthreads; i++)
		pthread_join(child[i], NULL);
	return 0;
}


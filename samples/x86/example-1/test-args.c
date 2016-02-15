#include <stdio.h>

int main(int argc, char **argv, char **envp) {
	int i;
	printf("number of arguments: %d\n", argc);
	for (i = 0; i < argc; i++)
		printf("\targv[%d] = %s\n", i, argv[i]);
	return 0;
}


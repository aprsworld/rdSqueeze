#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

static void init_srand(void) {
	struct timeval tv;

	gettimeofday(&tv,0);
	srand(tv.tv_usec);
}


static int rand_comparison(const void *a, const void *b ) {
	(void) a; (void) b;

	return	rand() % 2 ?  +1 : -1;
}

static void shuffle(void *base, size_t nmemb, size_t size ) {
	qsort(base,nmemb,size,rand_comparison);
}


/* this version randomizes the order of the command line arguements */

int main(int argc, char **argv ) {
	init_srand();
	shuffle(argv+1,argc-1,sizeof(char *));

	int i;
	for ( i = 0; argc > i;  i++ ) {
		printf("%s ",argv[i]);
	}
	printf("\n");

	return	0;
}

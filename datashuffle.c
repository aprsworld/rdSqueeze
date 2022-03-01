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
static char *strsave(char *s ) {
	char	*t = calloc(1,strlen(s));
	if ( 0 != t ) {
		strcpy(t,s);
	}
	return	t;
}

static void datashuffle(char *fname) {
	FILE	*in = fopen(fname,"r");
	if ( 0 != in ) {
		char	buffer[1024];
		char *array[1024];
		int i = 0;
		for (  ; i < 1024; i++ ) {
			if ( fgets(buffer,sizeof(buffer),in) ) {
				array[i] = strsave(buffer);
			} else {
				break;
			}
		}
		fclose(in);
		shuffle(array, i, sizeof(char *));

		int j = 0;

		for ( ; i > j; j++ ) {
			fputs(array[j],stdout);
			free(array[j]);
		}
		fflush(stdout);
	}
}
		

/* this version randomizes the order of the command line arguements  and the lines of data in the files */

int main(int argc, char **argv ) {
	init_srand();
	shuffle(argv+1,argc-1,sizeof(char *));

	int i;
	for ( i = 1; argc > i;  i++ ) {
		datashuffle(argv[i]);
	}

	return	0;
}

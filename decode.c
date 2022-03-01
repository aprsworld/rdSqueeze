
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <getopt.h>
#include "HC.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>


#define MAXE  sizeof(cmptab)/sizeof(char **)

typedef struct {
	int len;
	uint8_t outcode[8];
} EXPTAB;

#define DEFINE_EXPTAB
#include "clare.h"
 
static FILE *open_file_read_only(char *fname ) {
	FILE *in = fopen(fname,"r");
	if ( 0 == in ) {
		fprintf(stderr,"# unable to open %s %s\n",fname,strerror(errno));
	}
	return in;
}
static FILE *open_file_write_only(char *fname ) {
	FILE *out = fopen(fname,"w");
	if ( 0 == out ) {
		fprintf(stderr,"# unable to open %s %s\n",fname,strerror(errno));
	}
	return out;
}

static void decode(uint8_t *s, int len, FILE *out ) {
	uint8_t *s_end = s + len;

	for ( ; s < s_end ; ) {
		if ( escapeChar != s[0] ) {
			fputc(s[0],out);
			s++;
			continue;
		}
		/* we just received an escapeChar */
		s++;
		if ( 0xff == s[0] ) {
			fputc(escapeChar,out);
		} else if ( MAXE < s[0] ) {
			/* a problem.  we are off the endof the table */
			fflush(out);
			fprintf(stderr,"# off the end of the table\n");
			exit(1);
		} else {
			EXPTAB *p = exptab + s[0];
			fwrite(p->outcode,1,p->len,out);
		}
		s++;
	}
}

static void process(char *fname) {
	struct stat buf;
	if ( stat(fname,&buf)) {
		fprintf(stderr,"# cannot find %s\n",fname);
		exit(1);
	}
	char *buffer = calloc(buf.st_size+4,1);

	if ( 0 == buffer ) {
		fprintf(stderr,"# cannot calloc\n");
		exit(1);
	}



	FILE *in = open_file_read_only(fname);


	int rd = fread(buffer,1,buf.st_size,in);

	if ( buf.st_size != rd ) {
		fprintf(stderr,"# cannot fread the whole file \"%s\"\n",fname);
		exit(1);
	}


	fclose(in);
	char	oname[256] = {};
	strcpy(oname,fname);
	strcat(oname,".dec");
	FILE *out = open_file_write_only(oname);


	decode(buffer,rd,out);
	fclose(out);
}



enum arguments {
	A_lookup = 512,
	A_dump_table,
};

static void dump_table(void) {
	int i,j;

	for ( i = 0; MAXE > i; i++ ) {
		EXPTAB p = exptab[i];
		char *q = cmptab[i];
		fprintf(stdout," %3d. 0x%02x. len = %d, ",i,i,p.len);
		for ( j = 0 ; p.len > j; j++ ) {
			fprintf(stdout,"%02x ",p.outcode[j]);
		}
		for ( j = 0 ; p.len > j; j++ ) {
			fprintf(stdout,"%02x",p.outcode[j]);
		}
		fprintf(stdout, " %s",q + 2);
		fputc('\n',stdout);
		
	}


}
static void lookup(char *s ) {
	int	i,j;

	if ( 1 == sscanf(s,"%x",&i)) {
		EXPTAB p = exptab[i];
		fprintf(stdout," %3d. 0x%02x. len = %d, ",i,i,p.len);
		for ( j = 0 ; p.len > j; j++ ) {
			fprintf(stdout,"%02x ",p.outcode[j]);
		}
		for ( j = 0 ; p.len > j; j++ ) {
			fprintf(stdout,"%02x",p.outcode[j]);
		}
		fputc('\n',stdout);
	}



}

int main(int argc, char **argv ) {
	int i,n;
	
	while (1) {
		// int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
			/* normal program */
		        {"dump-table",                       no_argument,      0, A_dump_table, },
		        {"lookup",                                     1,      0, A_lookup, },
			{},
		};

		n = getopt_long(argc, argv, "", long_options, &option_index);

		if (n == -1) {
			break;
		}
		
		switch (n) {
			case A_lookup:
				lookup(optarg);
				break;
			case	A_dump_table:
				dump_table();
				break;
			case	'?':
				exit(0);
		}
	}
	
	i = 1;
	for ( ; argc >i; i++ ) {
		process(argv[i]);
	}



	return	0;
}

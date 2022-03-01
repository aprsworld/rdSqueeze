
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


#include "clare.h"

#define MAXE  sizeof(cmptab)/sizeof(char **);

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
static int findKey(char *s ) {
	/* s points to s null terminate string */
	int high,low;

	low = 0;
	high = MAXE;

	int i = (low + high) >>1;

	for ( ;low != i && high != i; i = (low + high) >> 1 ) {
		int cond = strcmp(s,cmptab[i]);
		if ( 0 == cond ) {
			return	i + 1;
		}
		if ( 0 > cond ) {
			if ( i == high ) {
				break;
			}
			high = i;
		} else {
			low = i;
		}
	}
	return	0;			

}

static int prepKey(char *d, uint8_t *s, int keyLen ) {
	uint8_t *s_end = s + keyLen;
	char	*d_start = d;
	memset(d,'\0',18);
	d[0] = HC[0];  d++;
	d[0] = HC[8 - keyLen]; d++;

	
	for ( ; s < s_end ; s++ ) {
		if ( escapeChar == s[0] ) {
			return	1;
		}
		d[0] = HC[s[0] >> 4];  d++;
		d[0] = HC[s[0] & 0xf]; d++;
	}
	for ( ; d < (d_start +16); d++ ) {
		d[0] = HC[0];
	}
	return	0;

}

static int encode(uint8_t *s,  int cnt, FILE *out ) {
	
	int i;
	int j;

	char	temp[18];
	for ( i = 6; 2 < i; i-- ) {
		if ( cnt < i ) {
			continue;
		}
		if ( prepKey(temp,s,i) ) {
			continue;
		}
		j = findKey(temp);
		if ( 0 != j ) {
			fputc(escapeChar,out);
			fputc(j-1,out);
			return	i;
		}
	}
	fputc(s[0],out);
	if ( escapeChar == s[0] ) {
		fputc(0xff,out);
	}

	return	1;
}

static void process(char *fname) {
	FILE *in = open_file_read_only(fname);
	char	buffer[256] = {};
	strcpy(buffer,fname);
	strcat(buffer,".enc");
	FILE *out = open_file_write_only(buffer);

	if ( 0 != in  && 0 != out ) {
		uint8_t buffer[8] = {};
		uint8_t *end = buffer;
		uint8_t temp[sizeof(buffer)];
		int rd,shift;

		for ( ; rd = fread(end , 1, sizeof(buffer) - ( end-buffer), in ) ; ) {
			end += rd;
			shift = encode(buffer,end - buffer,out);
			memset(temp,'\0',sizeof(temp));
			memcpy(temp,buffer+shift, sizeof(buffer) - shift );
			memcpy(buffer,temp,sizeof(buffer));
			end -= shift;
		}
		fclose(in);
		for ( ; end > buffer; ) {
			shift = encode(buffer,end - buffer,out);
			memset(temp,'\0',sizeof(temp));
			memcpy(temp,buffer+shift, sizeof(buffer) - shift );
			memcpy(buffer,temp,sizeof(buffer));
			end -= shift;
		}
		fclose(out);
		
	}
}



int main(int argc, char **argv ) {
	int i,j;

	j = MAXE;
	i = 1;
	for ( ; argc >i; i++ ) {
		process(argv[i]);
	}



	return	0;
}

// AVL tree implementation in C

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


enum arguments {
	A_pass_two = 512,
	A_output_filename,
	A_quiet,
	A_verbose,
	A_help,
};

int byteCounts[256];

static char *output_filename;
static int outputDebug=0;
static int total_nodes_visited;

static char	*strsave(char *s ) {
	char	*ret_val = 0;

	ret_val = malloc(strlen(s)+1);
	if ( 0 != ret_val) {
		strcpy(ret_val,s);
	}
	return ret_val;	
}
// Create Node
struct Node {
	char  decode[18];
	uint8_t encode;
	
	int visited;	/* keep track if ever used */

	/* avl tree stuff */
	int height;
	struct Node *left;
	struct Node *right;
};

// Calculate height
static int height(struct Node *N) {
	if (N == NULL) {
		return 0;
	}
	return N->height;
}

static int max(int a, int b) {
	return (a > b) ? a : b;
}
static char *hex_encode(uint8_t *decode) {
	static char buffer[64] = {};
	uint8_t *decode_end = decode + 8;
	char *d = buffer;
	for ( ; decode < decode_end; decode++ ) {
		d[0] = HC[decode[0] >> 4];  d++;
		d[0] = HC[decode[0] & 0xf]; d++;
	}

	return	buffer;
}


// Create a node
static struct Node *newNode(char *decode) {
	struct Node *node = (struct Node *) calloc(1,sizeof(struct Node));
	
	strcpy(node->decode,decode);
	node->left = NULL;
	node->right = NULL;
	node->height = 1;
	if ( 0 != outputDebug ) {
		fprintf(stdout,"\nnewNode=%s\n",node->decode);	fflush(stdout);
	}
	return (node);
}

// Right rotate
static struct Node *rightRotate(struct Node *y) {
	struct Node *x = y->left;
	struct Node *T2 = x->right;

	x->right = y;
	y->left = T2;

	y->height = max(height(y->left), height(y->right)) + 1;
	x->height = max(height(x->left), height(x->right)) + 1;

	return x;
}

// Left rotate
static struct Node *leftRotate(struct Node *x) {
	struct Node *y = x->right;
	struct Node *T2 = y->left;

	y->left = x;
	x->right = T2;

	x->height = max(height(x->left), height(x->right)) + 1;
	y->height = max(height(y->left), height(y->right)) + 1;

	return y;
}

// Get the balance factor
static int getBalance(struct Node *N) {
	if (N == NULL) {
		return 0;
	}
	return height(N->left) - height(N->right);
}

// Insert node
int debug2;
static struct Node *insertNode(struct Node *node, char  *decode) {
	// Find the correct position to insertNode the node and insertNode it
	if (node == NULL) {
		return newNode(decode);
	}

	if ( 0 != outputDebug ) {
		fprintf(stdout,"(%s)",node->decode);
	}
	int cond = memcmp(decode,node->decode,sizeof(node->decode));
	debug2 = ( 0 == strncmp("02004600",node->decode,8));

	
	if ( 0 > cond ) {
		if ( 0 != outputDebug ) {
			fprintf(stdout,"->left");
		}
		node->left = insertNode(node->left, decode);
	} else if ( 0 < cond ) {
		if ( 0 != outputDebug ) {
			fprintf(stdout,"->right");
		}
		node->right = insertNode(node->right, decode);
	} else {
		if ( 0 != outputDebug ) {
			fprintf(stdout,"->found(%s))",hex_encode(node->decode));
		}
		return	node;
	}
	

	// Update the balance factor of each node and
	// Balance the tree
	node->height = 1 + max(height(node->left), height(node->right));

	int balance = getBalance(node);
	int leftCond = 0, rightCond = 0;
	if ( 0 != node->left ) {
		leftCond = strcmp(decode,node->left->decode);
	}
	if ( 0 != node->right ) {
		rightCond = strcmp(decode,node->right->decode);
	}
	if (balance > 1 &&  0 > leftCond ) {
		return rightRotate(node);
	}
	if (balance < -1 &&  0 < rightCond ) {
		return leftRotate(node);
	}
	if (balance > 1 &&  0 < leftCond ) {
		node->left = leftRotate(node->left);
		return rightRotate(node);
	}
	if (balance < -1 &&  0 > rightCond ) {
		node->right = rightRotate(node->right);
		return leftRotate(node);
	}

  return node;
}

			
static void outputLine(FILE *out,struct Node *root )  {
		fprintf(out,"\t\"%s\",\n", root->decode);
}
static void outputEXPLine(FILE *out,struct Node *root )  {
	char buffer[32] = {};
	strcpy(buffer,root->decode);
	char *s = buffer;
	s[1] = ('8' - s[1] + '0');
	fprintf(out,"\t{ ");
	char *fmt = "0x%c%c, ";
	for ( ; 0 != s[0] ;  s += 2 ) {
		fprintf(out,fmt,s[0],s[1]);
	}
	fprintf(out,"},\n");
}
			
static void outputOrder( FILE *out,struct Node *root,void (*func)(FILE *, struct Node *)) {
	if ( 0 != root) {
		outputOrder(out,root->left,func);
		(*func)(out,root);
		outputOrder(out,root->right,func);
	}
}
static char *rtrimcrlf(char *s ) {
	char *t = s + strlen(s);

	for ( ; t > s ; t-- ) {
		if ( '\r' == t[0] || '\n' == t[0] ) {
			t[0] = '\0';
		}
	}

	return s;


}
static void pass_one_output_key(FILE *out, uint8_t *key ) {
	fprintf(out,"%s\n", hex_encode(key));
}
static void pass_one_process(FILE *out,char *fname) {

	FILE *in = fopen(fname,"r");
	if ( 0 != in ) {
		uint8_t	buffer[1024] = {};
		int rd;
		uint8_t *s,*s_end;
		for ( ; rd = fread(buffer,1,sizeof(buffer),in) ; ) {
			s_end = s = buffer;
			s_end += rd;
			for ( ; s < s_end ; s++ ) {
				int remaining = s_end - s;
				uint8_t key[8] = {};
				if ( 6 <= remaining ) {
					memset(key,'\0',sizeof(key));
					key[0] = 0x02;	/* 8 - 6 */
					memcpy(key+1,s,6);
					pass_one_output_key(out,key);
					}
				if ( 5 <= remaining ) {
					memset(key,'\0',sizeof(key));
					key[0] = 0x03;	/* 8 - 4 */
					memcpy(key+1,s,5);
					pass_one_output_key(out,key);
					}
				if ( 4 <= remaining ) {
					memset(key,'\0',sizeof(key));
					key[0] = 0x04;	/* 8 - 4 */
					memcpy(key+1,s,4);
					pass_one_output_key(out,key);
					}
				if ( 3 <= remaining ) {
					memset(key,'\0',sizeof(key));
					key[0] = 0x05;	/* 8 - 4 */
					memcpy(key+1,s,3);
					pass_one_output_key(out,key);
					}
			}
		}
		fclose(in);
	}
}
static struct Node *build_new_tree_from_earlier_pass(struct Node *root,FILE *in,int count){

	int	i;
	char	buffer[80] = {};
	char	key[32] = {};
		

	for ( ; 0 < count && 0 != fgets(buffer,sizeof(buffer),in) ; count-- ) {
		if ( 2 != sscanf(buffer,"%i %s",&i,key)) {
			break;
		}
		root = insertNode(root,key);
	}

	return	root;
}
static void printGivenLevel(FILE *out,struct Node* root, int level,int *cnt) {
	if (root == NULL) {
		return;
	}
	if (level == 1) {
		fprintf(out,"%s %d\n",root->decode,*cnt);
		*cnt = *cnt + 1;
	}
	else if (level > 1) {
		printGivenLevel(out,root->left, level-1,cnt);
		printGivenLevel(out,root->right, level-1,cnt);
	}
}
/* Function to line by line print level order traversal a tree*/
static void printLevelOrder(struct Node* root,FILE  *out ) {
	int h = height(root);
	int i;
	int cnt = 0;
	for (i=1; i<=h; i++) {
		fprintf(out,"level= %d\n",i);
		printGivenLevel(out,root, i,&cnt);
		fflush(out);
	}
}
static void freeGivenLevel(struct Node* root, int level) {
	if (root == NULL) {
		return;
	}
	if (level == 1) {
		free(root);
	}
	else if (level > 1) {
		freeGivenLevel(root->left, level-1);
		freeGivenLevel(root->right, level-1);
	}
}
/* Function to line by line print level order traversal a tree*/
static void freeLevelOrder(struct Node* root) {
	int h = height(root);
	int i;
	for (i=1; i<=h; i++) {
		freeGivenLevel(root, i);
	}
}
int debug;
static struct Node *visitNode(struct Node *node, char *decode) {
	// Find the correct position to insertNode the node and insertNode it
	if (node == NULL) {
		if ( 0 != outputDebug ) {
			fprintf(stderr,"visitNode failed %s\n",decode);
		}
		return	0;
	}
	

	int cond = strcmp(decode,node->decode);

	if ( 0 != outputDebug ) {
		fprintf(stderr,"%s\n",decode);
	}

	// debug = ( 0 == strncmp("02004600",node->decode,8));
	
	if ( 0 > cond ) {
		if ( 0 != outputDebug ) {
			fprintf(stderr,"->left");
		}
		return visitNode(node->left,decode);
	} else if ( 0 < cond ) {
		if ( 0 != outputDebug ) {
			fprintf(stderr,"->right");
		}
		return visitNode(node->right,decode);
	} 

	if ( 0 != outputDebug ) {
		fprintf(stderr,"%s visited success\n",node->decode);
	}
		
	node->visited++;	/* add one to the tally */
	if ( 1 == node->visited ) {
		total_nodes_visited++;
	}
	return	node;
}
static void search_new_tree(struct Node *root,char *s, char *s_end, char *fname ) {
	FILE *out = fopen("/tmp/w0","w");
	fprintf(out,"file = %s\n",fname);
	printLevelOrder(root,out);
	for ( ; s < s_end; ) {
		int remaining = s_end - s;
		uint8_t key[8] = {};
		if ( 6 <= remaining ) {
			memset(key,'\0',sizeof(key));
			key[0] = 0x02;	/* 8 - 6 */
			memcpy(key+1,s,6);
			if ( 0 != visitNode(root,hex_encode(key)) ) {
				s += 6;
				continue;
			}
		}
		if ( 5 <= remaining ) {
			memset(key,'\0',sizeof(key));
			key[0] = 0x03;	/* 8 - 4 */
			memcpy(key+1,s,5);
			if ( 0 != visitNode(root,hex_encode(key)) ) {
				s += 5;
				continue;
			}
		}
		if ( 4 <= remaining ) {
			memset(key,'\0',sizeof(key));
			key[0] = 0x04;	/* 8 - 4 */
			memcpy(key+1,s,4);
			if ( 0 != visitNode(root,hex_encode(key)) ) {
				s += 4;
				continue;
			}
		}
		if ( 3 <= remaining ) {
			memset(key,'\0',sizeof(key));
			key[0] = 0x05;	/* 8 - 3 */
			memcpy(key+1,s,3);
			if ( 0 != visitNode(root,hex_encode(key)) ) {
				s += 3;
				continue;
			}
		}
	/* not found */
	s++;
	}
	fclose(out);
}
static void do_byteCounts( uint8_t *s , uint8_t *s_end ) {


	for ( ; s < s_end ; s++ ) {
		byteCounts[s[0]] += 1;
	}
}

static int leastUsedByte(void) {
	int j = byteCounts[0];
	int	i;
	int 	n = 0;


	for ( i = 0; 256 > i; i++ ) {
		if ( byteCounts[i] < j ) {
			n = i;
			j = byteCounts[n];
		}
	}
	if ( 0 != outputDebug ) {
		fprintf(stderr,"leastUsedByte[0x%02x] = %d\n",n,j);
	}

	return	n;

}
static void new_tree_from_pass_two_search(struct Node *root,char *fname, FILE *out ) {
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


	FILE *in = fopen(fname,"r");

	int rd = fread(buffer,1,buf.st_size,in);

	if ( buf.st_size != rd ) {
		fprintf(stderr,"# cannot fread the whole file \"%s\"\n",fname);
		exit(1);
	}


	fclose(in);
	do_byteCounts(buffer,buffer+rd);
	search_new_tree(root,buffer,buffer+rd,fname);
	free(buffer);
}

static void visitedDump(struct Node *node, FILE *out ) {
	if ( 0 == node ) {
		return;
	}
	visitedDump(node->left,out);
	if ( 0 != node->visited ) {
		fprintf(out,"%6d %s\n",node->visited * ( '8' - node->decode[1]),node->decode); 
	}
	visitedDump(node->right,out);
}

static void exptab(FILE *out,struct Node *root) {
	fprintf(out,"#ifdef DEFINE_EXPTAB\n");
	fprintf(out,"EXPTAB exptab[] = {\n");
	outputOrder(out,root,outputEXPLine);
	fprintf(out,"\t{},\n};\n#endif\n");

}

static FILE *open_file_read_only(char *fname ) {
	FILE *in = fopen(fname,"r");
	if ( 0 == in ) {
		fprintf(stderr,"# unable to open %s %s\n",fname,strerror(errno));
	}
	return in;
}
static void visit_most_popular(struct Node *root ) {
	int i;
	char key[32] = {};
	char buffer[256] = {};
	if ( 254 > total_nodes_visited ) {
		FILE *in = open_file_read_only("/tmp/pass_one");
		if ( 0 == in ) {
			return;
		}
		for ( ; 254 > total_nodes_visited &&0 != fgets(buffer,sizeof(buffer),in)  ;  ) {
			rtrimcrlf(buffer);
			if ( 2 == sscanf(buffer,"%i %s", &i,key ) ) {
				visitNode(root,key);
			}
		}
		fclose(in);
	}
}
	

static int pass_one( int argc, char **argv, int optind , char *program_name  ) {
	FILE  *out;
	char cmd[256];
	int save_argc = argc;
	char **save_argv = argv;
	int save_optind = optind;

	snprintf(cmd,sizeof(cmd), " sort | uniq --count | sort -n -r  > /tmp/pass_one");

	out = popen(cmd,"w");
	for ( ; optind  < argc ;  optind++ ) {
		pass_one_process(out,argv[optind]);
	}
	pclose(out);

	snprintf(cmd,sizeof(cmd), "%s --pass-two  < /tmp/pass_one | sort -n -r "
		" > /tmp/pass_two",program_name);
	
	system(cmd);

	struct Node *root = NULL;
	FILE *in = fopen("/tmp/pass_two","r");
	root = build_new_tree_from_earlier_pass(root,in,1024);
	fclose(in);

	FILE *out1 = fopen("/tmp/1","w");
	printLevelOrder(root,out1);
	fclose(out1);

	/* rewind and do a normal search and keep track of what is found */
	 argc = save_argc;
	 argv = save_argv;
	 optind = save_optind;
	
	out = popen("sort -n -r > /tmp/pass_three","w");
	for ( ; optind  < argc ;  optind++ ) {
		new_tree_from_pass_two_search(root,argv[optind],out);
	}
	pclose(out);

	visit_most_popular(root);

	out = popen("sort -n -r > /tmp/pass_four","w");
	visitedDump(root,out);
	pclose(out);

	freeLevelOrder(root);	/* start clean */
	root = 0;

	in = fopen("/tmp/pass_four","r");
	root = build_new_tree_from_earlier_pass(root,in,255);
	fclose(in);

	if ( 0 != outputDebug ) {
		fprintf(stdout,"escapeChar = 0x%02x\n",leastUsedByte());
	}
	if ( 0 != output_filename ) {
		out = fopen(output_filename,"w");
		if ( 0 == out ) {
			fprintf(stderr,"# cannot fopen for writing %s\n",output_filename);
			return	2;
			}
		fprintf(out,"char *cmptab[] = { \n");
		outputOrder(out,root,outputLine);
		fprintf(out,"\t\"\",\n};\n\n");
		fprintf(out,"#define escapeChar 0x%02x\n",leastUsedByte());
		exptab(out,root);
		fclose(out);
	}

		
	return	0;
}
/* Print nodes at a given level */
static int pass_two_process(void) {
	char	buffer[80] = {};
	int	i,weight;
	char	key[64]= {};
	char 	*p;

	for ( ;   0 != fgets(buffer,sizeof(buffer),stdin); ) {
		if ( 2 != sscanf(buffer,"%d %s",&i,key)) {
			continue;
		}
		p = strpbrk(key,"12345678");
		weight = '8' - p[0];
		fprintf(stdout,"%7d %s\n",i * weight,key);
		}

	fflush(stdout);
	return	0;
}

int main(int argc, char **argv ) {
	char *program_name = argv[0];
	int n;
	int pass_two_flag = 0;
	program_name = strsave(argv[0]);

	/* command line arguments */
	while (1) {
		// int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
			/* normal program */
		        {"output-filename",                            1,      0, A_output_filename, },
		        {"pass-two",                         no_argument,      0, A_pass_two, },
			{"verbose",                          no_argument,       0, A_verbose, },
		        {"help",                             no_argument,       0, A_help, },
			{},
		};

		n = getopt_long(argc, argv, "", long_options, &option_index);

		if (n == -1) {
			break;
		}
		
		switch (n) {
			case A_output_filename:
				output_filename = strsave(optarg);
				break;
			case A_pass_two:
				pass_two_flag = 1;
				break;
			case A_quiet:
				outputDebug=0;
				fprintf(stderr,"# verbose (debugging) output to stderr disabled\n");
				break;
			case A_verbose:
				outputDebug=1;
				fprintf(stderr,"# verbose (debugging) output to stderr enabled\n");
				break;
			case	'?':
			case A_help:
				fprintf(stdout,"# --quiet\t\tDisable Output verbose / debugging to stderr\n"); 
				fprintf(stdout,"# --verbose\t\tOutput verbose / debugging to stderr\n"); 
				fprintf(stdout,"#\n");
				fprintf(stdout,"# --help\t\tThis help message then exit\n");
				fprintf(stdout,"#\n");
				exit(0);
		}
	}
	if ( 0 != pass_two_flag ) {
		return pass_two_process();
	}
	
	pass_one(argc,argv,optind,program_name);

	  return 0;
}



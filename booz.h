/* booz.h */
/* this file is public domain */

/* T_UINT16 must be an unsigned data type of exactly 16 bits */
#define T_UINT16     unsigned short

/* Define FIXFNAME to activate the fixfname() function that converts
filename syntax to be acceptable to the host system */
/* #define FIXFNAME */

/*
OPEN(x)     open file x for read
CREATE(x)   create file x for read/write

Files opened by OPEN() and CREATE() must be opened in
binary mode (not involving any newline translation).
*/

#define NEED_B

/* Conventional stdio, using "b" suffix for binary open */
#ifdef NEED_B
#define  CREATE(x)	fopen(x, "wb")
#define  OPEN(x)	fopen(x, "rb")

#else
/* some systems (e.g. Ultrix) don't like a trailinb "b" */
#define  CREATE(x)	fopen(x, "w")
#define  OPEN(x) 	fopen(x, "r")
#endif

/* don't change the rest of this file */
#define MEM_BLOCK_SIZE	8192

/* Functions defined by Booz */

int getfile ();
int lzd ();
int readdir ();
int rd_zooh ();
int rd_dir ();
int addbfcrc();
int prterror();
int oozext ();
int putstr ();
char *itoa ();
int fixfname ();

/* Standard functions */

char *malloc();
char *strcpy();
char *strcat();
char *strncat();

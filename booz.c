#define  VERSION  "Version 1.02 (1988/08/25)\n"

/* booz.c -- small, memory-efficient Barebones Ooz to extract Zoo archives.

For maximum portability, Booz does not use the data type `unsigned long'.

The functions and system calls required by Tiny Booz are:
   read()
   write()
   strlen()
   exit()
   malloc()
   lseek()
   open() and/or creat()
   close()
   unlink()                -- may be defined to be an empty function


Small Booz additionally uses the following functions:
   strcat()
   strcpy()

Big Booz additionally use the following function:
   strncat()
*/

/* 
The contents of this file are hereby released to the public domain.

                                   -- Rahul Dhesi 1988/08/25
*/

#include "options.h"
#include "func.h"
#include "zoo.h"

main(argc,argv)
register int argc;
register char **argv;
{
#ifdef TINY
   static char usage[]=
      "Usage:  booz archive.zoo\n";
   if (argc < 2) {
      putstr ("Public domain Barebones Ooz\nZoo archive extractor (Tiny) by Rahul Dhesi\n");
      putstr (VERSION);
      putstr (usage);
      exit (1);
   }
#endif

#ifdef SMALL
   static char usage[]=
      "Usage:  booz archive[.zoo] [ file ... ]\n";
   if (argc < 2) {
      putstr ("Public domain Barebones Ooz\nZoo archive extractor (Small) by Rahul Dhesi\n");
      putstr (VERSION);
      putstr (usage);
      exit (1);
   }
#endif

#ifdef BIG
   static char usage[]=
      "Usage:  booz {lxt} archive[.zoo] [ file ... ]\n";
   if (argc < 3) {
      putstr ("Public domain Barebones Ooz\nZoo archive extractor/lister (Big) by Rahul Dhesi\n");
      putstr (VERSION);
      putstr (usage);
      putstr ("l = list, x = extract, t = test\n");
      exit (1);
   }
#endif

#ifdef TINY
   oozext (argv[1]);
#endif

#ifdef SMALL
   oozext (argv[1], argc - 2, &argv[2]);
#endif

#ifdef BIG
   {
      char *p;
      p = argv[1];
      if (*p == 'L')
         *p = 'l';
      if (*p == 'X')
         *p = 'x';
      if (*p == 'T')
         *p = 't';
      if (*p != 'l' && *p != 'x' && *p != 't') {
         putstr (usage);
         exit (1);
      }
      oozext (argv[2], p, argc - 3, &argv[3]);
   }
#endif

   exit (0);
}

/**********************/
/* putstr()
This function prints a string to the standard output handle without
using printf() or the standard I/O library.  If a null string, nothing 
is printed (not even the null character).
*/
int putstr (str)
register char *str;
{
   if (str != (char *) 0)
      write (1, str, strlen(str));
}

/**********************/
/* prterror()
Prints an error message.  The first character controls the severity
of the error and the result.

   'm'   informative message
   'w'   warning     -- execution continues
   'e'   error       -- execution continues
   'f'   fatal error -- program exits
*/

int prterror (level, a, b, c)
int level;
char *a, *b, *c;

{

#ifdef DEBUG
   {
      char tmp[2];
      tmp[0] = level & 0x7f;
      tmp[1] = '\0';
      putstr ("prterror:  level = ");
      putstr (tmp);
      putstr ("\n");
   }
#endif

   switch (level & 0x7f) {
      case 'm': break;
      case 'w': putstr ("WARNING:  "); break;
      case 'e': putstr ("ERROR:  "); break;
      case 'f': putstr ("FATAL:  "); break;
      default: prterror ('f', "Internal error\n", ((char *) 0), ((char *) 0));
   }
   putstr (a);
   putstr (b);
   putstr (c);

   if (level == 'f')       /* and abort on fatal error 'f' but not 'F' */
      exit (1);
}

/*************
This function copies count characters from the source file to the
destination Function return value is 0 if no error, 2 if write error,
and 3 if read error.  

The global variable crccode is updated.
*/
extern char out_buf_adr[];

int getfile(input_han, output_han, count)
int input_han, output_han;
long count;
{
   register int how_much;

   while (count > 0) {
      if (count > OUT_BUF_SIZE)
         how_much = OUT_BUF_SIZE;
      else
         how_much = count;
      count -= how_much;
      if (read (input_han, out_buf_adr, how_much) != how_much)
         return (3);
      addbfcrc (out_buf_adr, how_much);
      if (output_han != -2 &&
            write (output_han, out_buf_adr, how_much) != how_much)
         return (2);
   }
   return (0);
}

#ifndef TINY

int needed (fname, argc, argv)
char *fname;
int argc;
char *argv[];
{
   register int i;
   if (argc == 0)
      return (1);
   for (i = 0; i < argc; i++) {
      if (match (fname, argv[i]))
         return (1);
   }
   return (0);
}

/***********************/
/*
match() compares a pattern with a string.  Wildcards accepted in
the pattern are:  "*" for zero or more arbitrary characters;  "?"
for any one characters.  Unlike the MS-DOS wildcard match, "*" is
correctly handled even if it isn't at the end of the pattern. ".'
is not special.

Originally written by Jeff Damens of Columbia University Center for
Computing Activities.  Taken from the source code for C-Kermit version
4C.
*/

int match (string, pattern) 
register char *string, *pattern;
{
   char *psave,*ssave;        /* back up pointers for failure */
   psave = ssave = ((char *) 0);
   while (1) {
      for (; *pattern == *string; pattern++,string++)  /* skip first */
         if (*string == '\0') 
            return(1);                          /* end of strings, succeed */
      if (*string != '\0' && *pattern == '?') {
         pattern++;                             /* '?', let it match */
         string++;
      } else if (*pattern == '*') {             /* '*' ... */
         psave = ++pattern;                     /* remember where we saw it */
         ssave = string;                        /* let it match 0 chars */
      } else if (ssave != ((char *) 0) && *ssave != '\0') {   /* if not at end  */
         /* ...have seen a star */
         string = ++ssave;                      /* skip 1 char from string */
         pattern = psave;                       /* and back up pattern */
      } else 
         return(0);                             /* otherwise just fail */
   }
}

#endif /* ndef TINY */

int memerr()
{
   prterror ('f', "Ran out of memory\n");
}

#ifdef BIG
/* cfactor() calculates the compression factor given a directory entry */
int cfactor (org_size, size_now)
long org_size, size_now;
{
   register int size_factor;

   while (org_size > 10000) { /* avoid overflow below */
      org_size = org_size >> 4;
      size_now = size_now >> 4;
   }
   if (org_size == 0)         /* avoid division by zero */
      size_factor = 0;
   else {
      size_factor = 
         (
            (1000 * 
               (org_size - size_now)
            ) / org_size + 5
         ) / 10;
   }
   return (size_factor);
}

/******
Function itoa() converts a positive long integer into a text string of
digits.  The buffer pointer buf must point to a buffer to receive the
digit string.  The digit string is stored right justified in the
buffer with leading blanks.  If the supplied number is negative, or if
overflow occurs, a single '*' is returned.
*/

char *itoa (pad_ch, n, buf, buflen)
char pad_ch;                  /* leading pad character */
long n;                       /* positive long int to convert */
char *buf;                    /* buffer to receive digit string */
int buflen;                   /* length of buffer */
{
   char *p;
   int i;
   for (i = 0;  i < buflen;  i++)         /* fill buffer with pad char */
      buf[i] = pad_ch;
   p = buf + buflen - 1;
   *p-- = '\0';                           /* ensure null termination */
   i = buflen - 1;
   for (;;) {
      if (n < 0) {                        /* can't handle negative n */
         goto overflow;
      } else {
         *p-- = (int) (n % 10) + '0';     /* store a converted digit */
         n = n / 10;
         i--;
         if (n == 0 || i == 0)
            break;
      } /* end of else of if (n < 0) */
   } /* end while (buflen > 0) */
   if (n != 0)                            /* buffer overflow */
      goto overflow;
   return (buf);

overflow:                                 /* bad value filled with stars */
   for (i = 0; i < buflen; i++)
      buf[i] = '*';
   buf[buflen-1] = '\0';
   return (buf);
}

#endif /* BIG */

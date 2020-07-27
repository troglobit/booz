/* oozext.c -- extracts files from archive */

/* The contents of this file are hereby released to the public domain.

                                   -- Rahul Dhesi 1987/02/08

The function fixfname() is currently defined to be empty but may be
activated if needed to fix filename syntax to be legal for your
computer system.  Look near the end of this file for instructions and
a sample implementation of fixfname().  Currently, fixfname() is used
only if the symbol FIXFNAME is defined.
*/

#include "options.h"
#include "zoo.h"
#include "oozio.h"
#include "func.h"

extern unsigned int crccode;

#ifndef TINY
int needed ();
#endif

#ifdef TINY
int oozext (zoo_path)
char *zoo_path;
#endif

#ifdef SMALL
int oozext (zoo_path, argc, argv)
char *zoo_path;
int argc;
char *argv[];
#endif

#ifdef BIG
int oozext (zoo_path, option, argc, argv)
char *zoo_path;
char *option;
int argc;
char *argv[];
#endif

{
int zoo_han;                              /* handle for open archive */
int this_han;                             /* handle of file to extract */
long next_ptr;                            /* pointer to within archive */
struct zoo_header zoo_header;             /* header for archive */
int status;                               /* error status */
struct direntry direntry;                 /* directory entry */
int all = 0;                              /* overwrite all? */
static char vermsg[] = "A higher version of Ooz is needed to extract ";
int exitstat = 0;                         /* return code */

#ifdef BIG
int first_time = 1;
static char *month_list="000JanFebMarAprMayJunJulAugSepOctNovDec";
#endif

#ifndef TINY
{
   /* If no dot in name, add ".zoo" extension */
   char *p;
   p = zoo_path;
   while (*p != '\0' && *p != '.')
      p++;
   if (*p != '.') {  /* no dot found */
      p = malloc (strlen (zoo_path) + 5);
      if (p == (char *) 0)
         memerr();
      strcpy (p, zoo_path);
      strcat (p, ".zoo");
      zoo_path = p;
   }
}
#endif

zoo_han = OPEN(zoo_path);

if (zoo_han == -1)
   prterror ('f', "Could not open ", zoo_path, "\n");

/* read header */
rd_zooh (&zoo_header, zoo_han);
lseek (zoo_han, zoo_header.zoo_start, 0); /* seek to where data begins */

while (1) {
   rd_dir (&direntry, zoo_han);

   if (direntry.lo_tag != LO_TAG || direntry.hi_tag != HI_TAG)
         prterror ('f', "Bad entry in archive\n", 
            ((char *) 0), ((char *) 0));
   if (direntry.next == 0L) {                /* END OF CHAIN */
      break;                                 /* EXIT on end of chain */
   }
   next_ptr = direntry.next;                 /* ptr to next dir entry */

#ifndef TINY
      /* See if this file is wanted */
      if (!needed (direntry.fname, argc, argv)) {
         goto loop_again;
      }
#endif

#ifdef BIG
  /* If list needed, give information and loop again */
   if (*option == 'l') {
      char outstr[80];
      char buf[20];
      int year, month, day, hours, min, sec;
      int size_factor;
      size_factor = cfactor (direntry.org_size, direntry.size_now);
   
      year  =  ((unsigned int) direntry.date >> 9) & 0x7f;
      month =  ((unsigned int) direntry.date >> 5) & 0x0f;
      day   =  direntry.date        & 0x1f;
      
      hours =  ((unsigned int) direntry.time >> 11)& 0x1f;
      min   =  ((unsigned int) direntry.time >> 5) & 0x3f;
      sec   =  ((unsigned int) direntry.time & 0x1f) * 2;

      if (first_time) {
         putstr ("Length    CF  Size Now  Date      Time\n");
         putstr ("--------  --- --------  --------- --------\n");
         first_time = 0;
      }
      strcpy (outstr, itoa(' ', direntry.org_size, buf, 9));
      strcat (outstr, itoa(' ',(long) size_factor, buf, 5));
      strcat (outstr, "% ");
      strcat (outstr, itoa(' ',direntry.size_now, buf, 9));
      strcat (outstr, "  ");
      strcat (outstr, itoa(' ',(long) day, buf, 3));
      strcat (outstr, " ");
      strncat (outstr, &month_list[month*3], 3);
      strcat (outstr, " ");
      if (day && month)
         strcat (outstr, itoa(' ',(long) (year+80) % 100, buf, 3));
      else
         strcat (outstr, itoa(' ',0L, buf, 3));
      strcat (outstr, " ");
      strcat (outstr, itoa('0',(long) hours, buf, 3));
      strcat (outstr, ":");
      strcat (outstr, itoa('0',(long) min, buf, 3));
      strcat (outstr, ":");
      strcat (outstr, itoa('0',(long) sec, buf, 3));
      strcat (outstr, "  ");
      strcat (outstr, direntry.fname);
      strcat (outstr, "\n");
      putstr (outstr);

#ifdef COMMENT
      printf ("%8lu %3u%% %8lu  %2d %-.3s %02d %02d:%02d:%02d  ",  
               direntry.org_size, 
               size_factor, direntry.size_now, 
               day, &month_list[month*3], 
               (day && month) ?  (year+80) % 100 : 0,
               hours, min, sec);
      printf ("%s\n", direntry.fname);
#endif

     goto loop_again;
   }
#endif /* SMALL */


   if (direntry.major_ver > 1 ||
         (direntry.major_ver == 1 && direntry.minor_ver > 0)) {
      prterror ('e', vermsg, direntry.fname, "\n");
      goto loop_again;
   }

#ifdef FIXFNAME
   /* Make the filename syntax acceptable to the host system */
   fixfname (direntry.fname);
#endif


#ifndef TINY
   /* See if this file already exists */

#ifdef BIG
   if (*option != 't' && !all)
#else
   if (!all)
#endif
   {

      this_han = OPEN(direntry.fname);
      if (this_han != -1) {
         char ans[2];
         char ans2[2];
         close (this_han);
   
         do {
            prterror ('m', "Overwrite ", direntry.fname, " (Yes/No/All)? ");
            read (0, ans, 1);
            do {
               read (0, ans2, 1);
            } while (*ans2 != '\n');
         }  while (*ans != 'y' && *ans != 'Y' && 
                   *ans != 'n' && *ans != 'N' &&
                   *ans != 'a' && *ans != 'A');    
   
         if (*ans == 'a' || *ans == 'A')
            all++;
         if (*ans == 'n' || *ans == 'N') {
            prterror ('m', "Skipping ", direntry.fname, "\n");
            goto loop_again;
         }
      }
   }
#endif /* ndef TINY */


#ifdef BIG
   if (*option == 't')
      this_han = -2;
   else
#endif
      this_han = CREATE(direntry.fname);

   if (this_han == -1) {
      prterror ('e', "Could not open ", direntry.fname, " for output.\n");
   } else {
      lseek (zoo_han, direntry.offset, 0);
      crccode = 0;      /* Initialize CRC before extraction */
      prterror ('m', direntry.fname, " ", ((char *) 0));

      if (direntry.packing_method == 0)
         status = getfile (zoo_han, this_han, direntry.size_now);
      else if (direntry.packing_method == 1)
         status = lzd (zoo_han, this_han); /* uncompress */
      else
         prterror ('e', vermsg, direntry.fname, "\n");
      if (status != 0) {
         unlink(direntry.fname);
         if (status == 1) {
            memerr();
         } else 
            prterror ('e', "I/O error writing ", direntry.fname, "\n");
      } else {
         if (direntry.file_crc != crccode) {
            putstr ("<--\007WARNING:  Bad CRC.\n");
            exitstat = 1;
         } else
            putstr ("\n");
      } /* end if */
   } /* end if */

#ifdef BIG
   if (*option != 't')
#endif
      close (this_han);

loop_again:
   lseek (zoo_han, next_ptr, 0); /* ..seek to next dir entry */
} /* end while */
close (zoo_han);
exit (exitstat);
} /* end oozext */

/*
Function fixfname() fixes the syntax of the supplied filename so it is
acceptable to the host system.  We call the standard string function 
strchr(s, c) which searches a string s for a character c and returns
a pointer to it or returns NULL if not found.  The scheme below
strips the 8th bit and changes control characters to corresponding 
printable characters (e.g.  ^A becomes A).

As supplied, fixfname() is activated only if the symbol FIXFNAME
is defined.
*/


#ifdef FIXFNAME
/* set of all characters legal for our system */
char legal[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.";

char *strchr ();

int fixfname (fname)
char *fname;
{
   char *p;
   for (p = fname;  *p != '\0';  p++) {
      *p &= 0x7f;
      if (strchr (legal, *p) == (char *) 0) {
         *p = legal[(*p) % 26];
      }
   }
}
#endif /* FIXFNAME */

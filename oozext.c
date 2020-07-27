/* oozext.c -- extracts files from archive */

/* Main extraction module.  This file is public domain.

                                   -- Rahul Dhesi 1991/07/07

The function fixfname() is currently defined to be empty but may be
activated if needed to fix filename syntax to be legal for your
computer system.  Look near the end of this file for instructions and
a sample implementation of fixfname().  Currently, fixfname() is used
only if the symbol FIXFNAME is defined.
*/

#include "booz.h"
#include "zoo.h"
#include <stdio.h>

extern unsigned int crccode;

int needed ();

int oozext (zoo_path, option, argc, argv)
char *zoo_path;
char *option;
int argc;
char *argv[];

{
FILE *zoofile;                            /* open archive */
FILE *this_file;                          /* file to extract */
long next_ptr;                            /* pointer to within archive */
struct zoo_header zoo_header;             /* header for archive */
int status;                               /* error status */
struct direntry direntry;                 /* directory entry */
int all = 0;                              /* overwrite all? */
static char vermsg[] = "A higher version of Ooz is needed to extract ";
int exitstat = 0;                         /* return code */

int first_time = 1;
static char *month_list="000JanFebMarAprMayJunJulAugSepOctNovDec";

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

zoofile = OPEN(zoo_path);

if (zoofile == NULL)
   prterror ('f', "Could not open ", zoo_path, "\n");

/* read header */
rd_zooh (&zoo_header, zoofile);
fseek(zoofile, zoo_header.zoo_start, 0); /* seek to where data begins */

while (1) {
   rd_dir (&direntry, zoofile);

   if (direntry.lo_tag != LO_TAG || direntry.hi_tag != HI_TAG)
         prterror ('f', "Bad entry in archive\n", 
            ((char *) 0), ((char *) 0));
   if (direntry.next == 0L) {                /* END OF CHAIN */
      break;                                 /* EXIT on end of chain */
   }
   next_ptr = direntry.next;                 /* ptr to next dir entry */

      /* See if this file is wanted */
      if (!needed (direntry.fname, argc, argv)) {
         goto loop_again;
      }

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


   if (direntry.major_ver > 2 ||
         (direntry.major_ver == 2 && direntry.minor_ver > 1)) {
      prterror ('e', vermsg, direntry.fname, "\n");
      goto loop_again;
   }

#ifdef FIXFNAME
   /* Make the filename syntax acceptable to the host system */
   fixfname (direntry.fname);
#endif


   /* See if this file already exists */

   if (*option != 't' && !all) {
      this_file = OPEN(direntry.fname);
      if (this_file != NULL) {
         char ans[2];
         char ans2[2];
         fclose(this_file);
   
         do {
            prterror ('m', "Overwrite ", direntry.fname, " (Yes/No/All)? ");
            fread(ans, 1, 1, stdin);
            do {
	       fread(ans2, 1, 1, stdin);
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

   if (*option == 't')
      this_file = NULL;
   else
      this_file = CREATE(direntry.fname);

   if (*option != 't' && this_file == NULL) {
      prterror ('e', "Could not open ", direntry.fname, " for output.\n");
   } else {
      fseek(zoofile, direntry.offset, 0);
      crccode = 0;      /* Initialize CRC before extraction */
      putstr(direntry.fname);
      putstr(" ");

      if (direntry.packing_method == 0)
         status = getfile(zoofile, this_file, direntry.size_now);
      else if (direntry.packing_method == 1)
         status = lzd(zoofile, this_file); /* uncompress */
      else if (direntry.packing_method == 2)
         status = lzh_decode(zoofile, this_file); /* uncompress */
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
         } else {
            putstr ("\n");
	 }
      } /* end if */
   } /* end if */

   if (*option != 't')
      fclose(this_file);

loop_again:
   fseek(zoofile, next_ptr, 0); /* ..seek to next dir entry */
} /* end while */
fclose(zoofile);
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

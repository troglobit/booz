/*$Source: /usr/home/dhesi/booz/RCS/decode.c,v $*/
/*$Id: decode.c,v 1.8 91/07/08 12:06:52 dhesi Exp $*/
/***********************************************************
Adapted from "ar" archiver written by Haruhiko Okumura.
***********************************************************/

#include "booz.h"
#include "zoo.h"
#include "ar.h"
#include "lzh.h"

extern int decoded;     /* from huf.c */

static int j;  /* remaining bytes to copy */

/* must call this before decoding each file */
int decode_start()
{
   huf_decode_start();
   j = 0;
   decoded = 0;
}

/* decodes up to 'count' chars (but no more than DICSIZ) into supplied
buffer; returns actual count.  */

int decode(count, buffer)
uint count;
uchar buffer[];
{
   static uint i;
   uint r, c;

   r = 0;
   while (--j >= 0) {
      buffer[r] = buffer[i];
      i = (i + 1) & (DICSIZ - 1);
      if (++r == count)
         return r;
   }
   for ( ; ; ) {
      c = decode_c();
      if (decoded)
         return r;
      if (c <= UCHAR_MAX) {
         buffer[r] = c;
         if (++r == count)
            return r;
      } else {
         j = c - (UCHAR_MAX + 1 - THRESHOLD);
         i = (r - decode_p() - 1) & (DICSIZ - 1);
         while (--j >= 0) {
            buffer[r] = buffer[i];
            i = (i + 1) & (DICSIZ - 1);
            if (++r == count)
               return r;
         }
      }
   }
}

FILE *arcfile;

extern char out_buf_adr[];       /* address of buffer */

/*
lzh_decode decodes its input and sends it to output.
Should return error status or byte count, but currently
returns 0.
*/

int lzh_decode(infile, outfile)
FILE *infile;
FILE *outfile;
{
   int n;
   extern int decoded;
   arcfile = infile;             /* stream to be decoded */

   decode_start();
   while (!decoded) {
      n = decode((uint) DICSIZ, (uchar *)out_buf_adr); 
      /* n = count of chars decoded */
      fwrite_crc(out_buf_adr, n, outfile);
   }
   return 0;
}

/*$Source: /usr/home/dhesi/booz/RCS/huf.c,v $*/
/*$Id: huf.c,v 1.8 91/07/08 12:06:53 dhesi Exp $*/
/***********************************************************
Static Huffman

Adapted from "ar" archiver written by Haruhiko Okumura.
***********************************************************/
#include "booz.h"
#include "zoo.h"
#include "ar.h"
#include "lzh.h"

#define NP (DICBIT + 1)
#define NT (CODE_BIT + 3)
#define PBIT 4  /* smallest integer such that (1U << PBIT) > NP */
#define TBIT 5  /* smallest integer such that (1U << TBIT) > NT */
#if NT > NP
# define NPT NT
#else
# define NPT NP
#endif

int decoded;		/* for use in decode.c */

ushort left[2 * NC - 1], right[2 * NC - 1];
static uchar c_len[NC], pt_len[NPT];
static uint  blocksize;
static ushort c_table[4096], pt_table[256];

static int read_pt_len(nn, nbit, i_special)
int nn;
int nbit;
int i_special;
{
   int i, c, n;
   uint mask;

   n = getbits(nbit);
   if (n == 0) {
      c = getbits(nbit);
      for (i = 0; i < nn; i++) pt_len[i] = 0;
      for (i = 0; i < 256; i++) pt_table[i] = c;
   } else {
      i = 0;
      while (i < n) {
         c = bitbuf >> (BITBUFSIZ - 3);
         if (c == 7) {
            mask = (unsigned) 1 << (BITBUFSIZ - 1 - 3);
            while (mask & bitbuf) {  mask >>= 1;  c++;  }
         }
         fillbuf((c < 7) ? 3 : c - 3);
         pt_len[i++] = c;
         if (i == i_special) {
            c = getbits(2);
            while (--c >= 0) pt_len[i++] = 0;
         }
      }
      while (i < nn) pt_len[i++] = 0;
      make_table(nn, pt_len, 8, pt_table);
   }
}

static int read_c_len()
{
   int i, c, n;
   uint mask;

   n = getbits(CBIT);
   if (n == 0) {
      c = getbits(CBIT);
      for (i = 0; i < NC; i++) c_len[i] = 0;
      for (i = 0; i < 4096; i++) c_table[i] = c;
   } else {
      i = 0;
      while (i < n) {
         c = pt_table[bitbuf >> (BITBUFSIZ - 8)];
         if (c >= NT) {
            mask = (unsigned) 1 << (BITBUFSIZ - 1 - 8);
            do {
               if (bitbuf & mask) c = right[c];
               else               c = left [c];
               mask >>= 1;
            } while (c >= NT);
         }
         fillbuf((int) pt_len[c]);
         if (c <= 2) {
            if      (c == 0) c = 1;
            else if (c == 1) c = getbits(4) + 3;
            else             c = getbits(CBIT) + 20;
            while (--c >= 0) c_len[i++] = 0;
         } else c_len[i++] = c - 2;
      }
      while (i < NC) c_len[i++] = 0;
      make_table(NC, c_len, 12, c_table);
   }
}

uint decode_c()
{
    uint j, mask;

   if (blocksize == 0) {
      blocksize = getbits(16);
      if (blocksize == 0) {
         decoded = 1;
         return 0;
      }
      read_pt_len(NT, TBIT, 3);
      read_c_len();
      read_pt_len(NP, PBIT, -1);
   }
   blocksize--;
   j = c_table[bitbuf >> (BITBUFSIZ - 12)];
   if (j >= NC) {
      mask = (unsigned) 1 << (BITBUFSIZ - 1 - 12);
      do {
         if (bitbuf & mask) j = right[j];
         else               j = left [j];
         mask >>= 1;
      } while (j >= NC);
   }
   fillbuf((int) c_len[j]);
   return j;
}

uint decode_p()
{
   uint j, mask;

   j = pt_table[bitbuf >> (BITBUFSIZ - 8)];
   if (j >= NP) {
      mask = (unsigned) 1 << (BITBUFSIZ - 1 - 8);
      do {
         if (bitbuf & mask) j = right[j];
         else               j = left [j];
         mask >>= 1;
      } while (j >= NP);
   }
   fillbuf((int) pt_len[j]);
   if (j != 0) j = ((unsigned) 1 << (j - 1)) + getbits((int) (j - 1));
   return j;
}

int huf_decode_start()
{
   init_getbits();  blocksize = 0;
}

/*
Lempel-Ziv decompression.  Mostly based on Tom Pfau's assembly language
code.

This file is public domain.

                                    -- Rahul Dhesi 1991/07/07
*/

#include "booz.h"
#include <stdio.h>

/* must fit in MEM_BLOCK_SIZE */
#define  OUT_BUF_SIZE    4096
#define  IN_BUF_SIZE    4096

#define  STACKSIZE   2000
#define  INBUFSIZ    (IN_BUF_SIZE - SPARE)
#define  OUTBUFSIZ   (OUT_BUF_SIZE - SPARE)
#define  MEMERR      2
#define  IOERR       1
#define  MAXBITS     13
#define  CLEAR       256         /* clear code */
#define  Z_EOF       257         /* end of file marker */
#define  FIRST_FREE  258         /* first free code */
#define  MAXMAX      8192        /* max code + 1 */
#define  SPARE       4

char out_buf_adr[MEM_BLOCK_SIZE];
#define in_buf_adr   (&out_buf_adr[OUT_BUF_SIZE])

struct tabentry {
   unsigned next;
   char z_ch;
};

static struct tabentry *table;
static int gotmem = 0;

static int init_dtab();
static unsigned rd_dcode();
static int wr_dchar();
static int ad_dcode();

static unsigned lzd_sp = 0;
static unsigned lzd_stack[STACKSIZE + SPARE];

static int push(ch)
int ch;
{
   lzd_stack[lzd_sp++] = ch;
   if (lzd_sp >= STACKSIZE)
      prterror ('f', "Stack overflow in lzd()\n", (char *) 0, (char *) 0);
}
   
#define  pop()    (lzd_stack[--lzd_sp])

unsigned cur_code;
unsigned old_code;
unsigned in_code;

unsigned free_code;
int nbits;
unsigned max_code;

char fin_char;
char k;
unsigned masks[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff };
unsigned bit_offset;
unsigned output_offset;
static FILE *in_file;
static FILE *out_file; 

int lzd(input_file, output_file)
FILE *input_file, *output_file;          /* input & output file handles */
{
   in_file = input_file;                 /* make it avail to other fns */
   out_file = output_file;               /* ditto */
   nbits = 9;
   max_code = 512;
   free_code = FIRST_FREE;
   lzd_sp = 0;
   bit_offset = 0;
   output_offset = 0;

   if (gotmem == 0) {
      table = (struct tabentry *) 
         malloc (MAXMAX * sizeof (struct tabentry) + SPARE);
      gotmem++;
   }
   if (table == (struct tabentry *) 0)
      memerr();

   fread(in_buf_adr, 1, INBUFSIZ, in_file);
   if (ferror(in_file))
      return(IOERR);

   init_dtab();             /* initialize table */

loop:
   cur_code = rd_dcode();
   if (cur_code == Z_EOF) {
      if (output_offset != 0) {
         if (out_file != NULL) {
	   if (fwrite(out_buf_adr, 1, output_offset, out_file) != output_offset)
              prterror ('f', "Output error in lzd()\n", 
						(char *) 0, (char *) 0);
         }
         addbfcrc(out_buf_adr, output_offset);
      }
      return (0);
   }

   if (cur_code == CLEAR) {
      init_dtab();
      fin_char = k = old_code = cur_code = rd_dcode();
      wr_dchar((int) k);
      goto loop;
   }

   in_code = cur_code;
   if (cur_code >= free_code) {        /* if code not in table (k<w>k<w>k) */
      cur_code = old_code;             /* previous code becomes current */
      push(fin_char);
   }

   while (cur_code > 255) {               /* if code, not character */
      push(table[cur_code].z_ch);         /* push suffix char */
      cur_code = table[cur_code].next;    /* <w> := <w>.code */
   }

   k = fin_char = cur_code;
   push(k);
   while (lzd_sp != 0) {
      wr_dchar((int) pop());
   }
   ad_dcode();
   old_code = in_code;

   goto loop;
} /* lzd() */

/* rd_dcode() reads a code from the input (compressed) file and returns
its value. */
static unsigned rd_dcode()
{
   register char *ptra, *ptrb;    /* miscellaneous pointers */
   unsigned word;                     /* first 16 bits in buffer */
   unsigned byte_offset;
   char nextch;                           /* next 8 bits in buffer */
   unsigned ofs_inbyte;               /* offset within byte */

   ofs_inbyte = bit_offset % 8;
   byte_offset = bit_offset / 8;
   bit_offset = bit_offset + nbits;

   if (byte_offset >= INBUFSIZ - 5) {
      int space_left;

      bit_offset = ofs_inbyte + nbits;
      space_left = INBUFSIZ - byte_offset;
      ptrb = byte_offset + in_buf_adr;          /* point to char */
      ptra = in_buf_adr;
      /* we now move the remaining characters down buffer beginning */
      while (space_left > 0) {
         *ptra++ = *ptrb++;
         space_left--;
      }
      fread(ptra, 1, byte_offset, in_file);
      if (ferror(in_file))
         prterror ('f', "Input error in lzd:rd_dcode\n", 
				(char *) 0, (char *) 0);
      byte_offset = 0;
   }
   ptra = byte_offset + in_buf_adr;
   /* NOTE:  "word = *((int *) ptra)" would not be independent of byte order. */
   word = (unsigned char) *ptra; ptra++;
   word = word | ((unsigned char) *ptra) << 8; ptra++;

   nextch = *ptra;
   if (ofs_inbyte != 0) {
      /* shift nextch right by ofs_inbyte bits */
      /* and shift those bits right into word; */
      word = (word >> ofs_inbyte) | (((unsigned)nextch) << (16-ofs_inbyte));
   }
   return (word & masks[nbits]); 
} /* rd_dcode() */

static int init_dtab()
{
   nbits = 9;
   max_code = 512;
   free_code = FIRST_FREE;
}

static int wr_dchar (ch)
int ch;
{
   if (output_offset >= OUTBUFSIZ) {      /* if buffer full */
      if (out_file != NULL) {
         if (fwrite(out_buf_adr, 1, output_offset, out_file) != output_offset)
            prterror ('f', "Write error in lzd:wr_dchar\n", 
					(char *) 0, (char *) 0);
      }
      addbfcrc(out_buf_adr, output_offset);     /* update CRC */
      output_offset = 0;                  /* restore empty buffer */
   }
   out_buf_adr[output_offset++] = ch;        /* store character */
} /* wr_dchar() */

/* adds a code to table */
static int ad_dcode()
{
   table[free_code].z_ch = k;                /* save suffix char */
   table[free_code].next = old_code;         /* save prefix code */
   free_code++;
   if (free_code >= max_code) {
      if (nbits < MAXBITS) {
         nbits++;
         max_code = max_code << 1;        /* double max_code */
      }
   }
}

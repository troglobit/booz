/*
Lempel-Ziv decompression.  Mostly based on Tom Pfau's assembly language
code.  The contents of this file are hereby released to the public domain.
                                 -- Rahul Dhesi 1986/11/14, 1987/02/08
*/

#include "func.h"
#include "options.h"

#define  INBUFSIZ    (IN_BUF_SIZE - SPARE)
#define  OUTBUFSIZ   (OUT_BUF_SIZE - SPARE)
#define  MEMERR      2
#define  IOERR       1
#define  MAXBITS     13
#define  CLEAR       256         /* clear code */
#define  Z_EOF       257         /* end of file marker */
#define  FIRST_FREE  258         /* first free code */
#define  MAXMAX      8192        /* max code + 1 */

#define  SPARE       5

struct tabentry {
   unsigned next;
   char z_ch;
};

struct tabentry *table;
int   gotmem = 0;

int init_dtab();
unsigned rd_dcode();
int wr_dchar();
int ad_dcode();

unsigned lzd_sp = 0;
unsigned lzd_stack[STACKSIZE + SPARE];

int push(ch)
int ch;
{
   lzd_stack[lzd_sp++] = ch;
   if (lzd_sp >= STACKSIZE)
      prterror ('f', "Stack overflow in lzd()\n", (char *) 0, (char *) 0);
}
   
#define  pop()    (lzd_stack[--lzd_sp])

char out_buf_adr[IN_BUF_SIZE + SPARE];
char in_buf_adr[OUT_BUF_SIZE + SPARE];

char memflag = 0;                /* memory allocated? flag */

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
int in_han, out_han; 

int lzd(input_handle, output_handle)
int input_handle, output_handle;          /* input & output file handles */
{
   in_han = input_handle;                 /* make it avail to other fns */
   out_han = output_handle;               /* ditto */
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

   if (read(in_han, in_buf_adr, INBUFSIZ) == -1)
      return(IOERR);

   init_dtab();             /* initialize table */

loop:
   cur_code = rd_dcode();
   if (cur_code == Z_EOF) {
      if (output_offset != 0) {
         if (out_han != -2) {
            if (write(out_han, out_buf_adr, output_offset) != output_offset)
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
      wr_dchar(k);
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
      wr_dchar(pop());
   }
   ad_dcode();
   old_code = in_code;

   goto loop;
} /* lzd() */

/* rd_dcode() reads a code from the input (compressed) file and returns
its value. */
unsigned rd_dcode()
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
      if (read(in_han, ptra, byte_offset) == -1)
         prterror ('f', "I/O error in lzd:rd_dcode\n", 
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

int init_dtab()
{
   nbits = 9;
   max_code = 512;
   free_code = FIRST_FREE;
}

int wr_dchar (ch)
char ch;
{
   if (output_offset >= OUTBUFSIZ) {      /* if buffer full */
      if (out_han != -2) {
         if (write(out_han, out_buf_adr, output_offset) != output_offset)
            prterror ('f', "Write error in lzd:wr_dchar\n", 
					(char *) 0, (char *) 0);
      }
      addbfcrc(out_buf_adr, output_offset);     /* update CRC */
      output_offset = 0;                  /* restore empty buffer */
   }
   out_buf_adr[output_offset++] = ch;        /* store character */
} /* wr_dchar() */

/* adds a code to table */
int ad_dcode()
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

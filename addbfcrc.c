/* adbfcrc.c */

/*
addbfcrc() accepts a buffer address and a count and adds the CRC for
all bytes in the buffer to the global variable crccode using
CRC-16.

This file is public domain.

                                    -- Rahul Dhesi 1991/07/07
*/

#define TABLEN	256

unsigned int crccode;
unsigned int crctab[TABLEN];

int addbfcrc(buffer,count)
char *buffer;
unsigned count;

{
   register unsigned int localcrc;
   register int i;
   localcrc = crccode;

   for (i=0; i<count; i++)
      localcrc = (localcrc>>8) ^ crctab[(localcrc ^ (buffer[i])) & 0x00ff];
   crccode = localcrc;
}

/* gentab() generates table for CRC calculation, as described in
"C Programmer's Guide to Serial Communications" by Joe Campbell */

/* reverse CRC-16 polynomial */
#define CRC_FUNC        (unsigned) 0xa001

unsigned int calcterm();

unsigned int calcterm (data)
register unsigned int data;
{
   int i;
   register unsigned int accum = 0;

   data <<= 1;
   for (i = 8;  i > 0;  i--) {
      data >>= 1;
      if ((data ^ accum) & 0x0001)
         accum = (accum >> 1) ^ CRC_FUNC;
      else
         accum >>= 1;
   }
   return accum;
}

int gentab()
{
   register unsigned int i;
   for (i = 0;  i < TABLEN;  i++)
      crctab[i] = calcterm (i);
}

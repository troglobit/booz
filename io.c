/*$Source: /usr/home/dhesi/booz/RCS/io.c,v $*/
/*$Id: io.c,v 1.8 91/07/08 12:06:55 dhesi Exp $*/
/***********************************************************
Input/output for lzh decoding.

Adapted from "ar" archiver written by Haruhiko Okumura.
***********************************************************/
#include "booz.h"
#include "zoo.h"
#include "ar.h"
#include "lzh.h"

extern FILE *arcfile;
t_uint16 bitbuf;

static uint  subbitbuf;
static int   bitcount;

int fillbuf(n)  /* Shift bitbuf n bits left, read n bits */
int n;
{
	bitbuf <<= n;
	while (n > bitcount) {
		bitbuf |= subbitbuf << (n -= bitcount);
		if (feof(arcfile))
			subbitbuf = 0;
		else
			subbitbuf = (uchar) getc(arcfile);
		bitcount = CHAR_BIT;
	}
	bitbuf |= subbitbuf >> (bitcount -= n);
}

uint getbits(n)
int n;
{
	uint x;

	x = bitbuf >> (BITBUFSIZ - n);  fillbuf(n);
	return x;
}

int fwrite_crc(p, n, f)
uchar *p;
int n;
FILE *f;
{
	if (f != NULL) {
		if (fwrite((char *) p, 1, n, f) < n) 
			prterror('f', "disk full", (char *)0, (char *)0);
	}
	addbfcrc((char *) p, (unsigned) n);
}

int init_getbits()
{
	bitbuf = 0;  subbitbuf = 0;  bitcount = 0;
	fillbuf(BITBUFSIZ);
}

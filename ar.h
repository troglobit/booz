/*$Source: /usr/home/dhesi/booz/RCS/ar.h,v $*/
/*$Id: ar.h,v 1.3 91/07/08 11:07:02 dhesi Exp $*/
/***********************************************************
	ar.h
***********************************************************/

#include <stdio.h>

typedef unsigned char uchar;	/* 8 bits or more */
typedef unsigned int   uint;    /* 16 bits or more */
typedef unsigned short ushort;  /* 16 bits or more */
typedef unsigned long  ulong;   /* 32 bits or more */

/* T_UINT16 must be #defined in options.h to be 
a 16-bit unsigned integer type */

#ifndef T_UINT16
# include "T_UINT16 not defined"
#endif

typedef T_UINT16		  t_uint16;	/* exactly 16 bits */
uint decode_c();
uint decode_p();
uint getbits();
#define DICBIT    13    /* 12(-lh4-) or 13(-lh5-) */
#define DICSIZ ((unsigned) 1 << DICBIT)

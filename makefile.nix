#Make Booz -- makefile for **IX.  Rename to `makefile' before use.

OOZOBJS = addbfcrc.o lzd.o booz.o oozext.o portable.o

cswitch = -c -O -DNIX_IO

.c.o :
	cc $(cswitch) $*.c

booz : $(OOZOBJS)
	cc $(OOZOBJS) -o booz

booz.o : options.h zoo.h func.h booz.c

lzd.o : options.h func.h lzd.c

oozext.o : options.h zoo.h oozio.h func.h oozext.c

portable.o : func.h zoo.h portable.c

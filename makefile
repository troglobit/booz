#Make Booz -- makefile for **IX.  Rename to `makefile' before use.

OOZOBJS = addbfcrc.o lzd.o booz.o oozext.o portable.o \
	io.o huf.o decode.o maketbl.o

CFLAGS = -c -O
# CFLAGS = -c -g
# CFLAGS = -c

.c.o :
	cc $(CFLAGS) $*.c

booz : $(OOZOBJS)
	cc $(OOZOBJS) -o booz

install : booz
	mv booz /usr/local/bin/tbooz

clean :
	-/bin/rm -f core *.o

# DEPENDENCIES follow

# DO NOT DELETE this line

booz.o: /usr/include/stdio.h booz.h zoo.h
decode.o: /usr/include/stdio.h ar.h booz.h lzh.h zoo.h
huf.o: /usr/include/stdio.h ar.h booz.h lzh.h zoo.h
io.o: /usr/include/stdio.h ar.h booz.h lzh.h zoo.h
lzd.o: /usr/include/stdio.h booz.h
maketbl.o: /usr/include/stdio.h ar.h booz.h lzh.h zoo.h
oozext.o: /usr/include/stdio.h booz.h zoo.h
portable.o: /usr/include/stdio.h booz.h zoo.h

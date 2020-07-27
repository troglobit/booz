# Makefile to build booz using Turbo C++ 1.0 and NDMAKE 4.31

OOZOBJS = booz.obj addbfcrc.obj lzd.obj oozext.obj portable.obj \
	io.obj huf.obj decode.obj maketbl.obj

CC = tcc
CFLAGS = -c -O
# CFLAGS = -c -g

.SUFFIXES : .exe .obj .c

.c.obj :
	$(CC) $(CFLAGS) $*.c

booz : booz.exe

booz.exe : $(OOZOBJS)
	$(CC) -ebooz $(OOZOBJS)

install : booz.exe
	copy booz.exe \bin\booz.exe

clean :
	-del *.obj

# DEPENDENCIES follow

booz.obj: booz.h zoo.h
decode.obj: ar.h booz.h lzh.h zoo.h
huf.obj: ar.h booz.h lzh.h zoo.h
io.obj: ar.h booz.h lzh.h zoo.h
lzd.obj: booz.h
maketbl.obj: ar.h booz.h lzh.h zoo.h
oozext.obj: booz.h zoo.h
portable.obj: booz.h zoo.h

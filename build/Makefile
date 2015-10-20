
#CC=gcc
CFLAGS=-Wall -O3
#CC=i386-mingw32msvc-gcc
#PATH=/opt/xmingw/bin:/usr/bin
VPATH=../src

OBJECTS=affs.o bootblock.o fileio.o partition.o rootblock.o

default: $(OBJECTS)
	$(CC) -o ../amiga_recovery ../src/amiga_recovery.c \
	   $(OBJECTS) \
	   $(CFLAGS)


%.o: %.c %.h
	$(CC) -c $< -o $*.o $(CFLAGS)


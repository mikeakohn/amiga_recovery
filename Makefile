
DEBUG=-DDEBUG -g
CFLAGS=-O2 -Wall $(DEBUG)
CC=gcc
#CC=i686-mingw32-gcc

default:
	$(CC) -c affs.c $(CFLAGS)
	$(CC) -c fileio.c $(CFLAGS)
	$(CC) -o amiga_recovery amiga_recovery.c affs.o fileio.o $(CFLAGS)

tools:
	$(CC) -o find_root_block find_root_block.c $(CFLAGS)
	$(CC) -o hex_file_view hex_file_view.c $(CFLAGS)

clean:
	@rm -f *.o
	@rm -f amiga_recovery
	@rm -f find_root_block
	@rm -f hex_file_view
	@rm -f amiga_recovery.exe



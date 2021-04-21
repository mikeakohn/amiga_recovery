
DEBUG=-DDEBUG -g
CFLAGS=-O2 -Wall $(DEBUG)
CC=gcc
#CC=i686-mingw32-gcc

default:
	@+make -C build

tools:
	$(CC) -o find_root_block src/find_root_block.c \
	   build/fileio.o build/rootblock.o \
	   $(CFLAGS) -Isrc
	$(CC) -o find_partitions src/find_partitions.c \
	   build/fileio.o build/partition.o \
	   $(CFLAGS) -Isrc
	$(CC) -o parse_hunk src/parse_hunk.c \
	   $(CFLAGS) -Isrc
clean:
	@rm -f build/*.o
	@rm -f amiga_recovery amiga_recovery.exe
	@rm -f find_root_block find_partitions parse_hunk


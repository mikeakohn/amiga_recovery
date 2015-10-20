
DEBUG=-DDEBUG -g
CFLAGS=-O2 -Wall $(DEBUG)
CC=gcc
#CC=i686-mingw32-gcc

default:
	@+make -C build

tools:
	$(CC) -o find_root_block find_root_block.c $(CFLAGS)
	$(CC) -o hex_file_view hex_file_view.c $(CFLAGS)

clean:
	@rm -f build/*.o
	@rm -f amiga_recovery amiga_recovery.exe
	@rm -f find_root_block
	@rm -f hex_file_view



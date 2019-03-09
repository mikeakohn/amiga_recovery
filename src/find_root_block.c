/*
  
Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2019 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

find_root_block - Scan a disk image for a root block.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "rootblock.h"

#define UINT32(a,i) ((a[i+0] << 24) + (a[i+1] << 16) + (a[i+2] << 8) + a[i+3])

int main(int argc, char *argv[])
{
  FILE *in;
  struct _amiga_rootblock rootblock;
  uint8_t buffer[512];
  long marker;
  int len;

  if (argc != 2)
  {
    printf("Usage: %s <filename>\n", argv[0]);
    exit(0);
  }

  in = fopen(argv[1], "rb");

  if (in == NULL)
  {
    printf("Cannot open file %s for reading.\n", argv[1]);
    exit(1);
  }

  while(1)
  {
    len = fread(buffer, 512, 1, in);
    if (len == 0) { break; }

    if (UINT32(buffer, 0) == 0x02 &&
        UINT32(buffer, 4) == 0x00 &&
        UINT32(buffer, 8) == 0x00 &&
        UINT32(buffer, 16) == 0x00)
    {

      marker = ftell(in);
      fseek(in, marker - 512, SEEK_SET);

      if (read_rootblock_data(in, &rootblock) == 0 && 
          rootblock.hash_table_size != 0)
      {
        printf("Possible Rootblock at %ld block=%d\n", marker, (int)marker / 512);
        print_rootblock(&rootblock);
      }

      fseek(in, marker, SEEK_SET);
    }
  }

  fclose(in);

  return 0;
}



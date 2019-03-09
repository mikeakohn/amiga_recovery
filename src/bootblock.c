/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2019 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "bootblock.h"
#include "fileio.h"

int read_bootblock(FILE *in, struct _amiga_bootblock *bootblock)
{
  int n;

  for (n = 0; n < 16; n++)
  {
    fseek(in, n * 512, SEEK_SET);
    read_chars(in, bootblock->magic, 4);
    bootblock->size = read_int(in);
    bootblock->checksum = read_int(in);
    bootblock->scsihost = read_int(in);
    bootblock->blksz = read_int(in);
    bootblock->flags = read_int(in);
    bootblock->badblcklst = read_int(in);
    bootblock->partitionlst = read_int(in);
    bootblock->fslst = read_int(in);

    if (memcmp(bootblock->magic, "RDSK", 4) == 0)
    {
      // NOTE: In my Amiga image this is not needed.  RDSK is always 0.
      // In an image someone sent me, bootblock is on block 3 and points
      // to the first partition on block 3, and everything is is who knows
      // where.  I added this to help locate his partitions.
      bootblock->offset = 512 * n;
      return n;
    }
  }

  bootblock->offset = 0;

  return -1;
}

void print_bootblock(struct _amiga_bootblock *bootblock)
{
  printf("================== Boot Block ===================\n");
  printf("            magic: %4.4s\n", bootblock->magic);
  printf("             size: %d\n", bootblock->size);
  printf("         checksum: %d\n", bootblock->checksum);
  printf("         scsihost: %d\n", bootblock->scsihost);
  printf("            blksz: %d\n", bootblock->blksz);
  printf("            flags: %d\n", bootblock->flags);
  printf("       badblcklst: %d\n", bootblock->badblcklst);
  printf("     partitionlst: %d\n", bootblock->partitionlst);
  printf("            fslst: %d\n", bootblock->fslst);
  printf("\n");
}


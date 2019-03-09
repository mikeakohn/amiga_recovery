/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2019 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

*/

#ifndef BOOTBLOCK_H
#define BOOTBLOCK_H

#include <stdint.h>

// This really should come from the rootblock.. but I'm lazy right now
#define BSIZE 512

struct _amiga_bootblock
{
  unsigned char magic[4];
  uint32_t size;
  int checksum;
  uint32_t scsihost;
  uint32_t blksz;
  uint32_t flags;
  uint32_t badblcklst;
  uint32_t partitionlst;
  uint32_t fslst;

  // Offset on the disk to the bootblock
  long offset;
};

int read_bootblock(FILE *in, struct _amiga_bootblock *bootblock);
void print_bootblock(struct _amiga_bootblock *bootblock);

#endif


/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#ifndef BOOTBLOCK_H
#define BOOTBLOCK_H

// This really should come from the rootblock.. but I'm lazy right now
#define BSIZE 512

struct _amiga_bootblock
{
  unsigned char magic[4];
  unsigned int size;
  int checksum;
  unsigned int scsihost;
  unsigned int blksz;
  unsigned int flags;
  unsigned int badblcklst;
  unsigned int partitionlst;
  unsigned int fslst;

  // Offset on the disk to the bootblock
  long offset;
};

int read_bootblock(FILE *in, struct _amiga_bootblock *bootblock);
void print_bootblock(struct _amiga_bootblock *bootblock);

#endif


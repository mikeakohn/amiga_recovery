/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#ifndef DATABLOCK_H
#define DATABLOCK_H

#include "affs.h"

struct _amiga_datablock
{
  uint32_t type;
  uint32_t header_key;
  uint32_t seq_num;
  uint32_t data_size;
  uint32_t next_data;
  uint32_t checksum;
  unsigned char data[BSIZE-24];
};

void read_datablock(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_datablock *datablock, uint32_t block);
void print_datablock(struct _amiga_datablock *datablock);

#endif


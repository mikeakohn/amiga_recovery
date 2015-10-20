/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#ifndef ROOTBLOCK_H
#define ROOTBLOCK_H

#include "bootblock.h"
#include "partition.h"

struct _amiga_rootblock
{
  unsigned int type;
  unsigned int header_key;
  unsigned int high_seq;
  unsigned int hash_table_size;
  unsigned int first_size;
  unsigned int checksum;
  unsigned int hash_table[BSIZE/4-56];
  unsigned int bm_flag;
  unsigned int bm_pages[25];
  unsigned int bm_ext;
  unsigned int r_days;
  unsigned int r_mins;
  unsigned int r_ticks;
  unsigned char diskname[32];  // name_len + 30 possible + 1 unused
  unsigned int unused1[2];
  unsigned int v_days;
  unsigned int v_min;
  unsigned int v_ticks;
  unsigned int c_days;
  unsigned int c_min;
  unsigned int c_ticks;
  unsigned int next_hash;
  unsigned int parent_dir;
  unsigned int extension;
  unsigned int sec_type;

  // DEBUG STUFF
  unsigned int partition_offset;
  unsigned int disk_offset;
};

void read_rootblock(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_rootblock *rootblock);
void print_rootblock(struct _amiga_rootblock *rootblock);

#endif


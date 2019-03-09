/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2019 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

*/

#ifndef ROOTBLOCK_H
#define ROOTBLOCK_H

#include "bootblock.h"
#include "partition.h"

#define UINT32(a,i) ((a[i+0] << 24) + (a[i+1] << 16) + (a[i+2] << 8) + a[i+3])

struct _amiga_rootblock
{
  uint32_t type;
  uint32_t header_key;
  uint32_t high_seq;
  uint32_t hash_table_size;
  uint32_t first_size;
  uint32_t checksum;
  uint32_t hash_table[BSIZE / 4 - 56];
  uint32_t bm_flag;
  uint32_t bm_pages[25];
  uint32_t bm_ext;
  uint32_t r_days;
  uint32_t r_mins;
  uint32_t r_ticks;
  unsigned char diskname[32];  // name_len + 30 possible + 1 unused
  uint32_t unused1[2];
  uint32_t v_days;
  uint32_t v_min;
  uint32_t v_ticks;
  uint32_t c_days;
  uint32_t c_min;
  uint32_t c_ticks;
  uint32_t next_hash;
  uint32_t parent_dir;
  uint32_t extension;
  uint32_t sec_type;

  // DEBUG STUFF
  uint32_t partition_offset;
  uint32_t disk_offset;
};

int read_rootblock_data(FILE *in, struct _amiga_rootblock *rootblock);

void read_rootblock(
  FILE *in,
  struct _amiga_bootblock *bootblock,
  struct _amiga_partition *partition,
  struct _amiga_rootblock *rootblock);

void print_rootblock(struct _amiga_rootblock *rootblock);

uint32_t find_root_block(FILE *in, struct _amiga_rootblock *rootblock);

#endif


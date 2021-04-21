/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2021 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

*/

#ifndef PARTITION_H
#define PARTITION_H

#include <stdint.h>

#include "bootblock.h"

struct _amiga_partition
{
  uint8_t magic[4];
  int size;
  int checksum;
  uint32_t scsihost;
  uint32_t next;
  uint32_t flags;
  uint32_t unused1[2];
  uint32_t devflags;
  unsigned char name[32];
  uint32_t unused2[15];

  uint32_t table_size;
  uint32_t size_block;
  uint32_t sec_org;
  uint32_t heads;
  uint32_t sectors_per_block;
  uint32_t blocks_per_track;
  uint32_t num_reserved;
  uint32_t pre_alloc;
  uint32_t interleave;
  uint32_t start_cyl;
  uint32_t end_cyl;
  uint32_t num_buffers;
  uint32_t buff_type;
  uint32_t max_transfer;
  uint32_t mask;
  int  boot_priority;
  uint8_t type[4];
  uint32_t baud;
  uint32_t control;
  uint32_t bootblocks;

  uint32_t start;
  uint32_t end;
  uint32_t size_in_bytes;
};

int read_partition(
  FILE *in,
  struct _amiga_bootblock *bootblock,
  struct _amiga_partition *partition);

void print_partition(struct _amiga_partition *partition);

void print_partition_list(FILE *in, struct _amiga_bootblock *bootblock);

void show_partitions(FILE *in, struct _amiga_bootblock *bootblock);

int dump_partition(
  FILE *in,
  struct _amiga_bootblock *bootblock,
  int num,
  const char *name);

int read_partition_num(
  FILE *in,
  struct _amiga_bootblock *bootblock,
  struct _amiga_partition *partition,
  int num);

int get_partition_num(FILE *in, struct _amiga_bootblock *bootblock, char *name);

#endif


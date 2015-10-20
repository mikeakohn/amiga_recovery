/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#ifndef PARTITION_H
#define PARTITION_H

#include "bootblock.h"

struct _amiga_partition
{
  unsigned char magic[4];
  int size;
  int checksum;
  unsigned int scsihost;
  unsigned int next;
  unsigned int flags;
  unsigned int unused1[2];
  unsigned int devflags;
  unsigned char name[32];
  unsigned int unused2[15];

  unsigned int table_size;
  unsigned int size_block;
  unsigned int sec_org;
  unsigned int heads;
  unsigned int sectors_per_block;
  unsigned int blocks_per_track;
  unsigned int num_reserved;
  unsigned int pre_alloc;
  unsigned int interleave;
  unsigned int start_cyl;
  unsigned int end_cyl;
  unsigned int num_buffers;
  unsigned int buff_type;
  unsigned int max_transfer;
  unsigned int mask;
  int  boot_priority;
  unsigned char type[4];
  unsigned int baud;
  unsigned int control;
  unsigned int bootblocks;

  unsigned int start;
  unsigned int end;
  unsigned int size_in_bytes;
};

int read_partition(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition);
void print_partition(struct _amiga_partition *partition);
void print_partition_list(FILE *in, struct _amiga_bootblock *bootblock);
void show_partitions(FILE *in, struct _amiga_bootblock *bootblock);
int dump_partition(FILE *in, struct _amiga_bootblock *bootblock, int num, const char *name);
int read_partition_num(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, int num);
int get_partition_num(FILE *in, struct _amiga_bootblock *bootblock, char *name);

#endif


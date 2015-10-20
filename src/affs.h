/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#ifndef AFFS_H
#define AFFS_H

#include "bootblock.h"
#include "directory.h"
#include "fileheader.h"
#include "partition.h"
#include "rootblock.h"

struct _amiga_file_ext
{
  uint32_t type;
  uint32_t header_key;
  uint32_t high_seq;
  uint32_t unused1;
  uint32_t unused2;
  uint32_t checksum;
  uint32_t datablocks[BSIZE/4-56];
  uint32_t info[46];
  uint32_t unused3;
  uint32_t parent;
  uint32_t extension;
  uint32_t sec_type;
};

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

void read_file_ext(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_file_ext *file_ext, uint32_t block);
void read_datablock(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_datablock *datablock, uint32_t block);

void print_file(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd, char *filename, FILE *out);

void print_file_ext(struct _amiga_file_ext *file_ext);
void print_datablock(struct _amiga_datablock *datablock);

uint32_t hash_name(unsigned char *name);
void print_hash_info(FILE *in, struct _amiga_rootblock *rootblock, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, uint32_t block);

int get_sec_type(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, uint32_t block);

#endif


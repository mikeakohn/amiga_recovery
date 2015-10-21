/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <stdint.h>

#include "bootblock.h"
#include "fileheader.h"
#include "partition.h"
#include "rootblock.h"

struct _amiga_directory
{
  uint32_t type;
  uint32_t header_key;
  uint32_t unused1[3];
  uint32_t checksum;
  uint32_t hash_table[BSIZE/4-56];
  uint32_t unused2[2];
  uint16_t uid;
  uint16_t gid;
  uint32_t protect;
  uint32_t unused3;
  unsigned char comment[80]; // first char is len
  unsigned char unused4[12];
  uint32_t days;
  uint32_t mins;
  uint32_t ticks;
  unsigned char dirname[32]; // first char is len, last char is unused
  uint32_t unused5[2];
  uint32_t next_link;
  uint32_t unused6[5];
  uint32_t hash_chain;
  uint32_t parent;
  uint32_t extension;
  uint32_t sec_type;
};

// I didn't want to put this here, but owell
struct _pwd
{
  struct _amiga_rootblock rootblock;
  struct _amiga_partition partition;
  int partition_num;
  char cwd[4096];  // FIXME - check bounds
  uint32_t dir_hash[BSIZE/4-56];
  uint32_t hash_table_size;
  uint32_t parent_dir;
};

void read_directory(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_directory *directory, uint32_t block);
void list_directory(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd);
void print_directory(struct _amiga_directory *directory);
int ch_dir(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd, char *dirname);

#endif


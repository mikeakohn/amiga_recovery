/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#ifndef FILEHEADER_H
#define FILEHEADER_H

#include <stdint.h>

#include "bootblock.h"
#include "partition.h"
#include "rootblock.h"

struct _amiga_fileheader
{
  uint32_t type;
  uint32_t header_key;
  uint32_t high_seq;
  uint32_t data_size;
  uint32_t first_data;
  uint32_t checksum;
  uint32_t datablocks[BSIZE/4-56];  //FIXME - wrong size?
  uint32_t unused1;
  uint16_t uid;
  uint16_t gid;
  uint32_t protect;
  uint32_t byte_size;
  unsigned char comment[80]; // first char is len
  unsigned char unused2[12];
  uint32_t days;
  uint32_t mins;
  uint32_t ticks;
  unsigned char filename[32]; // first char is len, last char is unused
  uint32_t unused3;
  uint32_t read_entry;
  uint32_t next_link;
  uint32_t unused4[5];
  uint32_t hash_chain;
  uint32_t parent;
  uint32_t extension;
  uint32_t sec_type;
};

void read_fileheader(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_fileheader *fileheader, uint32_t block);
void print_fileheader(struct _amiga_fileheader *fileheader);

#endif


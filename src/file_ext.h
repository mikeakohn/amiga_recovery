/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2019 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

*/

#ifndef FILE_EXT_H
#define FILE_EXT_H

#include "affs.h"

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

void read_file_ext(
  FILE * in,
  struct _amiga_bootblock *bootblock,
  struct _amiga_partition *partition,
  struct _amiga_file_ext *file_ext,
  uint32_t block);

void print_file_ext(struct _amiga_file_ext *file_ext);

#endif


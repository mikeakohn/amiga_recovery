/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2019 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "file_ext.h"
#include "fileio.h"

void read_file_ext(
  FILE * in,
  struct _amiga_bootblock *bootblock,
  struct _amiga_partition *partition,
  struct _amiga_file_ext *file_ext,
  uint32_t block)
{
  int t;

  fseek(in, partition->start + (block * bootblock->blksz), SEEK_SET);

  file_ext->type = read_int(in);
  file_ext->header_key = read_int(in);
  file_ext->high_seq = read_int(in);
  //uint32_t unused1;
  //uint32_t unused2;
  fseek(in, 2 * 4, SEEK_CUR);
  file_ext->checksum = read_int(in);
  //uint32_t datablocks[BSIZE/4-56]
  for (t = 0; t<(512 / 4 - 56); t++)
  {
    file_ext->datablocks[t] = read_int(in);
  }
  //uint32_t info[46];
  fseek(in, 46 * 4, SEEK_CUR);
  file_ext->unused3 = read_int(in);
  file_ext->parent = read_int(in);
  file_ext->extension = read_int(in);
  file_ext->sec_type = read_int(in);
}

void print_file_ext(struct _amiga_file_ext *file_ext)
{
  int t;

  printf("================== File Extension ===================\n");
  printf("             type: %d\n", file_ext->type);
  printf("       header_key: %d\n", file_ext->header_key);
  printf("         high_seq: %d\n", file_ext->high_seq);
  printf("         checksum: %d\n", file_ext->checksum);
  printf("       datablocks: ");
  for (t = 0; t < (512 / 4 - 56); t++)
  {
    if (t != 0) { printf(" "); }
    printf("%d", file_ext->datablocks[t]);
  }
  printf("\n");
  printf("           parent: %d\n", file_ext->parent);
  printf("        extension: %d\n", file_ext->extension);
  printf("         sec_type: %d\n", file_ext->sec_type);
}


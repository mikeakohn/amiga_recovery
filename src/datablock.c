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

#include "datablock.h"
#include "fileio.h"

void read_datablock(
  FILE * in,
  struct _amiga_bootblock *bootblock,
  struct _amiga_partition *partition,
  struct _amiga_datablock *datablock,
  uint32_t block)
{
  int t;

  fseek(in, partition->start + (block * bootblock->blksz), SEEK_SET);

  datablock->type = read_int(in);
  datablock->header_key = read_int(in);
  datablock->seq_num = read_int(in);
  datablock->data_size = read_int(in);
  datablock->next_data = read_int(in);
  datablock->checksum = read_int(in);

  for (t = 0; t < 512 - 24; t++)
  {
    datablock->data[t] = getc(in);
  }
}

void print_datablock(struct _amiga_datablock *datablock)
{
  char ascii[17];
  int ptr;
  int t;

  printf("================== Datablock ===================\n");
  printf("             type: %d\n", datablock->type);
  printf("       header_key: %d\n", datablock->header_key);
  printf("          seq_num: %d\n", datablock->seq_num);
  printf("        data_size: %d\n", datablock->data_size);
  printf("        next_data: %d\n", datablock->next_data);
  printf("         checksum: %d\n", datablock->checksum);
  printf("             data:\n");

  if (datablock->type != 8) return;
  ptr = 0;

  for (t = 0; t < datablock->data_size; t++)
  {
    if ((t % 16) == 0)
    {
      ascii[ptr] = 0;
      if (ptr != 0) printf("  %s",ascii);
      printf("\n");
      ptr = 0;
    }

    if ((t % 16) != 0) printf(" ");
    printf("%02x", datablock->data[t]);

    if (datablock->data[t] >= 32 && datablock->data[t] < 127)
    { ascii[ptr++] = datablock->data[t]; }
      else
    { ascii[ptr++] = '.'; }
  }

  //FIXME - stuff missing at bottom..
}


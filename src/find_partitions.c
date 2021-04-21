/*
  
Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2021 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

find_partitions - Scan a disk image for partitions.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "fileio.h"
#include "partition.h"

#define UINT32(a,i) ((a[i+0] << 24) + (a[i+1] << 16) + (a[i+2] << 8) + a[i+3])

int show_partition(FILE *in)
{
  struct _amiga_partition partition;

  //long marker = ftell(in);

  read_chars(in, partition.magic, 4);

  if (memcmp(partition.magic, "PART", 4) != 0)
  {
    return -1;
  }

  partition.size = read_int(in);
  partition.checksum = read_int(in);
  partition.scsihost = read_int(in);
  partition.next = read_int(in);
  partition.flags = read_int(in);
  fseek(in, 4 * 2, SEEK_CUR);
  partition.devflags = read_int(in);
  int namelen = getc(in);

  read_chars(in, partition.name, 31);
  partition.name[namelen] = 0;

  fseek(in, 4 * 15, SEEK_CUR);

  partition.table_size = read_int(in);
  partition.size_block = read_int(in);
  partition.sec_org = read_int(in);
  partition.heads = read_int(in);
  partition.sectors_per_block = read_int(in);
  partition.blocks_per_track = read_int(in);
  partition.num_reserved = read_int(in);
  partition.pre_alloc = read_int(in);
  partition.interleave = read_int(in);
  partition.start_cyl = read_int(in);
  partition.end_cyl = read_int(in);
  partition.num_buffers = read_int(in);
  partition.buff_type = read_int(in);
  partition.max_transfer = read_int(in);
  partition.mask = read_int(in);
  partition.boot_priority = read_int(in);

  read_chars(in, partition.type, 4);

  if (partition.type[0] == 'D' &&
      partition.type[1] == 'O' &&
      partition.type[2] == 'S')
  {
    partition.type[3] += '0';
  }

  partition.baud = read_int(in);
  partition.control = read_int(in);
  partition.bootblocks = read_int(in);

  int cylindar_size =
    //bootblock.blksz *
    512 *
    partition.heads *
    partition.sectors_per_block *
    partition.blocks_per_track;

  partition.start = cylindar_size*partition.start_cyl;

  partition.size_in_bytes =
    (partition.end_cyl-partition.start_cyl + 1) * cylindar_size;
  partition.end = partition.start+partition.size_in_bytes;

  print_partition(&partition);

  return 0;
}

int main(int argc, char *argv[])
{
  FILE *in;
  char *part = "PART";
  int ptr = 0, count = 0;

  if (argc != 2)
  {
    printf("Usage: %s <filename>\n", argv[0]);
    exit(0);
  }

  in = fopen(argv[1], "rb");

  if (in == NULL)
  {
    printf("Cannot open file %s for reading.\n", argv[1]);
    exit(1);
  }

  while (1) 
  {
    int ch = getc(in);

    if (ch == EOF) { break; }

    if (ch == part[ptr])
    {
      ptr++;

      if (ptr == 4)
      {
        fseek(in, -4, SEEK_CUR);

        printf("Found at %ld (0x%04lx)\n", ftell(in), ftell(in));

        show_partition(in);
        ptr = 0;
        count++;
      }
    }
      else
    {
      ptr = 0;
    }
  }

  printf("Found %d partitions\n", count);

  fclose(in);

  return 0;
}


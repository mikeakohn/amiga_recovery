/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2021 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "affs.h"
#include "partition.h"
#include "fileio.h"

int read_partition(
  FILE *in,
  struct _amiga_bootblock *bootblock,
  struct _amiga_partition *partition)
{
  uint32_t cylindar_size;
  int namelen;

  // Should be "PART"
  read_chars(in, partition->magic, 4);

  if (memcmp(partition->magic, "PART", 4) != 0)
  {
    printf("Error: Partition's magic number is '%c%c%c%c' instead of'PART'\n",
      partition->magic[0],
      partition->magic[1],
      partition->magic[2],
      partition->magic[3]);
  }

  partition->size = read_int(in);
  partition->checksum = read_int(in);
  partition->scsihost = read_int(in);
  partition->next = read_int(in);
  partition->flags = read_int(in);
  //uint32_t partition->unused1[2];
  fseek(in, 4 * 2, SEEK_CUR);
  //read_int(in);
  //read_int(in);
  partition->devflags = read_int(in);
  namelen = getc(in);

  read_chars(in, partition->name, 31);
  partition->name[namelen] = 0;

  //uint32_t partition->unused2[15];
  //uint32_t partition->unused3[3];
  fseek(in, 4 * 15, SEEK_CUR);
  //partition->heads = read_int(in);
  //partition->unused4 = read_int(in);
  //partition->block_per_track = read_int(in);
  //uint32_t partition->unused5[3];
  //fseek(in, 4*3, SEEK_CUR);
  //partition->lowcyl = read_int(in);
  //partition->highcyl = read_int(in);
  //partition->firstcyl = read_int(in);

  //fseek(in, 0x3a, SEEK_CUR);

  partition->table_size = read_int(in);
  partition->size_block = read_int(in);
  partition->sec_org = read_int(in);
  partition->heads = read_int(in);
  partition->sectors_per_block = read_int(in);
  partition->blocks_per_track = read_int(in);
  partition->num_reserved = read_int(in);
  partition->pre_alloc = read_int(in);
  partition->interleave = read_int(in);
  partition->start_cyl = read_int(in);
  partition->end_cyl = read_int(in);
  partition->num_buffers = read_int(in);
  partition->buff_type = read_int(in);
  partition->max_transfer = read_int(in);
  partition->mask = read_int(in);
  partition->boot_priority = read_int(in);

  read_chars(in, partition->type, 4);

  if (partition->type[0] == 'D' &&
      partition->type[1] == 'O' &&
      partition->type[2] == 'S')
  {
    partition->type[3] += '0';
  }

  partition->baud = read_int(in);
  partition->control = read_int(in);
  partition->bootblocks = read_int(in);

  cylindar_size =
    bootblock->blksz *
    partition->heads *
    partition->sectors_per_block *
    partition->blocks_per_track;

  partition->start = (uint64_t)cylindar_size * (uint64_t)partition->start_cyl;

  partition->size_in_bytes =
    (uint64_t)(partition->end_cyl-partition->start_cyl + 1) *
    (uint64_t)cylindar_size;
  partition->end = partition->start + partition->size_in_bytes;

  return 0;
}

void print_partition(struct _amiga_partition *partition)
{
  printf("===================== Partition ======================\n");
  printf("            magic: %4.4s\n", partition->magic);
  printf("             size: %d\n", partition->size);
  printf("         checksum: %d\n", partition->checksum);
  printf("         scsihost: %d\n", partition->scsihost);
  printf("             next: %d\n", partition->next);
  printf("            flags: %d\n", partition->flags);
  // partition->unused1[2];
  printf("         devflags: %d\n", partition->devflags);
  printf("             name: %s\n", partition->name);
  printf("       table_size: %d\n", partition->table_size);
  printf("       size_block: %d\n", partition->size_block);
  printf("          sec_org: %d\n", partition->sec_org);
  printf("            heads: %d\n", partition->heads);
  printf("sectors_per_block: %d\n", partition->sectors_per_block);
  printf(" blocks_per_track: %d\n", partition->blocks_per_track);
  printf("     num_reserved: %d\n", partition->num_reserved);
  printf("        pre_alloc: %d\n", partition->pre_alloc);
  printf("       interleave: %d\n", partition->interleave);
  printf("        start_cyl: %d (addr: %lu)\n", partition->start_cyl, partition->start);
  printf("          end_cyl: %d (addr: %lu)\n", partition->end_cyl, partition->end);
  printf("                  %lu bytes\n", partition->size_in_bytes);
  printf("      num_buffers: %d\n", partition->num_buffers);
  printf("        buff_type: %d\n", partition->buff_type);
  printf("     max_transfer: %d\n", partition->max_transfer);
  printf("             mask: %d\n", partition->mask);
  printf("    boot_priority: %d\n", partition->boot_priority);

  printf("             type: %4.4s\n", partition->type);

  printf("             baud: %d\n", partition->baud);
  printf("          control: %d\n", partition->control);
  printf("       bootblocks: %d\n", partition->bootblocks);

  printf("\n");
}

void print_partition_list(FILE *in, struct _amiga_bootblock *bootblock)
{
  struct _amiga_partition partition;
  int count, t;

  count = 0;
  t = bootblock->partitionlst;

  while (t > 0)
  {
    fseek(in, t * 512, SEEK_SET);
    read_partition(in, bootblock, &partition);
    print_partition(&partition);

    t = partition.next;
    count++;
  }
}

void show_partitions(FILE *in, struct _amiga_bootblock *bootblock)
{
  struct _amiga_partition partition;
  int count, t;

  count = 0;
  t = bootblock->partitionlst;

  printf("%-20s %4s %10s %10s %12s %12s\n",
    "Name", "Type", "Start Cyl", "End Cyl", "Offset", "Size");

  while (t > 0)
  {
    fseek(in, t * 512, SEEK_SET);
    read_partition(in, bootblock, &partition);

    printf("%-20s %4.4s %10d %10d %12ld %12ld\n",
      partition.name,
      partition.type,
      partition.start_cyl,
      partition.end_cyl,
      partition.start,
      partition.size_in_bytes);

    t = partition.next;
    count++;
  }
}

int dump_partition(
  FILE *in,
  struct _amiga_bootblock *bootblock,
  int num,
  const char *filename)
{
  struct _amiga_partition partition;
  int count,t;
  FILE *out;
  uint32_t size;
  uint8_t buffer[8192];

  count = 0;
  t = bootblock->partitionlst;

  while (t > 0)
  {
    fseek(in, t * 512, SEEK_SET);
    read_partition(in, bootblock, &partition);
    //print_partition(&partition);

    if (count == num)
    {
      printf("Creating %s from partition %d (%s)\n", filename, num, partition.name);
      out = fopen(filename, "wb");
      if (out == NULL)
      {
        printf("Could not open file %s for writing...\n", filename);
        return -1;
      }

      size = partition.size_in_bytes;

      fseek(in, partition.start, SEEK_SET);

      while (size > 0)
      {
        if (size > 8192)
        {
          t = fread(buffer, 1, 8192, in);
          if (t < 1) { printf("wtf\n"); break; }
          fwrite(buffer, 1, t, out);
          size -= t;
        }
        else
        {
          t = fread(buffer, 1, size, in);
          if (t < 1) { printf("wtf\n"); break; }
          fwrite(buffer, 1, t, out);
          size -= t;
        }
      }

      fclose(out);

      return 0;
    }

    t=partition.next;
    count++;
  }

  return -1;
}

int read_partition_num(
  FILE *in,
  struct _amiga_bootblock *bootblock,
  struct _amiga_partition *partition,
  int num)
{
  int count, t;

  count = 0;
  t = bootblock->partitionlst;
  while (t > 0)
  {
    fseek(in, t * 512, SEEK_SET);
    read_partition(in, bootblock, partition);

    if (count == num)
    {
      return 0;
    }

    t = partition->next;
    count++;
  }

  return -1;
}

int get_partition_num(FILE *in, struct _amiga_bootblock *bootblock, char *name)
{
  struct _amiga_partition partition;
  int count, t;

  count = 0;
  t = bootblock->partitionlst;
  while (t > 0)
  {
    fseek(in, t * 512, SEEK_SET);
    read_partition(in, bootblock, &partition);

    if (strcmp((char *)partition.name, name)==0)
    {
      return count;
    }

    t = partition.next;
    count++;
  }

  return -1;
}


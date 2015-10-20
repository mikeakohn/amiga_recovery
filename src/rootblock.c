/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "fileio.h"
#include "rootblock.h"

static uint32_t calc_rootblock(struct _amiga_bootblock *bootblock, struct _amiga_partition *partition)
{
  return (((((partition->end_cyl - partition->start_cyl + 1) *
            partition->heads * partition->blocks_per_track) - 1) +
            partition->num_reserved) / 2) * bootblock->blksz;
}

void read_rootblock(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_rootblock *rootblock)
{
  uint32_t offset;
  int namelen;
  int t;

  offset = calc_rootblock(bootblock, partition); // + bootblock->offset;

  rootblock->partition_offset = offset;
  rootblock->disk_offset = offset + partition->start;

  fseek(in, offset + partition->start, SEEK_SET);
  rootblock->type = read_int(in);
  rootblock->header_key = read_int(in);
  rootblock->high_seq = read_int(in);
  rootblock->hash_table_size = read_int(in);
  rootblock->first_size = read_int(in);
  rootblock->checksum = read_int(in);

  for (t = 0; t < rootblock->hash_table_size; t++)
  {
    rootblock->hash_table[t] = read_int(in);
    //hash_table[512/4-56];
  }
  rootblock->bm_flag = read_int(in);
  for (t = 0; t < 25; t++)
  {
    rootblock->bm_pages[t] = read_int(in);
  }
  rootblock->bm_ext = read_int(in);
  rootblock->r_days = read_int(in);
  rootblock->r_mins = read_int(in);
  rootblock->r_ticks = read_int(in);

  namelen = getc(in);

  read_chars(in, rootblock->diskname, 31);
  rootblock->diskname[namelen] = 0;

  //uint32_t unused1[2];
  read_int(in);
  read_int(in);
  rootblock->v_days = read_int(in);
  rootblock->v_min = read_int(in);
  rootblock->v_ticks = read_int(in);
  rootblock->c_days = read_int(in);
  rootblock->c_min = read_int(in);
  rootblock->c_ticks = read_int(in);
  rootblock->next_hash = read_int(in);
  rootblock->parent_dir = read_int(in);
  rootblock->extension = read_int(in);
  rootblock->sec_type = read_int(in);
}

void print_rootblock(struct _amiga_rootblock *rootblock)
{
  int t;

  printf("===================== Rootblock ======================\n");

  printf("             type: %d\n", rootblock->type);
  printf("       header_key: %d\n", rootblock->header_key);
  printf("         high_seq: %d\n", rootblock->high_seq);
  printf("  hash_table_size: %d\n", rootblock->hash_table_size);
  printf("       first_size: %d\n", rootblock->first_size);
  printf("         checksum: %d\n", rootblock->checksum);
  printf("       hash_table: ");
  for (t = 0; t < rootblock->hash_table_size; t++)
  {
    if (t != 0) printf(" ");
    printf("%d", rootblock->hash_table[t]);
  }
  printf("\n");
  printf("          bm_flag: %d\n", rootblock->bm_flag);
  printf("         bm_pages: ");
  for (t = 0; t < 25; t++)
  {
    if (t != 0) printf(" ");
    printf("%d", rootblock->bm_pages[t]);
  }
  printf("\n");
  printf("           bm_ext: %d\n", rootblock->bm_ext);
  printf("           r_days: %d\n", rootblock->r_days);
  printf("           r_mins: %d\n", rootblock->r_mins);
  printf("          r_ticks: %d\n", rootblock->r_ticks);
  printf("         diskname: %s\n", rootblock->diskname);
  printf("          unused1: %04x %04x\n", rootblock->unused1[0], rootblock->unused1[1]);
  printf("           v_days: %d\n", rootblock->v_days);
  printf("            v_min: %d\n", rootblock->v_min);
  printf("          v_ticks: %d\n", rootblock->v_ticks);
  printf("           c_days: %d\n", rootblock->c_days);
  printf("            c_min: %d\n", rootblock->c_min);
  printf("          c_ticks: %d\n", rootblock->c_ticks);
  printf("        next_hash: %d\n", rootblock->next_hash);
  printf("       parent_dir: %d\n", rootblock->parent_dir);
  printf("        extension: %d\n", rootblock->extension);
  printf("         sec_type: %d\n\n", rootblock->sec_type);

  printf(" partition_offset: 0x%08x (%d)\n", rootblock->partition_offset, rootblock->partition_offset);
  printf("      disk_offset: 0x%08x (%d)\n", rootblock->disk_offset, rootblock->disk_offset);
}


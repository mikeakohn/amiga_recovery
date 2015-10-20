/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "fileheader.h"
#include "fileio.h"

void read_fileheader(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_fileheader *fileheader, uint32_t block)
{
  int namelen;
  int t;

  fseek(in, partition->start + (block * bootblock->blksz), SEEK_SET);

  fileheader->type = read_int(in);
  fileheader->header_key = read_int(in);
  fileheader->high_seq = read_int(in);
  fileheader->data_size = read_int(in);
  fileheader->first_data = read_int(in);
  fileheader->checksum = read_int(in);
  //uint32_t datablocks[512/4-56];  //FIXME - wrong size?
  for (t = 0; t < (512 / 4 - 56); t++)
  {
    fileheader->datablocks[t] = read_int(in);
  }
  fileheader->unused1 = read_int(in);
  fileheader->uid = read_short(in);
  fileheader->gid = read_short(in);
  fileheader->protect = read_int(in);
  fileheader->byte_size = read_int(in);
  //unsigned char comment[80]; // first char is len
  namelen = getc(in);
  for (t = 0; t < 79; t++)
  {
    fileheader->comment[t] = getc(in);
  }
  fileheader->comment[namelen] = 0;
  //unsigned char unused2[12];
  fseek(in, 12, SEEK_CUR);
  fileheader->days = read_int(in);
  fileheader->mins = read_int(in);
  fileheader->ticks = read_int(in);
  //unsigned char filename[32]; // first char is len, last char is unused
  namelen = getc(in);
  for (t = 0; t < 31; t++)
  {
    fileheader->filename[t] = getc(in);
  }
  fileheader->filename[namelen] = 0;
  fileheader->unused3 = read_int(in);
  fileheader->read_entry = read_int(in);
  fileheader->next_link = read_int(in);
  //uint32_t unused4[5];
  fseek(in, 5 * 4, SEEK_CUR);
  fileheader->hash_chain = read_int(in);
  fileheader->parent = read_int(in);
  fileheader->extension = read_int(in);
  fileheader->sec_type = read_int(in);
}

void print_fileheader(struct _amiga_fileheader *fileheader)
{
  int t;

  printf("================== File Header ===================\n");
  printf("             type: %d\n", fileheader->type);
  printf("       header_key: %d\n", fileheader->header_key);
  printf("         high_seq: %d\n", fileheader->high_seq);
  printf("        data_size: %d\n", fileheader->data_size);
  printf("       first_data: %d\n", fileheader->first_data);
  printf("         checksum: %d\n", fileheader->checksum);
  printf("       datablocks:");
  for (t = 0; t < (512 / 4 - 56); t++)
  {
    if (t != 0) printf(" ");
    printf("%d", fileheader->datablocks[t]);
  }
  printf("\n");
  printf("              uid: %d\n", fileheader->uid);
  printf("              gid: %d\n", fileheader->gid);
  printf("          protect: %d\n", fileheader->protect);
  printf("        byte_size: %d\n", fileheader->byte_size);
  printf("          comment: %s\n", fileheader->comment);
  printf("             days: %d\n", fileheader->days);
  printf("             mins: %d\n", fileheader->mins);
  printf("            ticks: %d\n", fileheader->ticks);
  printf("         filename: %s\n", fileheader->filename);
  printf("       read_entry: %d\n", fileheader->read_entry);
  printf("        next_link: %d\n", fileheader->next_link);
  printf("       hash_chain: %d\n", fileheader->hash_chain);
  printf("           parent: %d\n", fileheader->parent);
  printf("        extension: %d\n", fileheader->extension);
  printf("         sec_type: %d\n", fileheader->sec_type);
}


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

#include "affs.h"
#include "fileio.h"

#define ST_FILE -3
#define ST_ROOT 1 
#define ST_USERDIR 2 
#define ST_SOFTLINK 3 
#define ST_LINKDIR 4 

/* hash_name function copied from http://lclevy.free.fr/adflib/adf_info.html */
uint32_t hash_name(unsigned char *name)
{
  uint32_t hash, l; /* sizeof(int)>=2 */
  int i;

  l = hash = strlen((char *)name);

  for(i = 0; i < l; i++)
  {
    hash = hash * 13;
    hash = hash + toupper(name[i]); /* not case sensitive */
    hash = hash & 0x7ff;
  }

  /* 0 < hash < 71 in the case of 512 byte blocks */
  hash = hash % ((BSIZE / 4) - 56);

  return(hash);
}

void read_file_ext(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_file_ext *file_ext, uint32_t block)
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

void read_datablock(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_datablock *datablock, uint32_t block)
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

void print_hash_info(FILE *in, struct _amiga_rootblock *rootblock, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, uint32_t block)
{
  struct _amiga_directory directory;
  struct _amiga_fileheader fileheader;
  int sec_type;

  sec_type = get_sec_type(in, bootblock, partition,block);

  if (sec_type == ST_USERDIR)
  {
    read_directory(in, bootblock, partition, &directory, block);
    //print_directory(&directory);
    printf("    (Dir) %s\n", directory.dirname);

    if (directory.hash_chain != 0)
    {
      print_hash_info(in, rootblock, bootblock, partition, directory.hash_chain);
    }
  }
    else
  if (sec_type == ST_FILE)
  {
    read_fileheader(in, bootblock, partition, &fileheader, block);
    //print_fileheader(&fileheader);
    printf("%9d %s\n", fileheader.byte_size, fileheader.filename);

    if (fileheader.hash_chain != 0)
    {
      print_hash_info(in, rootblock, bootblock, partition, fileheader.hash_chain);
    }
  }
    else
  {
    printf("Unknown sec_type %d\n", sec_type);
  }
}

static void print_file_at_block(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_fileheader *fileheader, FILE *out)
{
  //struct _amiga_datablock datablock;
  struct _amiga_file_ext file_ext;
  uint32_t *datablocks;
  uint32_t bytes_left;
  unsigned char buffer[bootblock->blksz];
  uint32_t next_block;
  int curr;
  int len;
  int t;

  datablocks = fileheader->datablocks;
  bytes_left = fileheader->byte_size;
  next_block = fileheader->extension;

  // print_fileheader(fileheader);

  while(bytes_left > 0)
  {
    for (curr = 71; curr >= 0; curr--)
    {
      if (datablocks[curr] == 0) break;

      fseek(in, partition->start + (datablocks[curr] * bootblock->blksz), SEEK_SET); 
      if (bytes_left>bootblock->blksz)
      {
        len=fread(buffer, 1, bootblock->blksz, in);
        bytes_left -= bootblock->blksz;
      }
        else
      {
        len=fread(buffer, 1, bytes_left, in);
        bytes_left -= bytes_left;
      }

      if (out == NULL)
      {
        for (t = 0; t < len; t++) { putchar(buffer[t]); }
      }
        else
      {
        fwrite(buffer, 1, len, out);
      }
    }

    if (next_block==0) break;

    read_file_ext(in, bootblock, partition, &file_ext, next_block);
    if (file_ext.type != 16)
    {
      printf("Error: File extension wasn't read right?  Bug?\n");
      break;
    }
    //print_file_ext(&file_ext);
    next_block = file_ext.extension;
    datablocks = file_ext.datablocks;
  }

  if (out == NULL) { printf("\n"); }
}

void print_file(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd, char *filename, FILE *out)
{
  struct _amiga_directory directory;
  struct _amiga_fileheader fileheader;
  uint32_t block;
  int sec_type;

  block = pwd->dir_hash[hash_name((unsigned char*)filename)];

  while(block != 0)
  {
    sec_type = get_sec_type(in, bootblock, &pwd->partition, block);

    if (sec_type == ST_USERDIR)
    {
      read_directory(in, bootblock, &pwd->partition, &directory, block);

      if (directory.hash_chain == 0) break;

      block = directory.hash_chain;
    }
      else
    if (sec_type == ST_FILE)
    {
      read_fileheader(in, bootblock, &pwd->partition, &fileheader, block);

      if (strcmp((char *)fileheader.filename, filename) == 0)
      {
        print_file_at_block(in, bootblock, &pwd->partition, &fileheader, out);
        return;
      }

      if (fileheader.hash_chain == 0) break;

      block = fileheader.hash_chain;
    }
      else
    {
      printf("Unknown sec_type %d\n", sec_type);
      break;
    }
  }

  printf("Error: file '%s' not found\n", filename);
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

int get_sec_type(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, uint32_t block)
{
  fseek(in, partition->start + (block * bootblock->blksz + (bootblock->blksz - 4)), SEEK_SET);

  return read_int(in);
}


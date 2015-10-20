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
#include "file_ext.h"
#include "fileio.h"

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

int get_sec_type(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, uint32_t block)
{
  fseek(in, partition->start + (block * bootblock->blksz + (bootblock->blksz - 4)), SEEK_SET);

  return read_int(in);
}


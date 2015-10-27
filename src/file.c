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
      if (datablocks[curr] == 0) { break; }

      fseek(in, partition->start + (datablocks[curr] * bootblock->blksz), SEEK_SET); 
      if (bytes_left > bootblock->blksz)
      {
        len=fread(buffer, 1, bootblock->blksz, in);
        bytes_left -= bootblock->blksz;
      }
        else
      {
        len = fread(buffer, 1, bytes_left, in);
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

    if (next_block == 0) { break; }

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

void copy_file(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd, char *filename, FILE *out)
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


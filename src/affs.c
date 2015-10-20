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

int get_sec_type(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, uint32_t block)
{
  fseek(in, partition->start + (block * bootblock->blksz + (bootblock->blksz - 4)), SEEK_SET);

  return read_int(in);
}


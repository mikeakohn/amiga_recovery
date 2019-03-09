/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2019 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

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

int get_sec_type(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, uint32_t block)
{
  fseek(in, partition->start + (block * bootblock->blksz + (bootblock->blksz - 4)), SEEK_SET);

  return read_int(in);
}


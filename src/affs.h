/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2019 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

*/

#ifndef AFFS_H
#define AFFS_H

#include "bootblock.h"
#include "directory.h"
#include "file.h"
#include "fileheader.h"
#include "partition.h"
#include "rootblock.h"

#define ST_FILE -3
#define ST_ROOT 1 
#define ST_USERDIR 2 
#define ST_SOFTLINK 3 
#define ST_LINKDIR 4 

uint32_t hash_name(unsigned char *name);

int get_sec_type(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, uint32_t block);

#endif


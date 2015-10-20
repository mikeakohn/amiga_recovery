/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#ifndef AFFS_H
#define AFFS_H

#include "bootblock.h"
#include "directory.h"
#include "fileheader.h"
#include "partition.h"
#include "rootblock.h"

void print_file(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd, char *filename, FILE *out);

uint32_t hash_name(unsigned char *name);
void print_hash_info(FILE *in, struct _amiga_rootblock *rootblock, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, uint32_t block);

int get_sec_type(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, uint32_t block);

#endif


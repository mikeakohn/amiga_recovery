/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "bootblock.h"
#include "fileheader.h"
#include "partition.h"
#include "rootblock.h"

struct _amiga_directory
{
  unsigned int type;
  unsigned int header_key;
  unsigned int unused1[3];
  unsigned int checksum;
  unsigned int hash_table[BSIZE/4-56];
  unsigned int unused2[2];
  unsigned short int uid;
  unsigned short int gid;
  unsigned int protect;
  unsigned int unused3;
  unsigned char comment[80]; // first char is len
  unsigned char unused4[12];
  unsigned int days;
  unsigned int mins;
  unsigned int ticks;
  unsigned char dirname[32]; // first char is len, last char is unused
  unsigned int unused5[2];
  unsigned int next_link;
  unsigned int unused6[5];
  unsigned int hash_chain;
  unsigned int parent;
  unsigned int extension;
  unsigned int sec_type;
};

// I didn't want to put this here, but owell
struct _pwd
{
  struct _amiga_rootblock rootblock;
  struct _amiga_partition partition;
  int partition_num;
  char cwd[4096];  // FIXME - check bounds
  unsigned int dir_hash[BSIZE/4-56];
  unsigned int hash_table_size;
  unsigned int parent_dir;
};

void read_directory(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_directory *directory, unsigned int block);
void list_directory(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd);
void print_directory(struct _amiga_directory *directory);
int ch_dir(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd, char *dirname);

#endif


/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#ifndef AFFS_H
#define AFFS_H

#include "bootblock.h"
#include "partition.h"

struct _amiga_rootblock
{
  unsigned int type;
  unsigned int header_key;
  unsigned int high_seq;
  unsigned int hash_table_size;
  unsigned int first_size;
  unsigned int checksum;
  unsigned int hash_table[BSIZE/4-56];
  unsigned int bm_flag;
  unsigned int bm_pages[25];
  unsigned int bm_ext;
  unsigned int r_days;
  unsigned int r_mins;
  unsigned int r_ticks;
  unsigned char diskname[32];  // name_len + 30 possible + 1 unused
  unsigned int unused1[2];
  unsigned int v_days;
  unsigned int v_min;
  unsigned int v_ticks;
  unsigned int c_days;
  unsigned int c_min;
  unsigned int c_ticks;
  unsigned int next_hash;
  unsigned int parent_dir;
  unsigned int extension;
  unsigned int sec_type;

  // DEBUG STUFF
  unsigned int partition_offset;
  unsigned int disk_offset;
};

struct _amiga_fileheader
{
  unsigned int type;
  unsigned int header_key;
  unsigned int high_seq;
  unsigned int data_size;
  unsigned int first_data;
  unsigned int checksum;
  unsigned int datablocks[BSIZE/4-56];  //FIXME - wrong size?
  unsigned int unused1;
  unsigned short int uid;
  unsigned short int gid;
  unsigned int protect;
  unsigned int byte_size;
  unsigned char comment[80]; // first char is len
  unsigned char unused2[12];
  unsigned int days;
  unsigned int mins;
  unsigned int ticks;
  unsigned char filename[32]; // first char is len, last char is unused
  unsigned int unused3;
  unsigned int read_entry;
  unsigned int next_link;
  unsigned int unused4[5];
  unsigned int hash_chain;
  unsigned int parent;
  unsigned int extension;
  unsigned int sec_type;
};

struct _amiga_file_ext
{
  unsigned int type;
  unsigned int header_key;
  unsigned int high_seq;
  unsigned int unused1;
  unsigned int unused2;
  unsigned int checksum;
  unsigned int datablocks[BSIZE/4-56];
  unsigned int info[46];
  unsigned int unused3;
  unsigned int parent;
  unsigned int extension;
  unsigned int sec_type;
};

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

struct _amiga_datablock
{
  unsigned int type;
  unsigned int header_key;
  unsigned int seq_num;
  unsigned int data_size;
  unsigned int next_data;
  unsigned int checksum;
  unsigned char data[BSIZE-24];
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

void read_rootblock(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_rootblock *rootblock);
void read_fileheader(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_fileheader *fileheader, unsigned int block);
void read_file_ext(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_file_ext *file_ext, unsigned int block);
void read_directory(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_directory *directory, unsigned int block);
void read_datablock(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_datablock *datablock, unsigned int block);

void list_directory(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd);
void print_file(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd, char *filename, FILE *out);

void print_rootblock(struct _amiga_rootblock *rootblock);
void print_fileheader(struct _amiga_fileheader *fileheader);
void print_file_ext(struct _amiga_file_ext *file_ext);
void print_directory(struct _amiga_directory *directory);
void print_datablock(struct _amiga_datablock *datablock);

int dump_partition(FILE *in, struct _amiga_bootblock *bootblock, int num, const char *name);
int read_partition_num(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, int num);
int get_partition_num(FILE *in, struct _amiga_bootblock *bootblock, char *name);
int ch_dir(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd, char *dirname);
int get_sec_type(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, unsigned int block);

#endif


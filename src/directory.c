/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2021 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "affs.h"
#include "directory.h"
#include "fileio.h"

void print_hash_info(
  FILE *in,
  struct _amiga_rootblock *rootblock,
  struct _amiga_bootblock *bootblock,
  struct _amiga_partition *partition,
  uint32_t block)
{
  struct _amiga_directory directory;
  struct _amiga_fileheader fileheader;
  int sec_type;

  sec_type = get_sec_type(in, bootblock, partition, block);

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

void read_directory(
  FILE * in,
  struct _amiga_bootblock *bootblock,
  struct _amiga_partition *partition,
  struct _amiga_directory *directory,
  uint32_t block)
{
  int namelen;
  int t;

  fseek(in, partition->start + (block * bootblock->blksz), SEEK_SET);

  directory->type = read_int(in);
  directory->header_key = read_int(in);
  //uint32_t unused1[3];
  fseek(in, 3 * 4, SEEK_CUR);
  directory->checksum = read_int(in);
  //uint32_t hash_table[512/4-56];
  for (t = 0; t < (512 / 4 - 56); t++)
  {
    directory->hash_table[t] = read_int(in);
  }
  //uint32_t unused2[2];
  fseek(in, 2 * 4, SEEK_CUR);
  directory->uid = read_short(in);
  directory->gid = read_short(in);
  directory->protect = read_int(in);
  // FIXME - WTF?
  //directory->unused3 = read_int(in);
  //unsigned char comment[80]; // first char is len
  namelen = getc(in);
  for (t = 0; t < 79; t++)
  {
    directory->comment[t] = getc(in);
  }
  directory->comment[namelen]  =0;
  //unsigned char unused4[12];
  fseek(in, 12, SEEK_CUR);
  directory->days= read_int(in);
  directory->mins = read_int(in);
  directory->ticks = read_int(in);
  //unsigned char dirname[32]; // first char is len, last char is unused
  namelen = getc(in);
  for (t = 0; t < 31; t++)
  {
    directory->dirname[t] = getc(in);
  }
  directory->dirname[namelen] = 0;
  //uint32_t unused5[2];
  fseek(in, 2 * 4, SEEK_CUR);
  directory->next_link = read_int(in);
  //uint32_t unused6[5];
  fseek(in, 5 * 4, SEEK_CUR);
  directory->hash_chain = read_int(in);
  directory->parent = read_int(in);
  directory->extension = read_int(in);
  directory->sec_type = read_int(in);
}

void list_directory(
  FILE *in,
  struct _amiga_bootblock *bootblock,
  struct _pwd *pwd)
{
  int t;

  for (t = 0; t < 72; t++)
  {
    if (pwd->dir_hash[t] != 0)
    {
      print_hash_info(in, &pwd->rootblock, bootblock, &pwd->partition, pwd->dir_hash[t]);
    }
  }
}

void print_directory(struct _amiga_directory *directory)
{
  int t;

  printf("================== Directory ===================\n");
  printf("             type: %d\n", directory->type);
  printf("       header_key: %d\n", directory->header_key);
  printf("         checksum: %d\n", directory->checksum);
  printf("       hash_table:");
  for (t = 0; t < (512 / 4 - 56); t++)
  {
    if (t != 0) { printf(" "); }
    printf("%d", directory->hash_table[t]);
  }
  printf("\n");
  printf("              uid: %d\n", directory->uid);
  printf("              gid: %d\n", directory->gid);
  printf("          protect: %d\n", directory->protect);
  printf("          comment: %s\n", directory->comment);
  printf("             days: %d\n", directory->days);
  printf("             mins: %d\n", directory->mins);
  printf("            ticks: %d\n", directory->ticks);
  printf("          dirname: %s\n", directory->dirname);
  printf("        next_link: %d\n", directory->next_link);
  printf("       hash_chain: %d\n", directory->hash_chain);
  printf("           parent: %d\n", directory->parent);
  printf("        extension: %d\n", directory->extension);
  printf("         sec_type: %d\n", directory->sec_type);
}

int ch_dir(
  FILE *in,
  struct _amiga_bootblock *bootblock,
  struct _pwd *pwd,
  char *dirname)
{
  struct _amiga_directory directory;
  struct _amiga_fileheader fileheader;

  if (strcmp(dirname, "..")==0)
  {
    if (pwd->parent_dir == 0) { return -1; }

    if (pwd->parent_dir == pwd->rootblock.header_key)
    {
      memcpy(pwd->dir_hash, pwd->rootblock.hash_table, (BSIZE / 4 - 56) * 4);
      pwd->cwd[0] = 0;
      pwd->parent_dir = 0;
    }
      else
    {
      read_directory(in, bootblock, &pwd->partition, &directory, pwd->parent_dir);
      memcpy(pwd->dir_hash, directory.hash_table, (BSIZE / 4 - 56) * 4);
      int l = strlen(pwd->cwd) - 2;
      while (l >= 0)
      {
        if (pwd->cwd[l] == '/') { break; }
        pwd->cwd[l--] = 0;
      }
    }
  }
    else
  {
    int block = pwd->dir_hash[hash_name((unsigned char*)dirname)];

    while (block != 0)
    {
      uint32_t sec_type = get_sec_type(in,bootblock,&pwd->partition,block);

      if (sec_type == ST_USERDIR)
      {
        read_directory(in,bootblock,&pwd->partition,&directory,block);

        if (strcmp((char *)directory.dirname, dirname)==0)
        {
          memcpy(pwd->dir_hash, directory.hash_table, (BSIZE / 4 - 56) * 4);
          pwd->parent_dir = directory.parent;
          strcat(pwd->cwd, dirname);
          strcat(pwd->cwd, "/");
          return 0;
        }

        if (directory.hash_chain == 0) { break; }
        block = directory.hash_chain;
      }
        else
      if (sec_type == ST_FILE)
      {
        read_fileheader(in, bootblock, &pwd->partition, &fileheader, block);

        if (fileheader.hash_chain == 0) { break; }

        block = fileheader.hash_chain;
      }
        else
      {
        printf("Unknown sec_type %d\n", sec_type);
        break;
      }
    }

    printf("Error: No such directory\n");
    return -1;
  }

  return 0;
}


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
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "affs.h"
#include "copy_all.h"
#include "directory.h"
#include "fileio.h"

static int copy_from_hash(FILE *in, struct _amiga_rootblock *rootblock, struct _amiga_bootblock *bootblock, struct _pwd *pwd, uint32_t block, const char *path)
{
  struct _amiga_directory directory;
  struct _amiga_fileheader fileheader;
  struct _amiga_partition *partition = &pwd->partition;
  char name[8192];
  int sec_type;

  sec_type = get_sec_type(in, bootblock, partition, block);

  if (sec_type == ST_USERDIR)
  {
    struct _pwd new_pwd;

    read_directory(in, bootblock, partition, &directory, block);

    printf("    (Dir) %s\n", directory.dirname);

    sprintf(name, "%s/%s", path, directory.dirname);
    printf(" -- mkdir %s\n", name);

    if (mkdir(name, 0777) == 0)
    {
      memcpy(&new_pwd, pwd, sizeof(struct _pwd));
      if (ch_dir(in, bootblock, &new_pwd, (char *)directory.dirname) == 0)
      {
        if (copy_all(in, bootblock, &new_pwd, name) != 0)
        {
          return -1;
        }
      }
    }
      else
    { 
      if (errno != EEXIST)
      { 
        printf("Could not create directory %s\n", path);
        return -1;
      }
    }

    if (directory.hash_chain != 0)
    {
      copy_from_hash(in, rootblock, bootblock, pwd, directory.hash_chain, path);
    }
  }
    else
  if (sec_type == ST_FILE)
  {
    read_fileheader(in, bootblock, partition, &fileheader, block);

    printf("%9d %s\n", fileheader.byte_size, fileheader.filename);

    sprintf(name, "%s/%s", path, fileheader.filename);
    printf(" -- Copy to %s\n", name);

    FILE *out = fopen(name, "wb");

    if (out == NULL)
    {
      printf("Could not open %s for writing.  Aborting.\n", name);
      return -1;
    }

    copy_file(in, bootblock, pwd, (char *)fileheader.filename, out);

    fclose(out);

    if (fileheader.hash_chain != 0)
    {
      copy_from_hash(in, rootblock, bootblock, pwd, fileheader.hash_chain, path);
    }
  }
    else
  {
    printf("Unknown sec_type %d\n", sec_type);
  }

  return 0;
}

int copy_all(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd, char *path)
{
  int t;

  for (t = 0; t < 72; t++)
  {
    if (pwd->dir_hash[t] != 0)
    {
      if (copy_from_hash(in, &pwd->rootblock, bootblock, pwd, pwd->dir_hash[t], path) != 0)
      {
        return -1;
      }
    }
  }

  return 0;
}



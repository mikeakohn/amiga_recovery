/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "affs.h"

#define CHECKDIR() \
      if (pwd.partition.magic[0] == 0) \
      { \
        printf("No current dir... cd somewhere first.\n"); \
        continue; \
      } \

static void print_help()
{
  printf("Commands:\n");
  printf("  print { bootblock, partition, rootblock } [ print debug info ]\n");
  printf("  show partitions                [ show partition list ]\n");
  printf("  dump partition <filename>      [ dump pwd partition in a new file ]\n");
  printf("  cd <dir>                       [ change Amiga drive/dir ]\n");
  printf("  lcd <dir>                      [ change local dir ]\n");
  printf("  dir or ls                      [ current directory listing ]\n");
  printf("  type or cat <filename>         [ write a text file to screen ]\n");
  printf("  get <filename>                 [ copy file to local disk ]\n");
  printf("  pwd                            [ show current directory ]\n");
  printf("  help or ?\n");
  printf("  exit or quit\n");
}

static void print_prompt(struct _pwd *pwd)
{
  if (pwd->partition.magic[0] != 0)
  {
    printf("%s", pwd->partition.name);
  }

  printf(":%s> ", pwd->cwd);
}

static void print_pwd(struct _pwd *pwd)
{
  if (pwd->partition.magic[0] != 0)
  {
    printf("%s", pwd->partition.name);
  }

  printf(":%s\n", pwd->cwd);
}

static void change_dir(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd, char *path)
{
  int t;

  if (path[0] == 0)
  {
    printf("Illegal directory\n");
    return;
  }

  t = 0;
  while(path[t] != 0)
  {
    if (path[t] == ':')
    {
      path[t] = 0;
      int p = get_partition_num(in, bootblock, path);
      if (p == -1)
      {
        printf("No such drive name %s\n", path);
        return;
      }

      read_partition_num(in, bootblock, &pwd->partition, p);

      if (pwd->partition_num != t)
      {
        read_rootblock(in, bootblock, &pwd->partition, &pwd->rootblock);
        memcpy(pwd->dir_hash, pwd->rootblock.hash_table, (BSIZE / 4 - 56) * 4);
      }
      pwd->partition_num = p;
      path = path + t + 1;

      break;
    }

    t++;
  }

  if (path[0] == 0) { pwd->cwd[0] = 0; return; }

  char *s = path;

  while(*path != 0)
  {
    while (*s != 0 && *s != '/') s++;

    if (*s == 0 && s != path)
    {
      ch_dir(in, bootblock, pwd, path);
      break;
    }
      else
    if (*s == '/')
    {
      *s = 0;
      if (ch_dir(in, bootblock, pwd, path) != 0) break;
      s++;
      path = s;
    }
  }
}

int main(int argc, char *argv[])
{
  FILE *in;
  struct _amiga_bootblock bootblock;
  struct _pwd pwd;
  char command[1024];
  int t;

  printf("Amiga Recovery - 2015-October-20\n");
  printf("Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)\n");
  printf("http://www.mikekohn.net/\n\n");

  if (argc < 2)
  {
    printf("Usage: %s <diskfile>\n",argv[0]);
    exit(0);
  }

  in = fopen(argv[1], "rb");

  if (in == NULL)
  {
    printf("Could not open file %s for reading.\n", argv[1]);
    exit(1);
  }

  memset(&pwd, 0, sizeof(pwd));

  t = read_bootblock(in, &bootblock);
  if (t >= 0)
  {
    printf("Found bootblock @ block %d\n\n", t);
  }
  else
  {
    printf("Error: Could not read bootblock\n\n");
  }

  printf("Type help for a list of commands.\n");
  command[1023] = 0;

  while(1)
  {
    print_prompt(&pwd);
    char *fu = fgets(command, 1023, stdin);
    if (fu == NULL) { printf("wtf gcc\n"); }

    t = 0;
    while(command[t] != 0)
    {
      if (command[t] == '\n' || command[t] == '\r')
      { command[t] = 0; break; }
      t++;
    }

    if (command[0] == 0) { continue; }

    if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0)
    {
      print_help();
    }
      else
    if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0)
    {
      break;
    }
      else
    if (strcmp(command, "show partitions") == 0 ||
        strcmp(command, "show") == 0)
    {
      show_partitions(in, &bootblock);
    }
      else
    if (strcmp(command, "print bootblock") == 0 ||
        strcmp(command, "bootblock") == 0)
    {
      print_bootblock(&bootblock);
    }
      else
    if (strcmp(command, "print partition") == 0 ||
        strcmp(command, "partition") == 0)
    {
      CHECKDIR();
      print_partition(&pwd.partition);
    }
      else
    if (strcmp(command, "print rootblock") == 0 ||
        strcmp(command, "rootblock") == 0)
    {
      CHECKDIR();
      struct _amiga_rootblock rootblock;
      read_rootblock(in, &bootblock, &pwd.partition, &rootblock);
      print_rootblock(&rootblock);
    }
      else
    if (strcmp(command, "pwd") == 0)
    {
      print_pwd(&pwd);
    }
      else
    if (strncmp(command, "cd ", 3) == 0)
    {
      change_dir(in, &bootblock, &pwd, command + 3);
    }
      else
    if (strncmp(command, "type ", 5) == 0)
    {
      CHECKDIR();
      char *filename = command + 5;
      if (filename[0] == 0)
      {
        printf("Error: No filename specified.\n"); 
      }
        else
      {
        print_file(in, &bootblock, &pwd, filename, NULL);
      }
    }
      else
    if (strncmp(command, "cat ", 4) == 0)
    {
      CHECKDIR();
      char *filename = command + 4;
      if (filename[0] == 0)
      {
        printf("Error: No filename specified.\n"); 
      }
        else
      {
        print_file(in, &bootblock, &pwd, filename, NULL);
      }
    }
      else
    if (strncmp(command, "get ", 4) == 0)
    {
      CHECKDIR();
      char *filename = command + 4;
      if (filename[0] == 0)
      {
        printf("Error: No filename specified.\n"); 
      }
        else
      {
        FILE *out = fopen(filename, "rb");
        if (out != NULL)
        {
          printf("Error: File exists on local disk.  Will not overwrite.\n");
          fclose(out);
        }
          else
        {
          out = fopen(filename, "wb");
          print_file(in, &bootblock, &pwd, filename, out);
          fclose(out);
          printf("Saved!\n");
        }
      }
    }
      else
    if (strncmp(command, "lcd ", 4) == 0)
    {
      char *dir = command + 4;
      if (dir[0] == 0)
      {
        printf("Error: No filename specified.\n"); 
      }
        else
      {
        if (chdir(dir) == 0)
        {
          printf("Local directory now: %s\n", dir);
        }
          else
        {
          printf("Error: No such directory?\n");
        }
      }
    }
      else
    if (strcmp(command, "dir") == 0 || strcmp(command, "ls") == 0)
    {
      CHECKDIR();
      list_directory(in, &bootblock, &pwd);
    }
      else
    if (strncmp(command, "dump partition ", sizeof("dump partition ") - 1) == 0)
    {
      char *filename = command + sizeof("dump partition ") - 1;

      if (filename[0] != 0)
      {
        FILE *fp = fopen(filename, "rb");

        if (fp != NULL)
        {
          printf("Error: File %s exists.  Will not overwrite.\n", filename);
          fclose(fp);
        }
          else
        {
          printf("Dumping partition to filename '%s'\n", filename);
          dump_partition(in, &bootblock, pwd.partition_num, filename);
        }
      }
        else
      {
        printf("Illegal filename.\n");
      }
    }
      else
    {
      printf("Unknown command '%s'.  Type help for a list of commands\n", command);
    }
  }

/*
  print_bootblock(&bootblock);
  print_partition_list(in, &bootblock);

  read_partition_num(in, &bootblock, &partition, 3);
  print_partition(&partition, 3);
  list_directory(in, &bootblock, &partition);
*/

  fclose(in);

  return 0;
}



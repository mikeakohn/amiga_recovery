/*
  
Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2021 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

parse_hunk - Read in an Amiga executable and dump out all sections of
             the hunk file.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define HUNK_UNIT         0x3e7
#define HUNK_NAME         0x3e8
#define HUNK_CODE         0x3e9
#define HUNK_DATA         0x3ea
#define HUNK_BSS          0x3eb
#define HUNK_RELOC32      0x3ec
#define HUNK_RELOC16      0x3ed
#define HUNK_RELOC8       0x3ee
#define HUNK_EXT          0x3ef
#define HUNK_SYMBOL       0x3f0
#define HUNK_DEBUG        0x3f1
#define HUNK_END          0x3f2
#define HUNK_HEADER       0x3f3
#define HUNK_OVERLAY      0x3f5
#define HUNK_BREAK        0x3f6
#define HUNK_DREL32       0x3f7
#define HUNK_DREL16       0x3f8
#define HUNK_DREL8        0x3f9
#define HUNK_LIB          0x3fa
#define HUNK_INDEX        0x3fb
#define HUNK_RELOC32SHORT 0x3fc
#define HUNK_RELRELOC32   0x3fd
#define HUNK_ABSRELOC16   0x3fe
#define HUNK_PPC_CODE     0x4e9
#define HUNK_RELRELOC26   0x4ec

static uint32_t read_int32(FILE *in)
{
  return (getc(in) << 24) |
         (getc(in) << 16) |
         (getc(in) << 8) |
          getc(in);
}

const char *get_hunk_name(uint32_t hunk_type)
{
  switch (hunk_type)
  {
    case HUNK_UNIT:
      return "HUNK_UNIT";
    case HUNK_NAME:
      return "HUNK_NAME";
    case HUNK_CODE:
      return "HUNK_CODE";
    case HUNK_DATA:
      return "HUNK_DATA";
    case HUNK_BSS:
      return "HUNK_BSS";
    case HUNK_RELOC32:
      return "HUNK_RELOC32";
    case HUNK_RELOC16:
      return "HUNK_RELOC16";
    case HUNK_RELOC8:
      return "HUNK_RELOC8";
    case HUNK_EXT:
      return "HUNK_EXT";
    case HUNK_SYMBOL:
      return "HUNK_SYMBOL";
    case HUNK_DEBUG:
      return "HUNK_DEBUG";
    case HUNK_END:
      return "HUNK_END";
    case HUNK_HEADER:
      return "HUNK_HEADER";
    case HUNK_OVERLAY:
      return "HUNK_OVERLAY";
    case HUNK_BREAK:
      return "HUNK_BREAK";
    case HUNK_DREL32:
      return "HUNK_DREL32";
    case HUNK_DREL16:
      return "HUNK_DREL16";
    case HUNK_DREL8:
      return "HUNK_DREL8";
    case HUNK_LIB:
      return "HUNK_LIB";
    case HUNK_INDEX:
      return "HUNK_INDEX";
    case HUNK_RELOC32SHORT:
      return "HUNK_RELOC32SHORT";
    case HUNK_RELRELOC32:
      return "HUNK_RELRELOC32";
    case HUNK_ABSRELOC16:
      return "HUNK_ABSRELOC16";
    case HUNK_PPC_CODE:
      return "HUNK_PPC_CODE";
    case HUNK_RELRELOC26:
      return "HUNK_RELRELOC26";
    default:
      break;
  }

  return "???";
}

int parse_hunk_code(FILE *in)
{
  int n;
  uint32_t length = read_int32(in);

  for (n = 0; n < length; n++)
  {
    read_int32(in);
  }

  printf("Length In Bytes: %d\n", length * 4);

  return 0;
}

int parse_hunk_reloc32(FILE *in)
{
  int n;

  while (1)
  {
    uint32_t count = read_int32(in);
    if (count == 0) { return 0; }

    uint32_t hunk_id = read_int32(in);

    printf("        Hunk ID: %d\n", hunk_id);

    for (n = 0; n < count; n++)
    {
      uint32_t hunk_offset = read_int32(in);

      printf("        0x%08x\n", hunk_offset);
    }
  }

  return 0;
}

int parse_hunk_symbol(FILE *in)
{
  int ch, n, ptr;
  char name[1024];

  while (1)
  {
    uint32_t name_length = read_int32(in);
    if (name_length == 0) { break; }

    ptr = 0;

    for (n = 0; n < name_length * 4; n++)
    {
      ch = getc(in);

      if (ch == 0) { continue; }
      if (ch == EOF) { break; }
      if (ptr >= sizeof(name) - 1) { continue; }

      name[ptr++] = ch;
    }

    name[ptr] = 0;

    uint32_t offset = read_int32(in);

    printf("        0x%08x %s\n", offset, name);
  }

  return 0;
}

int parse_hunk_header(FILE *in)
{
  int ch, n;
  uint32_t name_length = read_int32(in);

  printf("           Name: ");

  for (n = 0; n < name_length * 4; n++)
  {
    ch = getc(in);

    if (ch == 0) { continue; }
    if (ch == EOF) { break; }

    printf("%c", ch);
  }

  printf("\n");

  uint32_t table_length = read_int32(in);
  uint32_t first_hunk = read_int32(in);
  uint32_t last_hunk = read_int32(in);

  printf("   Table Length: %d\n", table_length);
  printf("     First Hunk: %d\n", first_hunk);
  printf("      Last Hunk: %d\n", last_hunk);

  for (n = 0; n < table_length; n++)
  {
    uint32_t size = read_int32(in);

    printf("         len=%d\n", size);
  }

  return 0;
}

int main(int argc, char *argv[])
{
  FILE *in;

  if (argc != 2)
  {
    printf("Usage: parse_hunk <filename>\n");
    return 0;
  }

  in = fopen(argv[1], "rb");

  if (in == NULL)
  {
    printf("Cannot open file %s\n", argv[1]);
    return -1;
  }

  uint32_t magic_cookie = read_int32(in);

  if (magic_cookie != HUNK_HEADER)
  {
    printf("Not an Amiga hunk file\n");
    fclose(in);
    return -1;
  }

  fseek(in, 0, SEEK_SET);

  int running = 1;

  while (running == 1)
  {
    long offset = ftell(in);
    uint32_t hunk_type = read_int32(in);
    uint32_t bits = hunk_type >> 30;

    hunk_type &= 0x3fffffff;

    const char *where = "";

    if (hunk_type != HUNK_HEADER)
    {
      switch (bits)
      {
        case 0:
          where = " (load anywhere)";
          break;
        case 1:
          where = " (load in fast RAM)";
          break;
        case 2:
          where = " (load in chip RAM)";
          break;
        case 3:
          where = " (load .. specific flags)";
          read_int32(in);
          break;
      }
    }

    printf("-- %s%s offset=%ld --\n",
      get_hunk_name(hunk_type), where, offset);

    switch (hunk_type)
    {
      case HUNK_UNIT:
      case HUNK_NAME:
        running = 0;
        break;
      case HUNK_CODE:
        parse_hunk_code(in);
        break;
      case HUNK_DATA:
      case HUNK_BSS:
        running = 0;
        break;
      case HUNK_RELOC32:
        parse_hunk_reloc32(in);
        break;
      case HUNK_RELOC16:
      case HUNK_RELOC8:
      case HUNK_EXT:
        running = 0;
        break;
      case HUNK_SYMBOL:
        parse_hunk_symbol(in);
        break;
      case HUNK_DEBUG:
      case HUNK_END:
        running = 0;
        break;
      case HUNK_HEADER:
        parse_hunk_header(in);
        break;
      case HUNK_OVERLAY:
      case HUNK_BREAK:
      case HUNK_DREL32:
      case HUNK_DREL16:
      case HUNK_DREL8:
      case HUNK_LIB:
      case HUNK_INDEX:
      case HUNK_RELOC32SHORT:
      case HUNK_RELRELOC32:
      case HUNK_ABSRELOC16:
      case HUNK_PPC_CODE:
      case HUNK_RELRELOC26:
      default:
        running = 0;
        break;
    }
  }

  fclose(in);

  return 0;
}


/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2019 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint32_t read_int(FILE *in)
{
  uint32_t c;

  c = getc(in) << 24;
  c = c | (getc(in) << 16);
  c = c | (getc(in) << 8);
  c = c | getc(in);

  return c;
}

uint32_t read_short(FILE *in)
{
  uint32_t c;

  c = getc(in) << 8;
  c = c | getc(in);

  return c;
}

int read_chars(FILE *in, uint8_t *s, int count)
{
  int t;

  for (t = 0; t < count; t++)
  {
    s[t] = getc(in);
  }

  s[t] = 0;

  return 0;
}


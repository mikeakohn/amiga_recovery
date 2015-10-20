/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2014 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#include <stdio.h>
#include <stdlib.h>

unsigned int read_int(FILE *in)
{
unsigned int c;

  //c=getc(in);
  //c=c|(getc(in)<<8);
  //c=c|(getc(in)<<16);
  //c=c|(getc(in)<<24);
  c=getc(in)<<24;
  c=c|(getc(in)<<16);
  c=c|(getc(in)<<8);
  c=c|getc(in);

  return c;
}

unsigned int read_short(FILE *in)
{
short int c;

  c=getc(in)<<8;
  c=c|getc(in);

  return c;
}

int read_chars(FILE *in, unsigned char *s, int count)
{
int t;

  for (t=0; t<count; t++)
  {
    s[t]=getc(in);
  }

  s[t]=0;

  return 0;
}



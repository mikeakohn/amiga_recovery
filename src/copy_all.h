/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2019 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

*/

#ifndef COPY_ALL_H
#define COPY_ALL_H

int copy_all(
  FILE *in,
   struct _amiga_bootblock *bootblock,
  struct _pwd *pwd,
  char *path);

#endif


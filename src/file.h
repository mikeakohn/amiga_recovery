/*

Amiga Recovery - Recover files from an Amiga AFFS disk image.

Copyright 2009-2019 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

*/

#ifndef FILE_H
#define FILE_H

#include "affs.h"

void copy_file(
  FILE *in,
  struct _amiga_bootblock *bootblock,
  struct _pwd *pwd,
  char *filename,
  FILE *out);

#endif


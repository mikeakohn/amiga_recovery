/*

Amiga Recovery - Recover files from an Amiga AFFS disk image
Copyright 2009-2015 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPL

*/

#ifndef FILEIO_H
#define FILEIO_H

int read_int(FILE *in);
int read_short(FILE *in);
int read_chars(FILE *in, unsigned char *s, int count);

#endif


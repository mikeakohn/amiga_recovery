/*

Amiga Recovery - Recover files from an Amiga AFFS disk imag.

Copyright 2009-2019 - Michael Kohn (mike@mikekohn.net)
http://www.mikekohn.net/

Released under GPLv3.

*/

#ifndef FILEIO_H
#define FILEIO_H

uint32_t read_int(FILE *in);
uint32_t read_short(FILE *in);
int read_chars(FILE *in, uint8_t *s, int count);

#endif


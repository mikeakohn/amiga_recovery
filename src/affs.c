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

#include "affs.h"
#include "fileio.h"

#define ST_FILE -3
#define ST_ROOT 1 
#define ST_USERDIR 2 
#define ST_SOFTLINK 3 
#define ST_LINKDIR 4 

/* hash_name function copied from http://lclevy.free.fr/adflib/adf_info.html */
static unsigned int hash_name(unsigned char *name)
{
uint32_t hash, l; /* sizeof(int)>=2 */
int i;

  l = hash = strlen((char *)name);

  for(i = 0; i < l; i++)
  {
    hash = hash * 13;
    hash = hash + toupper(name[i]); /* not case sensitive */
    hash = hash & 0x7ff;
  }

  /* 0 < hash < 71 in the case of 512 byte blocks */
  hash = hash % ((BSIZE / 4) - 56);

  return(hash);
}

int read_partition(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition)
{
unsigned int cylindar_size;
int namelen;

  // Should be "PART"
  read_chars(in, partition->magic, 4);

  if (memcmp(partition->magic, "PART", 4) != 0)
  {
    printf("Error: Partition's magic number is '%c%c%c%c' instead of'PART'\n",
      partition->magic[0],
      partition->magic[1],
      partition->magic[2],
      partition->magic[3]);
  }

  partition->size = read_int(in);
  partition->checksum = read_int(in);
  partition->scsihost = read_int(in);
  partition->next = read_int(in);
  partition->flags = read_int(in);
  //unsigned int partition->unused1[2];
  fseek(in, 4 * 2, SEEK_CUR);
  //read_int(in);
  //read_int(in);
  partition->devflags = read_int(in);
  namelen = getc(in);

  read_chars(in, partition->name, 31);
  partition->name[namelen] = 0;

  //unsigned int partition->unused2[15];
  //unsigned int partition->unused3[3];
  fseek(in, 4 * 15, SEEK_CUR);
  //partition->heads = read_int(in);
  //partition->unused4 = read_int(in);
  //partition->block_per_track = read_int(in);
  //unsigned int partition->unused5[3];
  //fseek(in, 4*3, SEEK_CUR);
  //partition->lowcyl = read_int(in);
  //partition->highcyl = read_int(in);
  //partition->firstcyl = read_int(in);

  //fseek(in, 0x3a, SEEK_CUR);

  partition->table_size = read_int(in);
  partition->size_block = read_int(in);
  partition->sec_org = read_int(in);
  partition->heads = read_int(in);
  partition->sectors_per_block = read_int(in);
  partition->blocks_per_track = read_int(in);
  partition->num_reserved = read_int(in);
  partition->pre_alloc = read_int(in);
  partition->interleave = read_int(in);
  partition->start_cyl = read_int(in);
  partition->end_cyl = read_int(in);
  partition->num_buffers = read_int(in);
  partition->buff_type = read_int(in);
  partition->max_transfer = read_int(in);
  partition->mask = read_int(in);
  partition->boot_priority = read_int(in);

  read_chars(in, partition->type, 4);
  if (partition->type[0] == 'D' && partition->type[1] == 'O' &&
      partition->type[2] == 'S')
  { partition->type[3] += '0'; }

  partition->baud = read_int(in);
  partition->control = read_int(in);
  partition->bootblocks = read_int(in);

  cylindar_size = bootblock->blksz*partition->heads*
                partition->sectors_per_block * partition->blocks_per_track;
  partition->start = cylindar_size*partition->start_cyl;

  //partition->size_in_bytes=partition->end-partition->start;
  partition->size_in_bytes = (partition->end_cyl-partition->start_cyl+1)*cylindar_size;
  partition->end = partition->start+partition->size_in_bytes;

  return 0;
}

static unsigned int calc_rootblock(struct _amiga_bootblock *bootblock, struct _amiga_partition *partition)
{
  return (((((partition->end_cyl - partition->start_cyl + 1) *
            partition->heads * partition->blocks_per_track) - 1) +
            partition->num_reserved) / 2) * bootblock->blksz;
}


void read_rootblock(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_rootblock *rootblock)
{
unsigned int offset;
int namelen;
int t;

  offset = calc_rootblock(bootblock, partition); // + bootblock->offset;

  rootblock->partition_offset = offset;
  rootblock->disk_offset = offset + partition->start;

  fseek(in, offset + partition->start, SEEK_SET);
  rootblock->type = read_int(in);
  rootblock->header_key = read_int(in);
  rootblock->high_seq = read_int(in);
  rootblock->hash_table_size = read_int(in);
  rootblock->first_size = read_int(in);
  rootblock->checksum = read_int(in);

  for (t = 0; t < rootblock->hash_table_size; t++)
  {
    rootblock->hash_table[t] = read_int(in);
    //hash_table[512/4-56];
  }
  rootblock->bm_flag = read_int(in);
  for (t = 0; t < 25; t++)
  {
    rootblock->bm_pages[t] = read_int(in);
  }
  rootblock->bm_ext = read_int(in);
  rootblock->r_days = read_int(in);
  rootblock->r_mins = read_int(in);
  rootblock->r_ticks = read_int(in);

  namelen = getc(in);

  read_chars(in, rootblock->diskname, 31);
  rootblock->diskname[namelen] = 0;

  //unsigned int unused1[2];
  read_int(in);
  read_int(in);
  rootblock->v_days = read_int(in);
  rootblock->v_min = read_int(in);
  rootblock->v_ticks = read_int(in);
  rootblock->c_days = read_int(in);
  rootblock->c_min = read_int(in);
  rootblock->c_ticks = read_int(in);
  rootblock->next_hash = read_int(in);
  rootblock->parent_dir = read_int(in);
  rootblock->extension = read_int(in);
  rootblock->sec_type = read_int(in);
}

void read_fileheader(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_fileheader *fileheader, unsigned int block)
{
int namelen;
int t;

  fseek(in, partition->start + (block * bootblock->blksz), SEEK_SET);

  fileheader->type = read_int(in);
  fileheader->header_key = read_int(in);
  fileheader->high_seq = read_int(in);
  fileheader->data_size = read_int(in);
  fileheader->first_data = read_int(in);
  fileheader->checksum = read_int(in);
  //unsigned int datablocks[512/4-56];  //FIXME - wrong size?
  for (t = 0; t < (512 / 4 - 56); t++)
  {
    fileheader->datablocks[t] = read_int(in);
  }
  fileheader->unused1 = read_int(in);
  fileheader->uid = read_short(in);
  fileheader->gid = read_short(in);
  fileheader->protect = read_int(in);
  fileheader->byte_size = read_int(in);
  //unsigned char comment[80]; // first char is len
  namelen = getc(in);
  for (t = 0; t < 79; t++)
  {
    fileheader->comment[t] = getc(in);
  }
  fileheader->comment[namelen] = 0;
  //unsigned char unused2[12];
  fseek(in, 12, SEEK_CUR);
  fileheader->days = read_int(in);
  fileheader->mins = read_int(in);
  fileheader->ticks = read_int(in);
  //unsigned char filename[32]; // first char is len, last char is unused
  namelen = getc(in);
  for (t = 0; t < 31; t++)
  {
    fileheader->filename[t] = getc(in);
  }
  fileheader->filename[namelen] = 0;
  fileheader->unused3 = read_int(in);
  fileheader->read_entry = read_int(in);
  fileheader->next_link = read_int(in);
  //unsigned int unused4[5];
  fseek(in, 5 * 4, SEEK_CUR);
  fileheader->hash_chain = read_int(in);
  fileheader->parent = read_int(in);
  fileheader->extension = read_int(in);
  fileheader->sec_type = read_int(in);
}

void read_file_ext(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_file_ext *file_ext, unsigned int block)
{
int t;

  fseek(in, partition->start + (block * bootblock->blksz), SEEK_SET);

  file_ext->type = read_int(in);
  file_ext->header_key = read_int(in);
  file_ext->high_seq = read_int(in);
  //unsigned int unused1;
  //unsigned int unused2;
  fseek(in, 2 * 4, SEEK_CUR);
  file_ext->checksum = read_int(in);
  //unsigned int datablocks[BSIZE/4-56]
  for (t = 0; t<(512 / 4 - 56); t++)
  {
    file_ext->datablocks[t] = read_int(in);
  }
  //unsigned int info[46];
  fseek(in, 46 * 4, SEEK_CUR);
  file_ext->unused3 = read_int(in);
  file_ext->parent = read_int(in);
  file_ext->extension = read_int(in);
  file_ext->sec_type = read_int(in);
}

void read_directory(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_directory *directory, unsigned int block)
{
int namelen;
int t;

  fseek(in, partition->start + (block * bootblock->blksz), SEEK_SET);

  directory->type = read_int(in);
  directory->header_key = read_int(in);
  //unsigned int unused1[3];
  fseek(in, 3 * 4, SEEK_CUR);
  directory->checksum = read_int(in);
  //unsigned int hash_table[512/4-56];
  for (t = 0; t < (512 / 4 - 56); t++)
  {
    directory->hash_table[t] = read_int(in);
  }
  //unsigned int unused2[2];
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
  //unsigned int unused5[2];
  fseek(in, 2 * 4, SEEK_CUR);
  directory->next_link = read_int(in);
  //unsigned int unused6[5];
  fseek(in, 5 * 4, SEEK_CUR);
  directory->hash_chain = read_int(in);
  directory->parent = read_int(in);
  directory->extension = read_int(in);
  directory->sec_type = read_int(in);
}

void read_datablock(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_datablock *datablock, unsigned int block)
{
int t;

  fseek(in, partition->start + (block * bootblock->blksz), SEEK_SET);

  datablock->type = read_int(in);
  datablock->header_key = read_int(in);
  datablock->seq_num = read_int(in);
  datablock->data_size = read_int(in);
  datablock->next_data = read_int(in);
  datablock->checksum = read_int(in);
  for (t = 0; t < 512 - 24; t++)
  {
    datablock->data[t] = getc(in);
  }
}

static void print_hash_info(FILE *in, struct _amiga_rootblock *rootblock, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, unsigned int block)
{
struct _amiga_directory directory;
struct _amiga_fileheader fileheader;
int sec_type;

  sec_type = get_sec_type(in, bootblock, partition,block);

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

void list_directory(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd)
{
//struct _amiga_rootblock rootblock;
int t;

  //read_rootblock(in,bootblock,partition,&rootblock);

  for (t = 0; t < 72; t++)
  {
    if (pwd->dir_hash[t] != 0)
    {
      print_hash_info(in, &pwd->rootblock, bootblock, &pwd->partition, pwd->dir_hash[t]);
    }
  }
}

static void print_file_at_block(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, struct _amiga_fileheader *fileheader, FILE *out)
{
//struct _amiga_datablock datablock;
struct _amiga_file_ext file_ext;
unsigned int *datablocks;
unsigned int bytes_left;
unsigned char buffer[bootblock->blksz];
unsigned int next_block;
int curr;
int len;
int t;

  datablocks = fileheader->datablocks;
  bytes_left = fileheader->byte_size;
  next_block = fileheader->extension;

  // print_fileheader(fileheader);

  while(bytes_left > 0)
  {
    for (curr = 71; curr >= 0; curr--)
    {
      if (datablocks[curr] == 0) break;

      fseek(in, partition->start + (datablocks[curr] * bootblock->blksz), SEEK_SET); 
      if (bytes_left>bootblock->blksz)
      {
        len=fread(buffer, 1, bootblock->blksz, in);
        bytes_left -= bootblock->blksz;
      }
        else
      {
        len=fread(buffer, 1, bytes_left, in);
        bytes_left -= bytes_left;
      }

      if (out == NULL)
      {
        for (t = 0; t < len; t++) { putchar(buffer[t]); }
      }
        else
      {
        fwrite(buffer, 1, len, out);
      }
    }

    if (next_block==0) break;

    read_file_ext(in, bootblock, partition, &file_ext, next_block);
    if (file_ext.type != 16)
    {
      printf("Error: File extension wasn't read right?  Bug?\n");
      break;
    }
    //print_file_ext(&file_ext);
    next_block = file_ext.extension;
    datablocks = file_ext.datablocks;
  }

  if (out == NULL) { printf("\n"); }
}

void print_file(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd, char *filename, FILE *out)
{
struct _amiga_directory directory;
struct _amiga_fileheader fileheader;
unsigned int block;
int sec_type;

  block = pwd->dir_hash[hash_name((unsigned char*)filename)];

  while(block != 0)
  {
    sec_type = get_sec_type(in, bootblock, &pwd->partition, block);

    if (sec_type == ST_USERDIR)
    {
      read_directory(in, bootblock, &pwd->partition, &directory, block);

      if (directory.hash_chain == 0) break;

      block = directory.hash_chain;
    }
      else
    if (sec_type == ST_FILE)
    {
      read_fileheader(in, bootblock, &pwd->partition, &fileheader, block);

      if (strcmp((char *)fileheader.filename, filename) == 0)
      {
        print_file_at_block(in, bootblock, &pwd->partition, &fileheader, out);
        return;
      }

      if (fileheader.hash_chain == 0) break;

      block = fileheader.hash_chain;
    }
      else
    {
      printf("Unknown sec_type %d\n", sec_type);
      break;
    }
  }

  printf("Error: file '%s' not found\n", filename);
}

void print_partition(struct _amiga_partition *partition)
{
  printf("===================== Partition ======================\n");
  printf("            magic: %4.4s\n", partition->magic);
  printf("             size: %d\n", partition->size);
  printf("         checksum: %d\n", partition->checksum);
  printf("         scsihost: %d\n", partition->scsihost);
  printf("             next: %d\n", partition->next);
  printf("            flags: %d\n", partition->flags);
  // partition->unused1[2];
  printf("         devflags: %d\n", partition->devflags);
  printf("             name: %s\n", partition->name);
  printf("       table_size: %d\n", partition->table_size);
  printf("       size_block: %d\n", partition->size_block);
  printf("          sec_org: %d\n", partition->sec_org);
  printf("            heads: %d\n", partition->heads);
  printf("sectors_per_block: %d\n", partition->sectors_per_block);
  printf(" blocks_per_track: %d\n", partition->blocks_per_track);
  printf("     num_reserved: %d\n", partition->num_reserved);
  printf("        pre_alloc: %d\n", partition->pre_alloc);
  printf("       interleave: %d\n", partition->interleave);
  printf("        start_cyl: %d (addr: %u)\n", partition->start_cyl, partition->start);
  printf("          end_cyl: %d (addr: %u)\n", partition->end_cyl, partition->end);
  printf("                   %u bytes\n", partition->size_in_bytes);
  printf("      num_buffers: %d\n", partition->num_buffers);
  printf("        buff_type: %d\n", partition->buff_type);
  printf("     max_transfer: %d\n", partition->max_transfer);
  printf("             mask: %d\n", partition->mask);
  printf("    boot_priority: %d\n", partition->boot_priority);

  printf("             type: %4.4s\n", partition->type);
 
  printf("             baud: %d\n", partition->baud);
  printf("          control: %d\n", partition->control);
  printf("       bootblocks: %d\n", partition->bootblocks);

  printf("\n");
}

void print_rootblock(struct _amiga_rootblock *rootblock)
{
int t;

  printf("===================== Rootblock ======================\n");

  printf("             type: %d\n", rootblock->type);
  printf("       header_key: %d\n", rootblock->header_key);
  printf("         high_seq: %d\n", rootblock->high_seq);
  printf("  hash_table_size: %d\n", rootblock->hash_table_size);
  printf("       first_size: %d\n", rootblock->first_size);
  printf("         checksum: %d\n", rootblock->checksum);
  printf("       hash_table: ");
  for (t = 0; t < rootblock->hash_table_size; t++)
  {
    if (t != 0) printf(" ");
    printf("%d", rootblock->hash_table[t]);
  }
  printf("\n");
  printf("          bm_flag: %d\n", rootblock->bm_flag);
  printf("         bm_pages: ");
  for (t = 0; t < 25; t++)
  {
    if (t != 0) printf(" ");
    printf("%d", rootblock->bm_pages[t]);
  }
  printf("\n");
  printf("           bm_ext: %d\n", rootblock->bm_ext);
  printf("           r_days: %d\n", rootblock->r_days);
  printf("           r_mins: %d\n", rootblock->r_mins);
  printf("          r_ticks: %d\n", rootblock->r_ticks);
  printf("         diskname: %s\n", rootblock->diskname);
  printf("          unused1: %04x %04x\n", rootblock->unused1[0], rootblock->unused1[1]);
  printf("           v_days: %d\n", rootblock->v_days);
  printf("            v_min: %d\n", rootblock->v_min);
  printf("          v_ticks: %d\n", rootblock->v_ticks);
  printf("           c_days: %d\n", rootblock->c_days);
  printf("            c_min: %d\n", rootblock->c_min);
  printf("          c_ticks: %d\n", rootblock->c_ticks);
  printf("        next_hash: %d\n", rootblock->next_hash);
  printf("       parent_dir: %d\n", rootblock->parent_dir);
  printf("        extension: %d\n", rootblock->extension);
  printf("         sec_type: %d\n\n", rootblock->sec_type);

  printf(" partition_offset: 0x%08x (%d)\n", rootblock->partition_offset, rootblock->partition_offset);
  printf("      disk_offset: 0x%08x (%d)\n", rootblock->disk_offset, rootblock->disk_offset);
}

void print_fileheader(struct _amiga_fileheader *fileheader)
{
int t;

  printf("================== File Header ===================\n");
  printf("             type: %d\n", fileheader->type);
  printf("       header_key: %d\n", fileheader->header_key);
  printf("         high_seq: %d\n", fileheader->high_seq);
  printf("        data_size: %d\n", fileheader->data_size);
  printf("       first_data: %d\n", fileheader->first_data);
  printf("         checksum: %d\n", fileheader->checksum);
  printf("       datablocks:");
  for (t = 0; t < (512 / 4 - 56); t++)
  {
    if (t != 0) printf(" ");
    printf("%d", fileheader->datablocks[t]);
  }
  printf("\n");
  printf("              uid: %d\n", fileheader->uid);
  printf("              gid: %d\n", fileheader->gid);
  printf("          protect: %d\n", fileheader->protect);
  printf("        byte_size: %d\n", fileheader->byte_size);
  printf("          comment: %s\n", fileheader->comment);
  printf("             days: %d\n", fileheader->days);
  printf("             mins: %d\n", fileheader->mins);
  printf("            ticks: %d\n", fileheader->ticks);
  printf("         filename: %s\n", fileheader->filename);
  printf("       read_entry: %d\n", fileheader->read_entry);
  printf("        next_link: %d\n", fileheader->next_link);
  printf("       hash_chain: %d\n", fileheader->hash_chain);
  printf("           parent: %d\n", fileheader->parent);
  printf("        extension: %d\n", fileheader->extension);
  printf("         sec_type: %d\n", fileheader->sec_type);
}

void print_file_ext(struct _amiga_file_ext *file_ext)
{
int t;

  printf("================== File Extension ===================\n");
  printf("             type: %d\n", file_ext->type);
  printf("       header_key: %d\n", file_ext->header_key);
  printf("         high_seq: %d\n", file_ext->high_seq);
  printf("         checksum: %d\n", file_ext->checksum);
  printf("       datablocks: ");
  for (t = 0; t < (512 / 4 - 56); t++)
  {
    if (t != 0) { printf(" "); }
    printf("%d", file_ext->datablocks[t]);
  }
  printf("\n");
  printf("           parent: %d\n", file_ext->parent);
  printf("        extension: %d\n", file_ext->extension);
  printf("         sec_type: %d\n", file_ext->sec_type);
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

void print_datablock(struct _amiga_datablock *datablock)
{
char ascii[17];
int ptr;
int t;

  printf("================== Datablock ===================\n");
  printf("             type: %d\n", datablock->type);
  printf("       header_key: %d\n", datablock->header_key);
  printf("          seq_num: %d\n", datablock->seq_num);
  printf("        data_size: %d\n", datablock->data_size);
  printf("        next_data: %d\n", datablock->next_data);
  printf("         checksum: %d\n", datablock->checksum);
  printf("             data:\n");

  if (datablock->type != 8) return;
  ptr = 0;

  for (t = 0; t < datablock->data_size; t++)
  {
    if ((t % 16) == 0)
    {
      ascii[ptr] = 0;
      if (ptr != 0) printf("  %s",ascii);
      printf("\n");
      ptr = 0;
    }

    if ((t % 16) != 0) printf(" ");
    printf("%02x", datablock->data[t]);

    if (datablock->data[t] >= 32 && datablock->data[t] < 127)
    { ascii[ptr++] = datablock->data[t]; }
      else
    { ascii[ptr++] = '.'; }
  }

  //FIXME - stuff missing at bottom..
}

void print_partition_list(FILE *in, struct _amiga_bootblock *bootblock)
{
struct _amiga_partition partition;
int count,t;

  count = 0;
  t = bootblock->partitionlst;
  while(t > 0)
  {
    fseek(in, t * 512, SEEK_SET);
    read_partition(in, bootblock, &partition);
    print_partition(&partition);

    t = partition.next;
    count++;
  }
}

void show_partitions(FILE *in, struct _amiga_bootblock *bootblock)
{
struct _amiga_partition partition;
int count,t;

  count = 0;
  t = bootblock->partitionlst;
  printf("%-20s %4s %10s %10s %12s %12s\n", "Name", "Type", "Start Cyl", "End Cyl", "Offset", "Size");

  while(t > 0)
  {
    fseek(in, t * 512, SEEK_SET);
    read_partition(in, bootblock, &partition);
    printf("%-20s %4.4s %10d %10d %12d %12d\n", partition.name, partition.type, partition.start_cyl, partition.end_cyl, partition.start, partition.size_in_bytes);

    t = partition.next;
    count++;
  }
}

int dump_partition(FILE *in, struct _amiga_bootblock *bootblock, int num, const char *filename)
{
struct _amiga_partition partition;
int count,t;
FILE *out;
unsigned int size;
unsigned char buffer[8192];

  count = 0;
  t = bootblock->partitionlst;
  while(t > 0)
  {
    fseek(in, t * 512, SEEK_SET);
    read_partition(in, bootblock, &partition);
    //print_partition(&partition);

    if (count == num)
    {
      printf("Creating %s from partition %d (%s)\n", filename, num, partition.name);
      out = fopen(filename, "wb");
      if (out == NULL)
      {
        printf("Could not open file %s for writing...\n", filename);
        return -1;
      }

      size = partition.size_in_bytes;

      fseek(in, partition.start, SEEK_SET);

      while(size > 0)
      {
        if (size > 8192)
        {
          t = fread(buffer, 1, 8192, in);
          if (t < 1) { printf("wtf\n"); break; }
          fwrite(buffer, 1, t, out);
          size -= t;
        }
        else
        {
          t = fread(buffer, 1, size, in);
          if (t < 1) { printf("wtf\n"); break; }
          fwrite(buffer, 1, t, out);
          size -= t;
        }
      }

      fclose(out);

      return 0;
    }

    t=partition.next;
    count++;
  }

  return -1;
}

int read_partition_num(FILE *in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, int num)
{
int count,t;

  count = 0;
  t = bootblock->partitionlst;
  while(t > 0)
  {
    fseek(in, t * 512, SEEK_SET);
    read_partition(in, bootblock, partition);

    if (count == num)
    {
      return 0;
    }

    t = partition->next;
    count++;
  }

  return -1;
}

int get_partition_num(FILE *in, struct _amiga_bootblock *bootblock, char *name)
{
struct _amiga_partition partition;
int count,t;

  count = 0;
  t = bootblock->partitionlst;
  while(t > 0)
  {
    fseek(in, t * 512, SEEK_SET);
    read_partition(in, bootblock, &partition);

    if (strcmp((char *)partition.name, name)==0)
    {
      return count;
    }

    t = partition.next;
    count++;
  }

  return -1;
}

int ch_dir(FILE *in, struct _amiga_bootblock *bootblock, struct _pwd *pwd, char *dirname)
{
struct _amiga_directory directory;
struct _amiga_fileheader fileheader;

  if (strcmp(dirname, "..")==0)
  {
    if (pwd->parent_dir == 0) { return -1; }

    if (pwd->parent_dir==pwd->rootblock.header_key)
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
      while(l >= 0)
      {
        if (pwd->cwd[l] == '/') break;
        pwd->cwd[l--] = 0;
      }
    }
  }
    else
  {
    int block = pwd->dir_hash[hash_name((unsigned char*)dirname)];

    while(block != 0)
    {
      unsigned int sec_type = get_sec_type(in,bootblock,&pwd->partition,block);

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

        if (directory.hash_chain == 0) break;
        block = directory.hash_chain;
      }
        else
      if (sec_type == ST_FILE)
      {
        read_fileheader(in, bootblock, &pwd->partition, &fileheader, block);

        if (fileheader.hash_chain == 0) break;

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

int get_sec_type(FILE * in, struct _amiga_bootblock *bootblock, struct _amiga_partition *partition, unsigned int block)
{
  fseek(in, partition->start + (block * bootblock->blksz + (bootblock->blksz - 4)), SEEK_SET);

  return read_int(in);
}



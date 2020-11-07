//=============================================================================
// mmaloc.c
// Simple managed memory allocation routines
// nyacom (C) 2020.10
//=============================================================================
#define _FILE_OFFSET_BITS 64     // For large file over 2GB
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

//=============================================================================
#define MMALLOC_BLOCK_SIZE 4096   // do not smaller than linux paging size

typedef struct _mmallocEnt {
    void   *ptr;                  // Correspond address pointer for this area
    off_t  p_addr;                // Physical address on the file
    size_t p_size;                // Physical size
    struct _mmallocEnt *prev;     // Prev entry
    struct _mmallocEnt *next;     // Next entry
} mmallocEnt;

typedef struct _mmallocInfo {
    int        mmap_fd;       // File descriptor for mmap target
    off_t      offset;            // Valid area offset
    size_t     area_size;         // Valid area size
    mmallocEnt *ent;              // Entry list
} mmallocInfo;

//=============================================================================
extern mmallocInfo info;

extern int mmallocInit(const char *path, off_t offset, size_t size);
extern void *mmalloc(size_t size);
extern void mfree(void *ptr);
extern unsigned long mget_phy_addr(void *ptr);
extern void mmsync();
extern void minfo();
void mmread(void *ptr);

//=============================================================================
// mmaloc.c
// Simple managed memory allocation routines
// nyacom (C) 2020.10
//=============================================================================
#include "mmalloc.h"
#include "ficlib2.h"

mmallocInfo info;

//=============================================================================
int mmallocInit(const char *path, off_t offset, size_t size) {
    int mem_fd = open(path, O_RDWR|O_SYNC|O_DIRECT);
	if (mem_fd < 0) {
		fprintf(stderr, "[ERROR]: can't open %s errno=%d\n", path, errno);
        return mem_fd;
	}

    info.mmap_fd   = mem_fd;
    info.offset    = offset;
    info.area_size = size;

    info.ent       = (mmallocEnt *)malloc(sizeof(mmallocEnt));
    info.ent->next = NULL;
    info.ent->prev = NULL;

    return 0;
}

//=============================================================================
void mmsync() {
    mmallocEnt *ent = info.ent->next;
    while (ent) {
        printf("msync %p %d\n", ent->ptr, ent->p_size);
        msync(ent->ptr, ent->p_size, MS_SYNC|MS_INVALIDATE);
        ent = ent->next;
    }
}

//=============================================================================
void mmread(void *ptr) {
    mmallocEnt *ent = info.ent->next;
    while (ent) {
        if (ent->ptr == ptr) {
            int fd = fic_gpio_open();
            fic_comm_reset();
            fic_hls_ddr_read(ent->ptr, ent->p_size, ent->p_addr);
            fic_gpio_close(fd);
            break;
        }
        ent = ent->next;
    }
}

//=============================================================================
void *mmalloc(size_t size) {
//    off_t free_offset = info.offset;
    off_t free_offset = 0;
    off_t free_size   = (size / MMALLOC_BLOCK_SIZE + 1) * MMALLOC_BLOCK_SIZE;

    // Find allocatable area
    mmallocEnt *ent_prev = info.ent;
    mmallocEnt *ent      = info.ent->next;

    while (ent) {
        off_t st_addr = ent->p_addr;
        off_t ed_addr = st_addr + ent->p_size;

        if (free_offset >= st_addr) {
            free_offset = ed_addr + MMALLOC_BLOCK_SIZE;
        } else if (free_offset+free_size < st_addr) {
            break;
        }

        ent_prev = ent;
        ent = ent->next;
    }

    if ((free_offset + free_size) > (info.offset + info.area_size)) {
		fprintf(stderr, "[ERROR]: can't allocate area \n");
        return NULL;    // Cant find area
    }

    // Create new memory entry
    mmallocEnt *new_ent = malloc(sizeof(mmallocEnt));
    if (new_ent == NULL) {
		fprintf(stderr, "[ERROR]: malloc failed\n");
        return NULL;    // Cant find area
    }

//    printf("DEBUG: addr=%lu size=%lu\n", free_offset, free_size);

	// MMap a file
	void *ptr = NULL;
    ptr = mmap(
        NULL,                   // Any adddress in our space will do
        free_size,              // Map length
        PROT_READ | PROT_WRITE, // Enable reading & writting to mapped memory
        MAP_SHARED,            //  shared with other processes
//        MAP_PRIVATE,            //  shared with other processes
        info.mmap_fd,          // File to map
        free_offset             // Offset to GPIO peripheral
    );

    if ((int)ptr < 0) {
        printf("[ERROR]: mmap errno=%d\n", errno);
        return NULL;
    }

//    printf("DEBUG: addr=%lu size=%lu ptr=%x\n", free_offset, free_size, ptr);

    new_ent->p_addr = free_offset;
    new_ent->p_size = free_size;
    new_ent->ptr    = ptr;
    new_ent->prev   = NULL;
    new_ent->next   = NULL;

    // Add to list
    if (ent) {
        // Insert
        new_ent->prev = ent->prev;
        new_ent->next = ent;
        if (ent->prev) {
            ent->prev->next = new_ent;
        }
        ent->prev = new_ent;

    } else {
        // Append
        ent_prev->next = new_ent;
        new_ent->prev = ent_prev;

    }

   return ptr;
}

//=============================================================================
void mfree(void *ptr) {
    // Find ptr area
    mmallocEnt *ent_prev = info.ent;
    mmallocEnt *ent      = info.ent->next;

    while (ent) {
        if (ent->ptr == ptr) {

            munmap(ent->ptr, ent->p_size);

            if (ent->prev) {
                ent->prev->next = ent->next;
            }

            if (ent->next) {
                ent->next->prev = ent->prev;
            }

            free(ent);

            return;
        }
        ent = ent->next;
    }

    return;
}

//=============================================================================
unsigned long mget_phy_addr(void *ptr) {
    mmallocEnt *ent = info.ent->next;
    while (ent) {
        if (ent->ptr == ptr) {
            return ent->p_addr;
        }
        ent = ent->next;
    }

    return 0;
}

//=============================================================================
void minfo() {
    mmallocEnt *ent = info.ent->next;

    printf("\n");
    printf(" mmalloc Managed memory allocation information\n");
    printf(" Address    Size       Pointer\n");

    printf("--------------------------------------------------------------\n");
    while (ent) {
        printf(" %08llx   %8u", ent->p_addr, ent->p_size);
        printf("    %p\n",ent->ptr);
        ent = ent->next;
    }

    printf("\n\n");
}

//=============================================================================
// Tests
//=============================================================================
/*
int main() {
    printf("mmalloc test\n");

    // Init
    mmallocInit("./mmalloc.bin", 0x0, 1024*1024);
    //mmallocInit("ficmem/mem", 0x0, 0xffffffff);

    void *p1 = mmalloc(4096);
    minfo();
    *(uint32_t *)p1 = 0x12341234;
    printf("Virtual:%016lx  Physical:%016lx\n", p1, mget_phy_addr(p1));

    void *p2 = mmalloc(8192);
    minfo();
    *(uint64_t *)p2 = 0xffffaaaabbbbcccc;
    printf("Virtual:%016lx  Physical:%016lx\n", p2, mget_phy_addr(p2));

    void *p3 = mmalloc(8192);
    minfo();
    printf("Virtual:%016lx  Physical:%016lx\n", p3, mget_phy_addr(p3));

    mfree(p2);
    minfo();

    void *p4 = mmalloc(8192);
    minfo();

    printf("Virtual:%08lx  Physical:%08lx\n", p4, mget_phy_addr(p4));

}
*/
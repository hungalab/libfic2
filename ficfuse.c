//=============================================================================
// FiC RasDDR FUSE driver
// nyacom (C) 2020.10
//=============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <errno.h>

#define FUSE_USE_VERSION 31
#include <fuse.h>

#include "ficlib2.h"

//=============================================================================
#define FIC_MEM_SIZE 0xffffffff     // 4GB area
#define FIC_REG_SIZE 0x7fffffff     // 2GB area

//=============================================================================
// FUSE callbacks
//=============================================================================
static int _getattr(const char *path, struct stat *st) {

    printf("\n\n get attr \n\n");

    if (strcmp(path, "/") == 0 ) {
        st->st_mode   = S_IFDIR | 0755;
        st->st_nlink  = 2;
        return 0;
    }

    if (strcmp(path, "/mem") == 0 ) {
        st->st_mode   = S_IFREG | 0644;
        st->st_nlink  = 1;
        st->st_uid    = getuid();
        st->st_gid    = getgid();
        st->st_size   = FIC_MEM_SIZE;
        st->st_blocks = 0;
        st->st_atime  = st->st_mtime = st->st_ctime = time(NULL);
        return 0;

    }

    if (strcmp(path, "/reg") == 0 ) {
        st->st_mode   = S_IFREG | 0644;
        st->st_nlink  = 1;
        st->st_uid    = getuid();
        st->st_gid    = getgid();
        st->st_size   = FIC_REG_SIZE;
        st->st_blocks = 0;
        st->st_atime  = st->st_mtime = st->st_ctime = time(NULL);
        return 0;

    }

    return -ENOENT;
}

static int _readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                    off_t offset, struct fuse_file_info *fi) {
    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0) {
        return -ENOENT;
    }

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, "mem", NULL, 0);
    filler(buf, "reg", NULL, 0);

    return 0;
}

static int _read(const char *path, char *buf, size_t size, off_t offset,
                 struct fuse_file_info *fi) {

    int i, j;
    int fd_gpio = 0;

    // MEM
    if (strcmp(path, "/mem") == 0) {

        if (offset + size > FIC_MEM_SIZE) {
            return -EFAULT;
        }

        printf("\nFICFUSE DEBUG[%d]: READ path=%s addr=%llu size=%zu\n", __LINE__, path, offset, size);

        // Check size
        if (size == 0) {
            return -EIO;
        }

        // Open GPIO
        fd_gpio = fic_gpio_open();
        if (fd_gpio < 0) {
            return -EIO;
        }

        fic_comm_reset();

        // Note: FiC DDR is 4B aligned access only
        size_t size_left = size;

        // Check misaligned start offset
        off_t mis_off_st = (offset % 4);
        if (mis_off_st != 0) {
            // Read out 1st 4B and transfer
            u_int8_t r_buf[4];
            if (fic_hls_ddr_read(r_buf, 4, offset) < 0) {
                goto READ_IO_ERROR;
            }

            size_t tx_size = 4 - mis_off_st;

            if (tx_size > size_left) {
                // A case for size is under 4B
                printf("FICFUSE DEBUG[%d]: READ path=%s addr=%llu offt=%llu size=%zu\n", __LINE__, path, offset, mis_off_st, size_left);
                memcpy(buf, r_buf + mis_off_st, size_left);
                buf       += size_left;
                offset    += size_left;
                size_left -= size_left;


            } else {
                printf("FICFUSE DEBUG[%d]: READ path=%s addr=%llu offt=%llu size=%zu\n", __LINE__, path, offset, mis_off_st, tx_size);
                memcpy(buf, r_buf + mis_off_st, tx_size);
                buf       += tx_size;
                offset    += tx_size;
                size_left -= tx_size;

            }
        }

        if (size_left > 0) {
            // Check misaligned end offset
            off_t mis_off_ed = ((offset + size_left) % 4);
            if (mis_off_ed != 0) {
                int tx_size = size_left - mis_off_ed;

                if (tx_size > 3) {
                    printf("FICFUSE DEBUG[%d]: READ path=%s addr=%08llx size=%zu\n", __LINE__, path, offset, tx_size);
                    if (fic_hls_ddr_read(buf, tx_size, offset) < 0) {
                        goto READ_IO_ERROR;
                    }
                    buf    += tx_size;
                    offset += tx_size;
                }
                
                // Read out last 4B and transfer
                u_int8_t r_buf[4];
                if (fic_hls_ddr_read(r_buf, 4, offset) < 0) {
                    goto READ_IO_ERROR;
                }
                printf("FICFUSE DEBUG[%d]: READ path=%s addr=%08llx size=%zu\n", __LINE__, path, offset, mis_off_ed);
                memcpy(buf, r_buf, mis_off_ed);

            } else {
                if (fic_hls_ddr_read(buf, size_left, offset) < 0) {
                    goto READ_IO_ERROR;
                }

            }
        }

        // Close GPIO
        fic_gpio_close(fd_gpio);
    }
    
    // REG
    if (strcmp(path, "/reg") == 0) {
        if (offset > FIC_MEM_SIZE) {
            return -EFAULT;
        }

        // Open GPIO
        fd_gpio = fic_gpio_open();
        if (fd_gpio < 0) {
            return -EIO;
        }

        fic_comm_reset();

        printf("FICFUSE DEBUG[%d]: READ path=%s addr=%08llx size=%zu\n", __LINE__, path, offset, size);

        //u_int8_t r_buf[4];
        if (fic_hls_read(buf, size, offset, RASDDR_CMD_AXI_READ) < 0) {
            goto READ_IO_ERROR;
        }
        //memcpy(buf, r_buf, 4);

        //printf("FICFUSE DEBUG[%d]: %08x %08x %x08 %08x\n", buf[0], buf[1], buf[2], buf[3]);

        // Close GPIO
        fic_gpio_close(fd_gpio);
    }

    return size;

READ_IO_ERROR:
    fic_gpio_close(fd_gpio);
    return -EIO;

}

static int _write(const char *path, const char *buf, size_t size, off_t offset,
                 struct fuse_file_info *fi) {

    int fd_gpio = 0;

    // MEM
    if (strcmp(path, "/mem") == 0) {

        if (offset + size > FIC_MEM_SIZE) {
            return -EFAULT;
        }

        // Check size
        if (size == 0) {
            return -EIO;
        }

        printf("\nFICFUSE DEBUG[%d]: WRITE path=%s addr=%llu size=%zu\n", __LINE__, path, offset, size);

        // Open GPIO
        fd_gpio = fic_gpio_open();
        if (fd_gpio < 0) {
            return -EIO;
        }

        fic_comm_reset();

        // Note: FiC DDR is 4B aligned access only
        size_t size_left = size;
        char *buf_p = buf;

        // Check misaligned start offset
        off_t mis_off_st = (offset % 4);
        if (mis_off_st != 0) {
            // Read out 1st 4B and modify then write (RMW)
            size_t tx_size = 4 - mis_off_st;

            u_int8_t r_buf[4];
            if (fic_hls_ddr_read(r_buf, 4, offset) < 0) {
                goto WRITE_IO_ERROR;
            }

            if (tx_size > size_left) {
                memcpy(r_buf + mis_off_st, buf_p, size_left);
                if (fic_hls_ddr_write(r_buf, 4, offset) < 0) {
                    goto WRITE_IO_ERROR;
                }

                buf_p     += tx_size;
                offset    += tx_size;
                size_left -= tx_size;

            } else {
               memcpy(r_buf + mis_off_st, buf_p, tx_size);
                if (fic_hls_ddr_write(r_buf, 4, offset) < 0) {
                    goto WRITE_IO_ERROR;
                }

                buf_p     += tx_size;
                offset    += tx_size;
                size_left -= tx_size;

            }
        }

        if (size_left > 0) {
            // Check misaligned end offset
            off_t mis_off_ed = ((offset + size_left) % 4);
            if (mis_off_ed != 0) {
                //size_t mis_size = 4 - mis_off_ed;
                size_t tx_size = size_left - mis_off_ed;

                if (tx_size > 3) {
                    if (fic_hls_ddr_write(buf_p, tx_size, offset) < 0) {
                        goto WRITE_IO_ERROR;
                    }
                    buf_p  += tx_size;
                    offset += tx_size;
                }
                
                // Read out last 4B and modify then write (RMW)
                u_int8_t r_buf[4];
                if (fic_hls_ddr_read(r_buf, 4, offset) < 0) {
                    goto WRITE_IO_ERROR;
                }
                memcpy(r_buf, buf_p, mis_off_ed);
                if (fic_hls_ddr_write(r_buf, 4, offset) < 0) {
                    goto WRITE_IO_ERROR;
                }

            } else {
                if (fic_hls_ddr_write(buf_p, size_left, offset) < 0) {
                    goto WRITE_IO_ERROR;
                }

            }
        }

        // Close GPIO
        fic_gpio_close(fd_gpio);
    }

     // REG
    if (strcmp(path, "/reg") == 0) {
        if (offset > FIC_MEM_SIZE) {
            return -EFAULT;
        }

        // Open GPIO
        fd_gpio = fic_gpio_open();
        if (fd_gpio < 0) {
            return -EIO;
        }

        fic_comm_reset();

        printf("FICFUSE DEBUG[%d]: WRITE path=%s addr=%08llx size=%zu\n", __LINE__, path, offset, size);

        if (fic_hls_write(buf, size, offset, RASDDR_CMD_AXI_WRITE) < 0) {
            goto WRITE_IO_ERROR;
        }

        // Close GPIO
        fic_gpio_close(fd_gpio);
    }
   
    return size;

WRITE_IO_ERROR:
    fic_gpio_close(fd_gpio);
    return -EIO;

}

static int _open(const char *path, struct fuse_file_info *fi) {
    return 0;
}

static int _truncate(const char *path, off_t size) {
    printf("truncate\n");
    return 0;
}

static int _init_fic() {
    // Open GPIO
    int fd_gpio = fic_gpio_open();
    if (fd_gpio < 0) {
        return -EIO;
    }

    if (fic_comm_reset() < 0) {
        goto INIT_ERROR;
    }

    if (fic_hls_reset() < 0) {
        goto INIT_ERROR;
    }

    if (fic_hls_start() < 0) {
        goto INIT_ERROR;
    }

    fic_gpio_close(fd_gpio);

    printf("FICFUSE DEBUG: Init success\n");
    return 0;

INIT_ERROR:
    fic_gpio_close(fd_gpio);
    return -EIO;
}

//=============================================================================
static const struct fuse_operations fuse_ops = {
    .getattr  = _getattr,
    .readdir  = _readdir,
    .read     = _read,
    .write    = _write,
    .open     = _open,
    .truncate = _truncate,
};

//=============================================================================
int main(int argc, char *argv[]) {

    _init_fic();

    // Call fuse
    fuse_main(argc, argv, &fuse_ops, NULL);
    return 0;
}
//=============================================================================
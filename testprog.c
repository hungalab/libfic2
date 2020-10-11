#include "ficlib2.h"
#include <time.h>

//-----------------------------------------------------------------------------
// TEST CODE
//-----------------------------------------------------------------------------
#define BITFILE "mk2_fic_ddrtest.bin"

void test_fpga_prog() {
    int fd;
    struct stat st;
    stat(BITFILE, &st);
    size_t size = st.st_size;
    uint8_t *buf = malloc(sizeof(char)*size);

    fd = open(BITFILE, O_RDONLY);
    read(fd, buf, size);

    printf("TEST for FPGA configuration\n");
    size_t tx = fic_prog_sm16(buf, size, PM_NORMAL);
//    size_t tx = fic_prog_sm8(buf, size, PM_NORMAL);
//    size_t tx = fic_prog_sm8_fast(buf, size, PM_NORMAL);
    printf("TEST: %d bytes are transffered\n", tx);

    close(fd);
    free(buf);

}

//-----------------------------------------------------------------------------
int main() {
    int fd = fic_gpio_open();    // Open GPIO
    printf("DEBUG: gpio fd %d \n", fd);
    test_fpga_prog();
    fic_gpio_close(fd);   // Close GPIO

}
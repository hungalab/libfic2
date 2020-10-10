#include "ficlib2.h"
#include <time.h>

//-----------------------------------------------------------------------------
// TEST CODE
//-----------------------------------------------------------------------------
#define BITFILE "fic_ddrtest.bin"

void test_fpga_prog() {
    int fd;
    struct stat st;
    stat(BITFILE, &st);
    size_t size = st.st_size;
    uint8_t *buf = malloc(sizeof(char)*size);

    fd = open(BITFILE, O_RDONLY);
    read(fd, buf, size);

    printf("TEST for FPGA configuration\n");
//    size_t tx = fic_prog_sm16(buf, size, PM_NORMAL);
    size_t tx = fic_prog_sm8(buf, size, PM_NORMAL);
//    size_t tx = fic_prog_sm8_fast(buf, size, PM_NORMAL);
    printf("TEST: %d bytes are transffered\n", tx);

    close(fd);
    free(buf);

}

//-----------------------------------------------------------------------------
void test_ddrrw() {
    int i, j;
    uint32_t addr   = 0x00000000;
    //uint8_t  data[] = {0xde, 0xad, 0xbe, 0xef};
    //uint8_t  data[] = {0xca, 0xfe, 0xba, 0xbe};
    uint8_t  buf[256] = {0};
    int ret = 0;

    clock_t t1, t2;

    // Load test data file
    FILE *fp_input;
    if ((fp_input = fopen("1G", "rb")) == NULL) {
        printf("Can not open test file\n");
        exit(EXIT_FAILURE);
    }

    uint32_t test_size = 1024*1024*1024;    // 1G
    uint32_t block_size = 1024*1024*100;    // 100M

    uint8_t *mem = malloc(sizeof(uint8_t) * block_size);
    if (!mem) {
        printf("Malloc failed\n");
        exit(EXIT_FAILURE);
    }

    fic_comm_reset();   // Reset FSM
    fic_hls_reset();
    fic_hls_start();

    for (i = 0; i < test_size; i+=block_size) {

        printf("Load next block... ");
        fread(mem, block_size, 1, fp_input);
        printf("OK\n");

        printf("HLS RESET.. ");
        fic_hls_reset();
        fic_hls_start();
        printf("OK\n");

        printf("DDR memory write test:\n");
        t1 = clock();
        ret = fic_hls_ddr_write(mem, block_size, addr+i);
        t2 = clock();

        printf("Transfer rate = %f KB/s\n",
            block_size / ((double)(t2 - t1) / CLOCKS_PER_SEC) / 1024);

        if (ret < 0) {
            printf("[libfic2][DEBUG]: ERROR at fic_ddr_write\n");
        }

        printf("DEBUG: Total write=%d\n", i+block_size);
    }

    fclose(fp_input);

    // Read test --------------------------------------------------------------
    FILE *fp_output;
    if ((fp_output = fopen("test.out", "wb")) == NULL) {
        printf("Can not open test output file\n");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < test_size; i+=block_size) {

        printf("HLS RESET.. ");
        fic_hls_reset();
        fic_hls_start();
        printf("OK\n");

        printf("TEST DDR memory read:\n");
        t1 = clock();
        ret = fic_hls_ddr_read(mem, block_size, addr+i);
        t2 = clock();

        printf("Transfer rate = %f KB/s\n",
            block_size / ((double)(t2 - t1) / CLOCKS_PER_SEC) / 1024);

        if (ret < 0) {
            printf("[libfic2][DEBUG]: ERROR at fic_ddr_read\n");
        }

        fwrite(mem, block_size, 1, fp_output);

    }

    fclose(fp_output);

    free(mem);
}

//-----------------------------------------------------------------------------
int main() {
    int fd = fic_gpio_open();    // Open GPIO
    printf("DEBUG: gpio fd %d \n", fd);

//    test_fpga_prog();
    test_ddrrw();

    fic_gpio_close(fd);   // Close GPIO

}
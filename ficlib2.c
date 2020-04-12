#include "ficlib2.h"

//-----------------------------------------------------------------------------
struct _prog_async_status PROG_ASYNC_STATUS = {
    .stat         = PM_STAT_INIT,
    .smap_mode    = PM_SMAP_8,
    .prog_mode    = PM_NORMAL,
    .prog_st_time = 0,
    .prog_ed_time = 0,
    .prog_size    = 0,
    .tx_size      = 0,
};

//-----------------------------------------------------------------------------
// GPIO operation (with GPIO check)
//-----------------------------------------------------------------------------
static inline int fic_set_gpio_fast(uint32_t set) {
    SET_GPIO = set;
    while ((GET_GPIO & set) ^ set) asm("nop");
}

static inline int fic_clr_gpio_fast(uint32_t set) {
    CLR_GPIO = set;
    while (GET_GPIO & set) asm("nop");
}

//-----------------------------------------------------------------------------
// GPIO operation (with GPIO check)
//-----------------------------------------------------------------------------
static inline int fic_set_gpio(uint32_t set) {
    time_t t1, t2;
    SET_GPIO = set;
    time(&t1);
    while ((GET_GPIO & set) ^ set) {
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
            fprintf(stderr, "[libfic2][ERROR]: Communication timeout at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        //usleep(1);
    }
    return 0;
}

static inline int fic_clr_gpio(uint32_t set) {
    time_t t1, t2;
    CLR_GPIO = set;
    time(&t1);
    while (GET_GPIO & set) {
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
            fprintf(stderr, "[libfic2][ERROR]: Communication timeout at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        //usleep(1);
    }
    return 0;
}

//-----------------------------------------------------------------------------
// Display GPIO status for DEBUG
// Use DEBUGCOM macro for debug
//-----------------------------------------------------------------------------
void fic_comm_busdebug(int line) {
    printf("[libfic2][DEBUG]: L%d GPIO=%x ", line, GET_GPIO);

    if (GET_GPIO & RP_PIN_RREQ) {
        printf("RREQ = 1 ");
    } else {
        printf("RREQ = 0 ");
    }

    if (GET_GPIO & RP_PIN_FREQ) {
        printf("FREQ = 1 ");
    } else {
        printf("FREQ = 0 ");
    }

    if (GET_GPIO & RP_PIN_RSTB) {
        printf("RSTB = 1 ");
    } else {
        printf("RSTB = 0 ");
    }

    if (GET_GPIO & RP_PIN_FACK) {
        printf("FACK = 1 ");
    } else {
        printf("FACK = 0 ");
    }
    printf("\n");
}

//-----------------------------------------------------------------------------
int fic_done() {
    SET_INPUT(RP_DONE);
    if (GET_GPIO & RP_PIN_DONE) {
        return 1;
    }
    return 0;
}

int fic_power() {
    SET_INPUT(RP_PWOK);
    if (GET_GPIO & RP_PIN_PWOK) {
        return 1;
    }
    return 0;
}

//-----------------------------------------------------------------------------
int fic_comm_setup() {
    int i;
    SET_ALL_INPUT;

    for (i = 0; i < GPIO_PIN_MAX; i++) {
        if (i == RP_FACK || i == RP_FREQ ) {
            SET_INPUT(i);
        }
        if (i == RP_RREQ || i == RP_RSTB || (i >= RP_DATA_LOW && i <= RP_DATA_TOP)) {
            SET_INPUT(i);
            SET_OUTPUT(i);
            if (fic_clr_gpio(0x01 << i) < 0) return -1;  // Negate
        }
    }

#ifdef FICMK2
    SET_OUTPUT(RP_CFSEL);
    if (fic_clr_gpio(RP_PIN_CFSEL) < 0) return -1;  // Clear CFG mode
    printf("INFO: RP_CFSEL is clr for FiC Mark2 board\n");
#endif

    return 0;
}

void fic_comm_portdir(enum COMM_PORT_DIR dir) {
    int i;
    if (dir == COMM_PORT_SND) {
        for (i = RP_DATA_LOW; i <= RP_DATA_TOP; i++ ) {
            SET_INPUT(i);
            SET_OUTPUT(i);
        }
    } else if (dir == COMM_PORT_RCV) {
        for (i = RP_DATA_LOW; i <= RP_DATA_TOP; i++ ) {
            SET_INPUT(i);
        }
    }
}

int fic_comm_wait_fack_down() {
    time_t t1, t2;
    time(&t1);
    while (GET_GPIO_PIN(RP_FACK) == 1) {
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
          fprintf(stderr, "[libfic2][ERROR]: Communication timeout at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
          return -1;
        }
    }

    return 0;
}

int fic_comm_wait_fack_up() {
    time_t t1, t2;
    time(&t1);
    while (GET_GPIO_PIN(RP_FACK) == 0) {
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
          fprintf(stderr, "[libfic2][ERROR]: Communication timeout at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
          return -1;
        }
    }

    return 0;
}

int fic_comm_wait_freq_down() {
    time_t t1, t2;
    time(&t1);
    while (GET_GPIO_PIN(RP_FREQ) == 1) {
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
          fprintf(stderr, "[libfic2][ERROR]: Communication timeout at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
          return -1;
        }
    }

    return 0;
}

int fic_comm_wait_freq_up() {
    time_t t1, t2;
    time(&t1);
    while (GET_GPIO_PIN(RP_FREQ) == 0) {
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
          fprintf(stderr, "[libfic2][ERROR]: Communication timeout at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
          return -1;
        }
    }

    return 0;
}

int fic_comm_send(uint32_t bus) {
#ifndef FICMK2
    // Clr RSTB and DATA bus
    if (fic_clr_gpio(RP_PIN_RSTB |
                     RP_PIN_DATA7 | RP_PIN_DATA6 | RP_PIN_DATA5 | RP_PIN_DATA4) < 0) {
        return -1;
    }
#else
    // Clr RSTB and DATA bus
    if (fic_clr_gpio(RP_PIN_RSTB |
                     RP_PIN_DATA0 | RP_PIN_DATA1 | RP_PIN_DATA2 | RP_PIN_DATA3 |
                     RP_PIN_DATA4 | RP_PIN_DATA5 | RP_PIN_DATA6 | RP_PIN_DATA7 |
                     RP_PIN_DATA8 | RP_PIN_DATA9 | RP_PIN_DATA10 | RP_PIN_DATA11 |
                     RP_PIN_DATA12 | RP_PIN_DATA13 | RP_PIN_DATA14 | RP_PIN_DATA15) < 0) {
        return -1;
    }
#endif

    if (fic_set_gpio(RP_PIN_RSTB | bus) < 0) {
        return -1;
    }

    if (fic_comm_wait_fack_up() < 0) {
        fprintf(stderr, "[libfic2][ERROR]: fic_comm_wait_fack_up failed at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    if (fic_clr_gpio(RP_PIN_RSTB) < 0) {
        return -1;
    }

    if (fic_comm_wait_fack_down() < 0) {
        fprintf(stderr, "[libfic2][ERROR]: fic_comm_wait_fack_down failed at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

//uint8_t fic_comm_receive() {
int fic_comm_receive() {
    if (fic_clr_gpio(RP_PIN_RSTB) < 0) return -1;
    if (fic_set_gpio(RP_PIN_RSTB) < 0) return -1;

    if (fic_comm_wait_fack_up() < 0) {
        fprintf(stderr, "[libfic2][ERROR]: fic_comm_wait_fack_up failed at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

#ifndef FICMK2
    uint8_t rcv = 0;
    rcv = (GET_GPIO >> RP_DATA_LOW) & 0xff;
#else
    uint16_t rcv = 0;
    rcv = (GET_GPIO >> RP_DATA_LOW) & 0xffff;
#endif

    if (fic_clr_gpio(RP_PIN_RSTB) < 0) return -1;

    if (fic_comm_wait_fack_down() < 0) {
        fprintf(stderr, "[libfic2][ERROR]: fic_comm_wait_fack_down failed at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return (int) rcv;
}

int fic_comm_setaddr(uint16_t addr) {
#ifndef FICMK2
    // Send 16bit high-high address
    if (fic_comm_send(((addr & 0xf000) >> 12) << RP_DATA_LOW) < 0) return -1;

    // Send 16bit high-low address
    if (fic_comm_send(((addr & 0x0f00) >> 8) << RP_DATA_LOW) < 0) return -1;

    // Send 16bit low-high address
    if (fic_comm_send(((addr & 0x00f0) >> 4) << RP_DATA_LOW) < 0) return -1;

    // Send 16bit low-low address
    if (fic_comm_send(((addr & 0x000f) << RP_DATA_LOW)) < 0) return -1;

#else
    // Send 16bit address
    if (fic_comm_send((addr & 0xffff) << RP_DATA_LOW) < 0) return -1;

#endif

    return 0;
}

//-----------------------------------------------------------------------------
// Write a byte via 4bit interface
//-----------------------------------------------------------------------------
int fic_write(uint16_t addr, uint16_t data) {
    if (fic_comm_setup() < 0) return -1;
    fic_comm_portdir(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;

    // Send command
    if (fic_comm_send(COMM_CMD_WRITE << RP_DATA_LOW) < 0) return -1;

    // Set address
    if (fic_comm_setaddr(addr) < 0) return -1;

#ifndef FICMK2
    // Send 4bit high data
    if (fic_comm_send(((data & 0xf0) >> 4) << RP_DATA_LOW) < 0) return -1;

    // Send 4bit low data
    if (fic_comm_send((data & 0x0f) << RP_DATA_LOW) < 0) return -1;

#else
    // Send 16bit data
    if (fic_comm_send(data << RP_DATA_LOW) < 0) return -1;

#endif

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ | RP_PIN_RSTB | COMM_DATABUS_MASK) < 0) return -1;
    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;
 
    return 0;
}

//-----------------------------------------------------------------------------
// Read a byte via 4bit interface
//-----------------------------------------------------------------------------
int fic_read(uint16_t addr) {
    if (fic_comm_setup() < 0) return -1;
    fic_comm_portdir(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;

    // Send command
    if (fic_comm_send(COMM_CMD_READ << RP_DATA_LOW) < 0) return -1;

    // Set address
    if (fic_comm_setaddr(addr) < 0) return -1;

    // Change port direction
    fic_comm_portdir(COMM_PORT_RCV);


    int i = 0;
#ifndef FICMK2
    uint8_t rcv = 0;
    // Receive 4bit high data
    i = fic_comm_receive(); if (i < 0) return -1;
    rcv = (i & 0x0f) << 4;

    i = fic_comm_receive(); if (i < 0) return -1;
    rcv |= i & 0x0f;

#else
    uint16_t rcv = 0;
    // Receive 16bit data
    i = fic_comm_receive(); if (i < 0) return -1;
    rcv = i;

#endif

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ) < 0) return -1;
    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;

    return rcv;
}

//-----------------------------------------------------------------------------
// Transfer bytes via 4bit interface
// Modify 2019.11.30: Make the function send each 1B on *buf via 4bit interface
//-----------------------------------------------------------------------------
int fic_hls_send(uint8_t *data, size_t size) {
    if (fic_comm_setup() < 0) return -1;
    fic_comm_portdir(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;
//    if (fic_comm_wait_freq_up() < 0) return -1;

    // Send command
    if (fic_comm_send(COMM_CMD_WRITE << RP_DATA_LOW) < 0) return -1;

    // Set address at HLS module entry point 0x1000
    if (fic_comm_setaddr(COMM_ADDR_HLS) < 0) return -1;

    size_t i;
    for (i = 0; i < size; i++) {
        // Send 4bit high data
        if (fic_comm_send(((*(data+i) & 0xf0) >> 4) << RP_DATA_LOW) < 0) return -1;
        // Send 4bit low data
        if (fic_comm_send((*(data+i) & 0x0f) << RP_DATA_LOW) < 0) return -1;
    }

    // RREQ dessert
    //if (fic_clr_gpio(RP_PIN_RREQ | RP_PIN_RSTB | COMM_DATABUS_MASK) < 0) return -1;
    if (fic_clr_gpio(RP_PIN_RREQ) < 0) return -1;
    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;
 
    return 0;
}

//-----------------------------------------------------------------------------
// Receive bytes via 4bit interface
// Modify 2019.11.30: Make the function receive each 1B on *buf via 4bit interface
//-----------------------------------------------------------------------------
int fic_hls_receive(uint8_t *buf, size_t size) {
    if (fic_comm_setup() < 0) return -1;
    fic_comm_portdir(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;
 //   if (fic_comm_wait_freq_up() < 0) return -1;

    // Send command
    if (fic_comm_send(COMM_CMD_READ << RP_DATA_LOW) < 0) {
        printf("FAILED at %d\n", __LINE__);
        return -1;
    }

    // Set address
    if (fic_comm_setaddr(COMM_ADDR_HLS) < 0) return -1;

    // Change port direction
    fic_comm_portdir(COMM_PORT_RCV);

    size_t i;
    for (i = 0; i < size; i++) {
        int rcv = 0;
        uint8_t rvh, rvl;

        // Receive Low 4bit
        rcv = fic_comm_receive(); if (rcv < 0) return -1;
        rvh = rcv & 0x0f;

        // Receive High 4bit
        rcv = fic_comm_receive(); if (rcv < 0) return -1;
        rvl = rcv & 0x0f;

        *(buf+i) = (rvh << 4) | rvl;
        //*(buf+i) = rvh;
    }

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ) < 0) return -1;
    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;

    return i;
}

//-----------------------------------------------------------------------------
// Selectmap x16 PIN init
//-----------------------------------------------------------------------------
int fic_prog_init_sm16() {
    int i;
    SET_ALL_INPUT;

    for (i = 0; i <= GPIO_PIN_MAX; i++) {
#ifndef FICMK2
        if (i == RP_PWOK || i == RP_INIT || i == RP_DONE || i == RP_G_CKSEL) {
            SET_INPUT(i);
        }
        if (i == RP_PROG_B || i == RP_CSI_B || i == RP_RDWR_B) {
            SET_INPUT(i);
            SET_OUTPUT(i);
            if (fic_set_gpio(0x01 << i) < 0) return -1; // Disabled
        }
        if (i == RP_CCLK || i == RP_CD0 || i == RP_CD1 || i == RP_CD2 ||
            i == RP_CD3 || i == RP_CD4 || i == RP_CD5 || i == RP_CD6 ||
            i == RP_CD7 || i == RP_CD8 || i == RP_CD9 || i == RP_CD10 ||
            i == RP_CD11 || i == RP_CD12 || i == RP_CD13 || i == RP_CD14 ||
            i == RP_CD15) {
            SET_INPUT(i);
            SET_OUTPUT(i);
            if (fic_clr_gpio(0x01 << i) < 0) return -1; // Negate
        }
#else
        if (i == RP_PWOK || i == RP_INIT || i == RP_DONE) {
            SET_INPUT(i);
        }
        if (i == RP_PROG_B || i == RP_CSI_B || i == RP_RDWR_B || i == RP_CFSEL) {
            SET_INPUT(i);
            SET_OUTPUT(i);
            if (fic_set_gpio(0x01 << i) < 0) return -1; // Set PINS on
        }
        if (i == RP_CCLK || i == RP_CD0 || i == RP_CD1 || i == RP_CD2 ||
            i == RP_CD3 || i == RP_CD4 || i == RP_CD5 || i == RP_CD6 ||
            i == RP_CD7 || i == RP_CD8 || i == RP_CD9 || i == RP_CD10 ||
            i == RP_CD11 || i == RP_CD12 || i == RP_CD13 || i == RP_CD14 ||
            i == RP_CD15) {
            SET_INPUT(i);
            SET_OUTPUT(i);
            if (fic_clr_gpio(0x01 << i) < 0) return -1; // Negate
        }
#endif
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Selectmap x8 PIN init
//-----------------------------------------------------------------------------
int fic_prog_init_sm8() {
    int i;
    SET_ALL_INPUT;

    for (i = 0; i <= GPIO_PIN_MAX; i++) {
#ifndef FICMK2
        if (i == RP_PWOK || i == RP_INIT || i == RP_DONE || i == RP_G_CKSEL) {
            SET_INPUT(i);
        }
        if (i == RP_PROG_B || i == RP_CSI_B || i == RP_RDWR_B ) {
            SET_INPUT(i);
            SET_OUTPUT(i);
            if (fic_set_gpio(0x01 << i) < 0) return -1; // Disabled
        }
        if (i == RP_CCLK || i == RP_CD0 || i == RP_CD1 || i == RP_CD2 ||
            i == RP_CD3 || i == RP_CD4 || i == RP_CD5 || i == RP_CD6 ||
            i == RP_CD7) {
            SET_INPUT(i);
            SET_OUTPUT(i);
            if (fic_clr_gpio(0x01 << i) < 0) return -1; // Negate
        }
#else
        if (i == RP_PWOK || i == RP_INIT || i == RP_DONE) {
            SET_INPUT(i);
        }
        if (i == RP_PROG_B || i == RP_CSI_B || i == RP_RDWR_B || i == RP_CFSEL) {
            SET_INPUT(i);
            SET_OUTPUT(i);
            if (fic_set_gpio(0x01 << i) < 0) return -1; // Set PINS on
        }
        if (i == RP_CCLK || i == RP_CD0 || i == RP_CD1 || i == RP_CD2 ||
            i == RP_CD3 || i == RP_CD4 || i == RP_CD5 || i == RP_CD6 ||
            i == RP_CD7) {
            SET_INPUT(i);
            SET_OUTPUT(i);
            if (fic_clr_gpio(0x01 << i) < 0) return -1; // Negate
        }
#endif
    }

    return 0;
}

//-----------------------------------------------------------------------------
// FPGA Init
//-----------------------------------------------------------------------------
int fic_prog_init(enum PROG_MODE pm) {
    // Pin setup for FPGA Init
    SET_INPUT(RP_PROG_B); SET_OUTPUT(RP_PROG_B);
    SET_INPUT(RP_CSI_B); SET_OUTPUT(RP_CSI_B);
    SET_INPUT(RP_RDWR_B); SET_OUTPUT(RP_RDWR_B);

    // Set disabled pins
    if (fic_set_gpio(RP_PIN_PROG_B | RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) return -1;

#ifdef FICMK2
    SET_OUTPUT(RP_CFSEL);
    if (fic_set_gpio(RP_PIN_CFSEL) < 0) return -1;  // Set CFG mode
    printf("INFO: RP_CFSEL is set for FiC Mark2 board\n");
#endif

    if (pm == PM_PR) {
        // Partial reconfiguration mode
        if (fic_clr_gpio(RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) return -1;  // Assert
        return 0;
    }

    // Do FPGA init sequence
    if (fic_set_gpio(RP_PIN_PROG_B | RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) return -1;
    if (fic_clr_gpio(RP_PIN_PROG_B | RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) return -1;
    if (fic_set_gpio(RP_PIN_PROG_B) < 0) return -1;

    while (GET_GPIO_PIN(RP_INIT) == 0) {
        usleep(1);
    }

    if (GET_GPIO_PIN(RP_DONE) == 1) {
        DEBUGOUT("[libfic2][DEBUG]: FPGA reset failed\n");
        return -1;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Note: all gpio set/hold timing is very empirical for RPi3B
// You need adjust values if you change HW
//-----------------------------------------------------------------------------
static inline void _gpio_hold(uint32_t out) {
    int i;
    for (i = 0; i < 2; i++) SET_GPIO = out;
}

static inline void _toggle_cclk16() {
    int i, j;
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 2; j++) CLR_GPIO = RP_PIN_CCLK;
        for (j = 0; j < 2; j++) SET_GPIO = RP_PIN_CCLK;
    }
}

static inline void _toggle_cclk12() {
    int i, j;
    for (i = 0; i < 12; i++) {
        for (j = 0; j < 2; j++) CLR_GPIO = RP_PIN_CCLK;
        for (j = 0; j < 2; j++) SET_GPIO = RP_PIN_CCLK;
    }
}

static inline void _toggle_cclk8() {
    int i, j;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 2; j++) CLR_GPIO = RP_PIN_CCLK;
        for (j = 0; j < 2; j++) SET_GPIO = RP_PIN_CCLK;
    }
}

static inline void _toggle_cclk4() {
    int i, j;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) CLR_GPIO = RP_PIN_CCLK;
        for (j = 0; j < 2; j++) SET_GPIO = RP_PIN_CCLK;
    }
}

//-----------------------------------------------------------------------------
// Selectmap x16 FPGA configuration 
//-----------------------------------------------------------------------------
void _fic_prog_sm16_async_wrap(void) {
    fic_prog_sm16(PROG_ASYNC_STATUS.prog_data, 
                PROG_ASYNC_STATUS.prog_size,
                PROG_ASYNC_STATUS.prog_mode);

    free(PROG_ASYNC_STATUS.prog_data);
    PROG_ASYNC_STATUS.prog_data = NULL;
}

int fic_prog_sm16_async(uint8_t *data, size_t size, enum PROG_MODE pm) {
    uint8_t *buf = malloc(sizeof(uint8_t)*size);
    if (buf == NULL) {
        PROG_ASYNC_STATUS.stat = PM_STAT_FAIL;
        return -1;
    }
    memcpy(buf, data, size);
    PROG_ASYNC_STATUS.prog_data = buf;
    PROG_ASYNC_STATUS.prog_size = size;
    PROG_ASYNC_STATUS.prog_mode = pm;

    int ret;
    ret = pthread_create(&PROG_ASYNC_STATUS.prog_th, NULL,
                         (void *)_fic_prog_sm16_async_wrap, NULL);

    if (ret != 0) {
        return -1;
    }

    return 0;
}

size_t fic_prog_sm16(uint8_t *data, size_t size, enum PROG_MODE pm) {

    // For profiler metrics
    PROG_ASYNC_STATUS.stat         = PM_STAT_PROG;
    PROG_ASYNC_STATUS.smap_mode    = PM_SMAP_16;
    PROG_ASYNC_STATUS.prog_mode    = pm;
    PROG_ASYNC_STATUS.prog_st_time = clock();
    PROG_ASYNC_STATUS.prog_size    = size;
    PROG_ASYNC_STATUS.tx_size      = 0;

    if (fic_prog_init_sm16() < 0) goto PM_SM16_EXIT_ERROR; // Set pinmode

    // Reset FPGA
    if (fic_prog_init(pm) < 0) goto PM_SM16_EXIT_ERROR;

    // Configure FPGA
    if (fic_clr_gpio(RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
    size_t i;

    time_t t1, t2;
    time(&t1);

    uint32_t _d_hi = 0;     // prev data
    uint32_t _d_low = 0;    // prev data

    for (i = 0; i < size; i+=2) {
        uint32_t d_hi = data[i+1];
        uint32_t d_low = data[i];

        if (d_hi == _d_hi && d_low == _d_low) {
            if ((d_hi == data[i+3] && d_low == data[i+2]) && (d_hi == data[i+5] && d_low == data[i+4]) &&
                (d_hi == data[i+7] && d_low == data[i+6])) {
                if ((d_hi == data[i+9] && d_low == data[i+8]) && (d_hi == data[i+11] && d_low == data[i+10]) &&
                    (d_hi == data[i+13] && d_low == data[i+12]) && (d_hi == data[i+15] && d_low == data[i+14])) {
                        _toggle_cclk8();    // toggle CCLK x8
                        i+=14;

                } else {
                    _toggle_cclk4();    // toggle CCLK x4
                    i+=6;

                }

            } else {
                fic_clr_gpio_fast(RP_PIN_CCLK);
                _gpio_hold(RP_PIN_CCLK);

            }

        } else {
            uint32_t db = (d_hi << 8 | d_low) << 8; // ;data[i+1] << 8 | data[i]) << 8;
            CLR_GPIO = (~db & 0x00ffff00) | RP_PIN_CCLK;
            SET_GPIO = db & 0x00ffff00;
            _gpio_hold((db & 0x00ffff00) | RP_PIN_CCLK);

        }

        _d_hi = d_hi;
        _d_low = d_low;

        PROG_ASYNC_STATUS.tx_size += 2;

#ifdef SHOW_PROGRESS
        // Show progress
        time(&t2);
        if (t2 - t1 > 2) {
            printf("Transfer %d / %d [%.02f %%]\n", i, size, (i/(float)size)*100);
            t1 = t2;
        }
#endif

    }

    // If something happen during send configuration
    if (GET_GPIO_PIN(RP_INIT) == 0) {
        fprintf(stderr,
                "[libfic2][ERROR]: FPGA configuration failed at %s %s %d\n",
                __FILE__, __FUNCTION__, __LINE__);
        goto PM_SM16_EXIT_ERROR;
    }

    // Wait until RP_DONE asserted
    if (pm == PM_NORMAL) {
        DEBUGOUT("[libfic2][DEBUG]: Waiting for RP_DONE\n");
        time(&t1);
        while (GET_GPIO_PIN(RP_DONE) == 0) {
            time(&t2);
            if (GET_GPIO_PIN(RP_INIT) == 0 || t2 - t1 > COMM_TIMEOUT) {
                fprintf(stderr,
                        "[libfic2][ERROR]: FPGA configuration failed at %s %s %d\n",
                        __FILE__, __FUNCTION__, __LINE__);
                goto PM_SM16_EXIT_ERROR;
            }
            if (fic_set_gpio(RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
            if (fic_clr_gpio(RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
        }
        DEBUGOUT("[libfic2][DEBUG]: RP_DONE\n");
        if (fic_clr_gpio(0x00ffff00 | RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
    }

    SET_ALL_INPUT;
    PROG_ASYNC_STATUS.stat         = PM_STAT_DONE;
    PROG_ASYNC_STATUS.prog_ed_time = clock();

    return i;

PM_SM16_EXIT_ERROR:
    SET_ALL_INPUT;
    PROG_ASYNC_STATUS.stat         = PM_STAT_FAIL;
    PROG_ASYNC_STATUS.prog_ed_time = clock();

    return 0;
}

//-----------------------------------------------------------------------------
// Selectmap x8 FPGA configuration 
//-----------------------------------------------------------------------------
void _fic_prog_sm8_async_wrap(void) {
    fic_prog_sm8(PROG_ASYNC_STATUS.prog_data, 
                PROG_ASYNC_STATUS.prog_size,
                PROG_ASYNC_STATUS.prog_mode);

    free(PROG_ASYNC_STATUS.prog_data);
    PROG_ASYNC_STATUS.prog_data = NULL;
}

int fic_prog_sm8_async(uint8_t *data, size_t size, enum PROG_MODE pm) {
    uint8_t *buf = malloc(sizeof(uint8_t)*size);
    if (buf == NULL) {
        PROG_ASYNC_STATUS.stat = PM_STAT_FAIL;
        return -1;
    }
    memcpy(buf, data, size);
    PROG_ASYNC_STATUS.prog_data = buf;
    PROG_ASYNC_STATUS.prog_size = size;
    PROG_ASYNC_STATUS.prog_mode = pm;

    int ret;
    ret = pthread_create(&PROG_ASYNC_STATUS.prog_th, NULL,
                         (void *)_fic_prog_sm8_async_wrap, NULL);

    if (ret != 0) {
        return -1;
    }

    return 0;
}


//-----------------------------------------------------------------------------
size_t fic_prog_sm8(uint8_t *data, size_t size, enum PROG_MODE pm) {

    // For profiler metrics
    PROG_ASYNC_STATUS.stat         = PM_STAT_PROG;
    PROG_ASYNC_STATUS.smap_mode    = PM_SMAP_8;
    PROG_ASYNC_STATUS.prog_mode    = pm;
    PROG_ASYNC_STATUS.prog_st_time = clock();
    PROG_ASYNC_STATUS.prog_size    = size;
    PROG_ASYNC_STATUS.tx_size      = 0;

    if (fic_prog_init_sm8() < 0) goto PM_SM8_EXIT_ERROR;  // Set pinmode

    // Reset FPGA
    if (fic_prog_init(pm) < 0) goto PM_SM8_EXIT_ERROR;

    // Configure FPGA
    if (fic_clr_gpio(RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
    size_t i;

    time_t t1, t2;
    time(&t1);

    uint32_t _d = 0;    // prev data
    for (i = 0; i < size; i++) {
        uint32_t hold_gpio = 0;
        uint32_t d = data[i];
        if (d == _d) {
            if ((d == data[i+1]) && (d == data[i+2]) && (d == data[i+3])) {
                if ((d == data[i+4]) && (d == data[i+5]) && (d == data[i+6]) && (d == data[i+7])) {
                    if ((d == data[i+8]) && (d == data[i+9]) && (d == data[i+10]) && (d == data[i+11])) {
                        if ((d == data[i+12]) && (d == data[i+13]) && (d == data[i+14]) && (d == data[i+15])) {
                            _toggle_cclk16(); // toggle CCLK x16
                            i+=15;

                        } else {
                            _toggle_cclk12(); // toggle CCLK x12
                            i+=11;
                        }

                    } else {
                        _toggle_cclk8(); // toggle CCLK x8
                        i+=7;
                    }

                } else {
                    _toggle_cclk4(); // toggle CCLK x4
                    i+=3;
                }

            } else {
                fic_clr_gpio_fast(RP_PIN_CCLK); // Securely CCLK down
                _gpio_hold(RP_PIN_CCLK);

            }

        } else {
            uint32_t db = (d << 8);
            CLR_GPIO = (~db & 0x0000ff00) | RP_PIN_CCLK;
            SET_GPIO = db & 0x0000ff00;
            _gpio_hold((db & 0x0000ff00) | RP_PIN_CCLK);

        }

        _d = d;

        PROG_ASYNC_STATUS.tx_size++;

#ifdef SHOW_PROGRESS
        // Show progress
        time(&t2);
        if (t2 - t1 > 2) {
            printf("Transfer %d / %d [%.02f %%]\n", i, size, (i/(float)size)*100);
            t1 = t2;
        }
#endif

    }

    // If something happen during send configuration
    if (GET_GPIO_PIN(RP_INIT) == 0) {
        fprintf(stderr,
                "[libfic2][ERROR]: FPGA configuration failed at %s %s %d\n",
                __FILE__, __FUNCTION__, __LINE__);
        goto PM_SM8_EXIT_ERROR;
    }

    // Waitng RP_DONE asserted
    if (pm == PM_NORMAL) {
        DEBUGOUT("[libfic2][DEBUG]: Waiting for RP_DONE\n");
        time(&t1);
        while (GET_GPIO_PIN(RP_DONE) == 0) {
            time(&t2);
            if (GET_GPIO_PIN(RP_INIT) == 0 || t2 - t1 > COMM_TIMEOUT) {
                fprintf(stderr,
                        "[libfic2][ERROR]: FPGA configuration failed at %s %s %d\n",
                        __FILE__, __FUNCTION__, __LINE__);
                goto PM_SM8_EXIT_ERROR;
            }
            if (fic_set_gpio(RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
            if (fic_clr_gpio(RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
        }
        DEBUGOUT("[libfic2][DEBUG]: RP_DONE\n");
        if (fic_clr_gpio(0x0000ff00 | RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
    }

    SET_ALL_INPUT;
    PROG_ASYNC_STATUS.stat         = PM_STAT_DONE;
    PROG_ASYNC_STATUS.prog_ed_time = clock();

    return i;

PM_SM8_EXIT_ERROR:
    SET_ALL_INPUT;
    PROG_ASYNC_STATUS.stat         = PM_STAT_FAIL;
    PROG_ASYNC_STATUS.prog_ed_time = clock();

    return 0;
}

//-----------------------------------------------------------------------------
int fic_hls_start() {
    if (fic_comm_setup() < 0) return -1;
    fic_comm_portdir(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;

    // Send command
    if (fic_comm_send(COMM_CMD_HLS_START << RP_DATA_LOW) < 0) return -1;

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ) < 0) return -1;

    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;

    return 0;
}

int fic_hls_reset() {
    if (fic_comm_setup() < 0) return -1;
    fic_comm_portdir(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;

    // Send command
    if (fic_comm_send(COMM_CMD_HLS_RESET << RP_DATA_LOW) < 0) return -1;

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ) < 0) return -1;

    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;

    return 0;
}

//-----------------------------------------------------------------------------
// Create LOCK_FILE
//-----------------------------------------------------------------------------
int gpio_lock() {
    printf("[libfic2][DEBUG]: Obtaining GPIO lock...\n");

    // Check LOCKFILE
    time_t t1, t2;
    time(&t1);
    // Wait while lockfile is existed
    struct stat st;
    while (stat(LOCK_FILE, &st) == 0) {
        sleep(1);
        printf("[libfic2][DEBUG]: Attempting obtaining GPIO lock...\n");
        time(&t2);
        if ((t2 - t1) > GPIO_LOCK_TIMEOUT) {
            fprintf(stderr, "[libfic2][ERROR]: GPIO lock timeout\n");
            return -1;
        }
    }

    // Create LOCKFILE
    int lock_fd = open(LOCK_FILE, O_CREAT|O_RDONLY, 0666);
    if (lock_fd < 0) {
		fprintf(stderr, "[libfic2][ERROR]: Cant create LOCK_FILE\n");
        return -1;
    }

    // flock LOCKFILE
    if (flock(lock_fd, LOCK_EX) < 0) {
		fprintf(stderr, "[libfic2][ERROR]: Cant lock LOCK_FILE\n");
        return -1;
    }

    return lock_fd;
}

//-----------------------------------------------------------------------------
// Delete LOCKFILE
//-----------------------------------------------------------------------------
int gpio_unlock(int fd_lock) {
    close(fd_lock);                 // Close lockfile fd
    if (unlink(LOCK_FILE) < 0 ) {   // Delete lockfile
		fprintf(stderr, "[libfic2][ERROR]: Cant remove LOCK_FILE\n");
        return -1;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Set up a memory regions to access GPIO
//-----------------------------------------------------------------------------
int fic_gpio_open() {
    int fd_lock = gpio_lock();
    if (fd_lock < 0) {
		fprintf(stderr, "[libfic2][ERROR]: GPIO open failed\n");
        return -1;
    }

	/* open GPIO_DEV */
    int mem_fd = open(GPIO_DEV, O_RDWR|O_SYNC);
	if (mem_fd < 0) {
		fprintf(stderr, "[libfic2][ERROR]: can't open %s \n", GPIO_DEV);
        return -1;
	}

	/* mmap GPIO */
	void *gpio_map = NULL;
    gpio_map = mmap(
        NULL,                   //Any adddress in our space will do
        BLOCK_SIZE,             //Map length
        PROT_READ | PROT_WRITE, // Enable reading & writting to mapped memory
        MAP_SHARED,             //Shared with other processes
        mem_fd,                 //File to map
        GPIO_BASE               //Offset to GPIO peripheral
    );

    close(mem_fd); //No need to keep mem_fd open after mmap

	if (gpio_map == MAP_FAILED) {
		fprintf(stderr, "[libfic2][ERROR]: mmap error %d\n", (int)gpio_map);//errno also set!
        return -1;
	}

	// Always use volatile pointer!
	gpio = (volatile unsigned *)gpio_map;

    return fd_lock;
} 

//-----------------------------------------------------------------------------
int fic_gpio_close(int fd_lock) {
    if (munmap((void *)gpio, BLOCK_SIZE) < 0) {
        return -1;
    }

    if (gpio_unlock(fd_lock) < 0) {
        return -1;
    }

    return 0;
}

#ifndef SHARED_LIB
//-----------------------------------------------------------------------------
// TEST CODE
//-----------------------------------------------------------------------------
//#define BITFILE "ring_8bit.bin"
//#define BITFILE "ring_4bit.bin"
//#define BITFILE "ring_akram.bin"
//#define BITFILE "RPBT115.bin"
//#define BITFILE "AURORA.bin"
#define BITFILE "fic_top.bin"
//#define BITFILE "mk2_fic_top.bin"

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
    printf("TEST: %d bytes are transffered\n", tx);

    close(fd);
    free(buf);

}

void test_fpga_async_prog() {
    int fd;
    struct stat st;
    stat(BITFILE, &st);
    size_t size = st.st_size;
    uint8_t *buf = malloc(sizeof(char)*size);

    fd = open(BITFILE, O_RDONLY);
    read(fd, buf, size);
    close(fd);

    printf("TEST for FPGA configuration\n");
//    size_t tx = fic_prog_sm16(buf, size, PM_NORMAL, NULL);
    fic_prog_sm8_async(buf, size, PM_NORMAL);
    free(buf);

    while(PROG_ASYNC_STATUS.stat != PM_STAT_DONE) {
        printf("DEBUG: %d\n", PROG_ASYNC_STATUS.tx_size);
        sleep(1);
    }

    pthread_join(PROG_ASYNC_STATUS.prog_th, NULL);

}

void test_rw() {
    uint16_t addr = 0xffff;
    uint16_t data = 'B';
    int ret = 0;

    printf("TEST write:\n");
    ret = fic_write(addr, data);
    if (ret < 0) {
        printf("[libfic2][DEBUG]: ERROR at fic_write\n");
    }

    printf("TEST read:\n");
    ret = fic_read(addr);
    if (ret < 0) {
        printf("[libfic2][DEBUG]: ERROR at fic_read\n");
    }
    uint8_t rcv = ret;
    printf("TEST RW write=%x, read=%x result=%s\n", data, rcv, data == rcv ? "PASS" : "FAIL");

    printf("TEST read:\n");
    ret = fic_read(0xfffa);
    if (ret < 0) {
        printf("[libfic2][DEBUG]: ERROR at fic_read\n");
    }
    printf("TEST read=%x\n", ret);

//    // HLS logic reset
//    printf("HLS module reset\n");
//    fic_hls_reset();
//    printf("HLS module start\n");
//    fic_hls_start();
//
//    // Multi byte transfer
//    addr = 0xffff;
//    addr = 0x1000;
//    uint8_t list = 0x02;
//    uint8_t buf[16] = {0};
//
//    printf("HLS module write at %x\n", addr);
//    fic_hls_send(&list, 1);
//
//    printf("HLS module read at %x\n", addr);
//    fic_hls_receive(16, buf);
//    printf("[libfic2][DEBUG]: read=");
//    for (int i = 0; i < 16; i++) {
//        printf("%x ", buf[i]);
//    }
//    printf("\n");

}
//-----------------------------------------------------------------------------
int main() {
    int fd = fic_gpio_open();    // Open GPIO
    printf("DEBUG: gpio fd %d \n", fd);
    test_fpga_async_prog();
//    test_fpga_prog();
//    test_rw();
    fic_gpio_close(fd);   // Close GPIO

}

#endif
//-----------------------------------------------------------------------------

#include "ficlib2.h"

//-----------------------------------------------------------------------------
// GPIO operation (with GPIO check)
//-----------------------------------------------------------------------------
int inline fic_set_gpio(uint32_t set) {
    time_t t1, t2;
    time(&t1);
    SET_GPIO = set;
    while ((GET_GPIO & set) != set) {
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
            fprintf(stderr, "ERROR: Communication timeout at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        usleep(1);
    }
    return 0;
}

int inline fic_clr_gpio(uint32_t set) {
    time_t t1, t2;
    time(&t1);
    CLR_GPIO = set;
    while ((GET_GPIO & set) != 0) {
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
            fprintf(stderr, "ERROR: Communication timeout at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
            return -1;
        }
        usleep(1);
    }
    return 0;
}

void inline fic_clr_gpio_debug(uint32_t set) {
    CLR_GPIO = set;
    while ((GET_GPIO & set) != 0) {
        printf("DEBUG GPIO=%x SET=%x RESULT=%x\n", GET_GPIO, set, GET_GPIO & set);
        usleep(1);
    }
}

//-----------------------------------------------------------------------------
// Display GPIO status for DEBUG
// Use DEBUGCOM macro for debug
//-----------------------------------------------------------------------------
void fic_comm_busdebug(int line) {
    printf("DEBUG: L%d GPIO=%x ", line, GET_GPIO);

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
int fic_comm_setup8() {
    int i;
    SET_ALL_INPUT;

    for (i = 0; i < GPIO_PIN_MAX; i++) {
        if (i == RP_FACK || i == RP_FREQ ) {
            SET_INPUT(i);
        }
        if (i == RP_RREQ || i == RP_RSTB 
        || i == RP_DATA0 || i == RP_DATA1 || i == RP_DATA2 || i == RP_DATA3
        || i == RP_DATA4 || i == RP_DATA5 || i == RP_DATA6 || i == RP_DATA7 ) {
            SET_INPUT(i);
            SET_OUTPUT(i);
            if (fic_clr_gpio(0x01 << i) < 0) return -1; // Negate
        }
    }

    return 0;
}

int fic_comm_setup4() {
    int i;
    SET_ALL_INPUT;

    for (i = 0; i < GPIO_PIN_MAX; i++) {
        if (i == RP_FACK || i == RP_FREQ ) {
            SET_INPUT(i);
        }
        if (i == RP_RREQ || i == RP_RSTB 
        || i == RP_DATA4 || i == RP_DATA5 || i == RP_DATA6 || i == RP_DATA7 ) {
            SET_INPUT(i);
            SET_OUTPUT(i);
            if (fic_clr_gpio(0x01 << i) < 0) return -1;  // Negate
        }
    }

    return 0;
}

void fic_comm_portdir8(enum COMM_PORT_DIR dir) {
    int i;
    if (dir == COMM_PORT_SND) {
        for (i = RP_DATA0; i <= RP_DATA7; i++ ) {
            SET_INPUT(i);
            SET_OUTPUT(i);
        }
    } else if (dir == COMM_PORT_RCV) {
        for (i = RP_DATA0; i <= RP_DATA7; i++ ) {
            SET_INPUT(i);
        }
    }
}

void fic_comm_portdir4(enum COMM_PORT_DIR dir) {
    int i;
    if (dir == COMM_PORT_SND) {
        for (i = RP_DATA4; i <= RP_DATA7; i++ ) {
            SET_INPUT(i);
            SET_OUTPUT(i);
        }
    } else if (dir == COMM_PORT_RCV) {
        for (i = RP_DATA4; i <= RP_DATA7; i++ ) {
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
          fprintf(stderr, "ERROR: Communication timeout at %s %s %d\n",
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
          fprintf(stderr, "ERROR: Communication timeout at %s %s %d\n",
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
          fprintf(stderr, "ERROR: Communication timeout at %s %s %d\n",
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
//        DEBUGCOMM;
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
          fprintf(stderr, "ERROR: Communication timeout at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
          return -1;
        }
    }

    return 0;
}

int fic_comm_send8(uint32_t bus) {
    // Clr RSTB and DATA bus
    if (fic_clr_gpio(RP_PIN_RSTB |
        RP_PIN_DATA7 | RP_PIN_DATA6 | RP_PIN_DATA5 | RP_PIN_DATA4 |
        RP_PIN_DATA3 | RP_PIN_DATA2 | RP_PIN_DATA1 | RP_PIN_DATA0) < 0) {
            return -1;
        }

    if (fic_set_gpio(RP_PIN_RSTB | bus) < 0) {
        return -1;
    }

    if (fic_comm_wait_fack_up() < 0) {
        fprintf(stderr, "ERROR: fic_comm_wait_fack_up failed at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    if (fic_clr_gpio(RP_PIN_RSTB) < 0) {
        return -1;
    }

    if (fic_comm_wait_fack_down() < 0) {
        fprintf(stderr, "ERROR: fic_comm_wait_fack_down failed at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

int fic_comm_send4(uint32_t bus) {
    // Clr RSTB and DATA bus
    if (fic_clr_gpio(RP_PIN_RSTB |
        RP_PIN_DATA7 | RP_PIN_DATA6 | RP_PIN_DATA5 | RP_PIN_DATA4) < 0) {
            return -1;
        }

    if (fic_set_gpio(RP_PIN_RSTB | bus) < 0) {
        return -1;
    }

    if (fic_comm_wait_fack_up() < 0) {
        fprintf(stderr, "ERROR: fic_comm_wait_fack_up failed at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    if (fic_clr_gpio(RP_PIN_RSTB) < 0) {
        return -1;
    }

    if (fic_comm_wait_fack_down() < 0) {
        fprintf(stderr, "ERROR: fic_comm_wait_fack_down failed at %s %s %d\n",
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
        fprintf(stderr, "ERROR: fic_comm_wait_fack_up failed at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    uint8_t rcv = 0;
    rcv = (GET_GPIO >> RP_DATA0) & 0xff;

    if (fic_clr_gpio(RP_PIN_RSTB) < 0) return -1;

    if (fic_comm_wait_fack_down() < 0) {
        fprintf(stderr, "ERROR: fic_comm_wait_fack_down failed at %s %s %d\n",
                  __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }

    return (int) rcv;
}

int fic_comm_setaddr8(uint16_t addr) {
    // Send 16bit high address
    if (fic_comm_send8(((addr & 0xff00) >> 8) << RP_DATA0) < 0) return -1;
    if (fic_comm_send8(((addr & 0x00ff) << RP_DATA0)) < 0) return -1;

    return 0;
}

int fic_comm_setaddr4(uint16_t addr) {
    // Send 16bit high-high address
    if (fic_comm_send4(((addr & 0xf000) >> 12) << RP_DATA4) < 0) return -1;

    // Send 16bit high-low address
    if (fic_comm_send4(((addr & 0x0f00) >> 8) << RP_DATA4) < 0) return -1;

    // Send 16bit low-high address
    if (fic_comm_send4(((addr & 0x00f0) >> 4) << RP_DATA4) < 0) return -1;

    // Send 16bit low-low address
    if (fic_comm_send4(((addr & 0x000f) << RP_DATA4)) < 0) return -1;

    return 0;
}
//-----------------------------------------------------------------------------
// Write a byte via 8bit interface
//-----------------------------------------------------------------------------
int fic_wb8(uint16_t addr, uint8_t data) {
    if (fic_comm_setup8() < 0) return -1;
    fic_comm_portdir8(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;
//    if (fic_comm_wait_freq_up() < 0) return -1;

    // Send command
    if (fic_comm_send8(COMM_CMD_WRITE << RP_DATA0) < 0) return -1;

    // Set address
    if (fic_comm_setaddr8(addr) < 0) return -1;

    // Send 8bit data
    if (fic_comm_send8(data << RP_DATA0) < 0) return -1;

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ | RP_PIN_RSTB | COMM_DATABUS_MASK) < 0) return -1;

    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;
 
    return 0;
}

//-----------------------------------------------------------------------------
// Read a byte via 8bit interface
//-----------------------------------------------------------------------------
int fic_rb8(uint16_t addr) {
    if (fic_comm_setup8() < 0) return -1;
    fic_comm_portdir8(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;
//    if (fic_comm_wait_freq_up() < 0) return -1;

    // Send command
    if (fic_comm_send8(COMM_CMD_READ << RP_DATA0) < 0) return -1;

    // Set address
    if (fic_comm_setaddr8(addr) < 0) return -1;

    // Change port direction
    fic_comm_portdir8(COMM_PORT_RCV);

    // Receive 8bit data
    //uint8_t rcv = 0;
    int rcv = fic_comm_receive();
    if (rcv < 0) return -1;

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ) < 0) return -1;

    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;

    return (rcv & 0xff);
}

//-----------------------------------------------------------------------------
// Transfer bytes via 8bit interface
//-----------------------------------------------------------------------------
//int fic_write8(uint16_t addr, uint8_t *data, size_t size) {
//    fic_comm_setup8();
//    fic_comm_portdir8(COMM_PORT_SND);
//
//    // RREQ assert
//    fic_set_gpio(RP_PIN_RREQ);
////    if (fic_comm_wait_freq_up() < 0) {
////        return -1;
////    }
//
//    // Send command
//    if (fic_comm_send8(COMM_CMD_WRITE << RP_DATA0) < 0) {
//        return -1;
//    }
//
//    // Set address
//    fic_comm_setaddr8(addr);
//
//    size_t i;
//    for (i = 0; i < size; i++) {
//        // Send 8bit data
//        printf("data=%x\n", *(data+i));
//        if (fic_comm_send8(*(data+i) << RP_DATA0) < 0) {
//            return -1;
//        }
//    }
//
//    // RREQ dessert
//    fic_clr_gpio(RP_PIN_RREQ | RP_PIN_RSTB | COMM_DATABUS_MASK);
//
//    SET_ALL_INPUT;
//    if (fic_comm_wait_freq_down() < 0) {
//        return -1;
//    }
// 
//    return 0;
//}
//
////-----------------------------------------------------------------------------
//// Receive bytes via 4bit interface
////-----------------------------------------------------------------------------
//int fic_read8(uint16_t addr, size_t size, uint8_t *buf) {
//    fic_comm_setup8();
//    fic_comm_portdir8(COMM_PORT_SND);
//
//    // RREQ assert
//    fic_set_gpio(RP_PIN_RREQ);
////    if (fic_comm_wait_freq_up() < 0) {
////        return -1;
////    }
//
//    // Send command
//    if (fic_comm_send8(COMM_CMD_READ << RP_DATA0) < 0) {
//        return -1;
//    }
//
//    // Set address
//    fic_comm_setaddr8(addr);
//
//    // Change port direction
//    fic_comm_portdir8(COMM_PORT_RCV);
//
//    size_t i;
//    for (i = 0; i < size; i++) {
//        // Receive 8bit data
//        *(buf+i) = fic_comm_receive();
//    }
//
//    // RREQ dessert
//    fic_clr_gpio(RP_PIN_RREQ);
//    if (fic_comm_wait_freq_down() < 0) {
//        return -1;
//    }
//
//    return i;
//}

//-----------------------------------------------------------------------------
// Write a byte via 4bit interface
//-----------------------------------------------------------------------------
int fic_wb4(uint16_t addr, uint8_t data) {
    if (fic_comm_setup4() < 0) return -1;
    fic_comm_portdir4(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;
//    if (fic_comm_wait_freq_up() < 0) return -1;

    // Send command
    if (fic_comm_send4(COMM_CMD_WRITE << RP_DATA4) < 0) return -1;

    // Set address
    if (fic_comm_setaddr4(addr) < 0) return -1;

    // Send 4bit high data
    if (fic_comm_send4(((data & 0xf0) >> 4) << RP_DATA4) < 0) return -1;

    // Send 4bit low data
    if (fic_comm_send4((data & 0x0f) << RP_DATA4) < 0) return -1;

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ | RP_PIN_RSTB | COMM_DATABUS_MASK) < 0) return -1;
    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;
 
    return 0;
}

//-----------------------------------------------------------------------------
// Read a byte via 4bit interface
//-----------------------------------------------------------------------------
int fic_rb4(uint16_t addr) {
    if (fic_comm_setup4() < 0) return -1;
    fic_comm_portdir4(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;
 //   if (fic_comm_wait_freq_up() < 0) return -1;

    // Send command
    if (fic_comm_send4(COMM_CMD_READ << RP_DATA4) < 0) return -1;

    // Set address
    if (fic_comm_setaddr4(addr) < 0) return -1;

    // Change port direction
    fic_comm_portdir4(COMM_PORT_RCV);

    // Receive 4bit high data
    int i = 0;
    uint8_t rcv = 0;
    i = fic_comm_receive(); if (i < 0) return -1;
    rcv = i & 0xf0;

    i = fic_comm_receive(); if (i < 0) return -1;
    rcv |= (i >> 4) & 0x0f;

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ) < 0) return -1;
    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;

    return rcv;
}

//-----------------------------------------------------------------------------
// Transfer bytes via 4bit interface
//-----------------------------------------------------------------------------
int fic_hls_send4(uint8_t *data, size_t size) {
    if (fic_comm_setup4() < 0) return -1;
    fic_comm_portdir4(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;
//    if (fic_comm_wait_freq_up() < 0) return -1;

    // Send command
    if (fic_comm_send4(COMM_CMD_WRITE << RP_DATA4) < 0) return -1;

    // Set address at HLS module entry point 0x1000
    if (fic_comm_setaddr4(COMM_ADDR_HLS) < 0) return -1;

    size_t i;
    for (i = 0; i < size; i++) {
        // Send 4bit high data
        if (fic_comm_send4((*(data+i) & 0x0f) << RP_DATA4) < 0) return -1;
    }

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ | RP_PIN_RSTB | COMM_DATABUS_MASK) < 0) return -1;
    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;
 
    return 0;
}

//-----------------------------------------------------------------------------
// Receive bytes via 4bit interface
//-----------------------------------------------------------------------------
int fic_hls_receive4(size_t size, uint8_t *buf) {
    if (fic_comm_setup4() < 0) return -1;
    fic_comm_portdir4(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;
 //   if (fic_comm_wait_freq_up() < 0) return -1;

    // Send command
    if (fic_comm_send4(COMM_CMD_READ << RP_DATA4) < 0) {
        printf("FAILED at %d\n", __LINE__);
        return -1;
    }

    // Set address
    if (fic_comm_setaddr4(COMM_ADDR_HLS) < 0) return -1;

    // Change port direction
    fic_comm_portdir4(COMM_PORT_RCV);

    size_t i;
    for (i = 0; i < size; i++) {
        //uint8_t rcv = 0;
        int rcv = fic_comm_receive(); if (rcv < 0) return -1;
        *(buf+i) = rcv & 0xff;
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
            i == RP_CD7) {
            SET_INPUT(i);
            SET_OUTPUT(i);
            if (fic_clr_gpio(0x01 << i) < 0) return -1; // Negate
        }
    }

    return 0;
}

//-----------------------------------------------------------------------------
// FPGA Init
//-----------------------------------------------------------------------------
int fic_prog_init() {
    if (fic_set_gpio(RP_PIN_PROG_B | RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) return -1;
    if (fic_clr_gpio(RP_PIN_PROG_B | RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) return -1;
    if (fic_set_gpio(RP_PIN_PROG_B) < 0) return -1;

    while (GET_GPIO_PIN(RP_INIT) == 0) {
        usleep(1);
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Selectmap x16 FPGA configuration 
//-----------------------------------------------------------------------------
size_t fic_prog_sm16(uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx_byte) {
    if (fic_prog_init_sm16() < 0) goto PM_SM16_EXIT_ERROR; // Set pinmode

    // Reset FPGA
    if (pm == PM_NORMAL) {
        // Normal mode -> Normal init
        fic_prog_init();

    } else if (pm == PM_PR) {
        // PR mode -> without init
        if (fic_set_gpio(RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) goto PM_SM16_EXIT_ERROR;
        if (fic_clr_gpio(RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) goto PM_SM16_EXIT_ERROR;

    }

    // Configure FPGA
    if (fic_clr_gpio(RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
    size_t i;

    // Transffered byte if specified
    if (tx_byte != NULL) {
        *tx_byte = 0;
    }

    time_t t1, t2;
    time(&t1);

    for (i = 0; i < size; i+=2) {
        uint32_t d = (data[i+1] << 8 | data[i]) << 8;
        if (fic_clr_gpio((~d & 0x00ffff00) | RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
        if (fic_set_gpio(d & 0x00ffff00) < 0) goto PM_SM16_EXIT_ERROR;
        if (fic_set_gpio(RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;

//        DEBUGOUT("DEBUG:%lx %x\n", i, GET_GPIO);
        if (tx_byte != NULL) {
            *tx_byte += 2;
        }

        // Show progress
        time(&t2);
        if (t2 - t1 > 2) {
            printf("Transfer %d / %d [%.02f %%]\n", i, size, (i/(float)size)*100);
            t1 = t2;
        }

        if (GET_GPIO_PIN(RP_INIT) == 0) {
            fprintf(stderr,
                    "ERROR: FPGA configuration failed at %s %s %d\n",
                    __FILE__, __FUNCTION__, __LINE__);
            goto PM_SM16_EXIT_ERROR;
        }
    }

    // Wait until RP_DONE asserted
    if (pm == PM_NORMAL) {
        DEBUGOUT("DEBUG: Waiting for RP_DONE\n");
        while (GET_GPIO_PIN(RP_DONE) == 0) {
            if (GET_GPIO_PIN(RP_INIT) == 0) {
                fprintf(stderr,
                        "ERROR: FPGA configuration failed at %s %s %d\n",
                        __FILE__, __FUNCTION__, __LINE__);
                goto PM_SM16_EXIT_ERROR;
            }
            if (fic_set_gpio(RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
            if (fic_clr_gpio(RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
        }
        DEBUGOUT("DEBUG: RP_DONE\n");
        if (fic_clr_gpio(0x00ffff00 | RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
    }

    SET_ALL_INPUT;
    return i;

PM_SM16_EXIT_ERROR:
    SET_ALL_INPUT;
    return 0;
}

//-----------------------------------------------------------------------------
// Selectmap x8 FPGA configuration 
//-----------------------------------------------------------------------------
size_t fic_prog_sm8(uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx_byte) {
    if (fic_prog_init_sm8() < 0) goto PM_SM8_EXIT_ERROR;  // Set pinmode

    // Reset FPGA
    if (pm == PM_NORMAL) {
        // Normal mode -> Normal init
        if (fic_prog_init() < 0) goto PM_SM8_EXIT_ERROR;

    } else if (pm == PM_PR) {
        // PR mode -> without init
        if (fic_set_gpio(RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) goto PM_SM8_EXIT_ERROR;
        if (fic_clr_gpio(RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) goto PM_SM8_EXIT_ERROR;
    }

    // Configure FPGA
    if (fic_clr_gpio(RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
    size_t i;

    // Transffered byte if specified
    if (tx_byte != NULL) {
        tx_byte = 0;
    }
 
    time_t t1, t2;
    time(&t1);

    for (i = 0; i < size; i++) {
        uint32_t d = (data[i] << 8);
        if (fic_clr_gpio((~d & 0x0000ff00) | RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
        if (fic_set_gpio(d & 0x0000ff00) < 0) goto PM_SM8_EXIT_ERROR;
        if (fic_set_gpio(RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;

        if (tx_byte != NULL) {
            tx_byte++;
        }

        // Show progress
        time(&t2);
        if (t2 - t1 > 2) {
            printf("Transfer %d / %d [%.02f %%]\n", i, size, (i/(float)size)*100);
            t1 = t2;
        }

        if (GET_GPIO_PIN(RP_INIT) == 0) {
            fprintf(stderr,
                    "ERROR: FPGA configuration failed at %s %s %d\n",
                    __FILE__, __FUNCTION__, __LINE__);
            goto PM_SM8_EXIT_ERROR;
        }
    }

    // Waitng RP_DONE asserted
    if (pm == PM_NORMAL) {
        DEBUGOUT("DEBUG: Waiting for RP_DONE\n");
        while (GET_GPIO_PIN(RP_DONE) == 0) {
            if (GET_GPIO_PIN(RP_INIT) == 0) {
                fprintf(stderr,
                        "ERROR: FPGA configuration failed at %s %s %d\n",
                        __FILE__, __FUNCTION__, __LINE__);
                goto PM_SM8_EXIT_ERROR;
            }
            if (fic_set_gpio(RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
            if (fic_clr_gpio(RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
        }
        DEBUGOUT("DEBUG: RP_DONE\n");
        if (fic_clr_gpio(0x0000ff00 | RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
    }

    SET_ALL_INPUT;
    return i;

PM_SM8_EXIT_ERROR:
    SET_ALL_INPUT;
    return 0;
}

//-----------------------------------------------------------------------------
int fic_hls_start8() {
    if (fic_comm_setup8() < 0) return -1;
    fic_comm_portdir8(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;

    // Send command
    if (fic_comm_send8(COMM_CMD_HLS_START << RP_DATA0) < 0) return -1;

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ) < 0) return -1;

    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;

    return 0;
}

int fic_hls_start4() {
    if (fic_comm_setup4() < 0) return -1;
    fic_comm_portdir4(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;

    // Send command
    if (fic_comm_send4(COMM_CMD_HLS_START << RP_DATA4) < 0) return -1;

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ) < 0) return -1;

    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;

    return 0;
}

int fic_hls_reset8() {
    if (fic_comm_setup8() < 0) return -1;
    fic_comm_portdir8(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;

    // Send command
    if (fic_comm_send8(COMM_CMD_HLS_RESET << RP_DATA0) < 0) return -1;

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ) < 0) return -1;

    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;

    return 0;
}

int fic_hls_reset4() {
    if (fic_comm_setup8() < 0) return -1;
    fic_comm_portdir8(COMM_PORT_SND);

    // RREQ assert
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;

    // Send command
    if (fic_comm_send4(COMM_CMD_HLS_RESET << RP_DATA4) < 0) return -1;

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
    // Check LOCKFILE
    time_t t1, t2;
    time(&t1);
    // Wait while lockfile is existed
    struct stat st;
    while (stat(LOCK_FILE, &st) == 0) {
        sleep(1);
        time(&t2);
        if ((t2 - t1) > GPIO_LOCK_TIMEOUT) {
            fprintf(stderr, "ERROR: GPIO lock timeout\n");
            return -1;
        }
    }

    // Create LOCKFILE
    int lock_fd = 0;
    if (lock_fd = open(LOCK_FILE, O_CREAT|O_RDONLY, 0666) < 0) {
		fprintf(stderr, "ERROR: Cant create LOCK_FILE\n");
        return -1;
    }

    // flock LOCKFILE
    if (flock(lock_fd, LOCK_EX) < 0) {
		fprintf(stderr, "ERROR: Cant lock LOCK_FILE\n");
        return -1;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Delete LOCKFILE
//-----------------------------------------------------------------------------
int gpio_unlock() {
    if (unlink(LOCK_FILE) < 0 ) {
		fprintf(stderr, "ERROR: Cant remove LOCK_FILE\n");
        return -1;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Set up a memory regions to access GPIO
//-----------------------------------------------------------------------------
int fic_gpio_open() {
    if (gpio_lock() < 0) {
		fprintf(stderr, "ERROR: GPIO open failed\n");
        return -1;
    }

	/* open GPIO_DEV */
    int mem_fd = 0;
	if ((mem_fd = open(GPIO_DEV, O_RDWR|O_SYNC) ) < 0) {
		fprintf(stderr, "ERROR: can't open %s \n", GPIO_DEV);
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
		fprintf(stderr, "ERROR: mmap error %d\n", (int)gpio_map);//errno also set!
        return -1;
	}

	// Always use volatile pointer!
	gpio = (volatile unsigned *)gpio_map;

    return 0;
} 

//-----------------------------------------------------------------------------
int fic_gpio_close() {
    munmap((void *)gpio, BLOCK_SIZE);
    gpio_unlock();

    return 0;
}

//-----------------------------------------------------------------------------
// TEST CODE
//-----------------------------------------------------------------------------
//#define BITFILE "ring_8bit.bin"
//#define BITFILE "ring_4bit.bin"
#define BITFILE "ring_akram.bin"

void test_fpga_prog() {
    int fd;
    struct stat st;
    stat(BITFILE, &st);
    size_t size = st.st_size;
    uint8_t *buf = malloc(sizeof(char)*size);

    fd = open(BITFILE, O_RDONLY);
    read(fd, buf, size);

    printf("TEST for FPGA configuration\n");
//    size_t tx = fic_prog_sm16(buf, size, PM_NORMAL, NULL);
    size_t tx = fic_prog_sm8(buf, size, PM_NORMAL, NULL);
    printf("TEST: %d bytes are transffered\n", tx);

    close(fd);
    free(buf);

}

void test_rw_4bit() {
    uint16_t addr = 0xffff;
    uint8_t data = 'B';
    int ret = 0;

    printf("TEST write:\n");
    ret = fic_wb4(addr, data);
    if (ret < 0) {
        printf("DEBUG: ERROR at fic_wb4\n");
    }

    printf("TEST read:\n");
    ret = fic_rb4(addr);
    if (ret < 0) {
        printf("DEBUG: ERROR at fic_rb4\n");
    }
    uint8_t rcv = ret;
    printf("TEST RW via 4bit IF write=%x, read=%x result=%s\n", data, rcv, data == rcv ? "PASS" : "FAIL");

//    // HLS logic reset
//    printf("HLS module reset\n");
//    fic_hls_reset4();
//    printf("HLS module start\n");
//    fic_hls_start4();
//
//    // Multi byte transfer
////    addr = 0xffff;
//    addr = 0x1000;
//    uint8_t list[] = "TEST\0";
//    uint8_t buf[8] = {0};
//
//    printf("HLS module write at %x\n", addr);
//    fic_hls_send4(list, 1);
//    printf("HLS module read at %x\n", addr);
//    fic_hls_receive4(1, buf);
////    printf("DEBUG: read=%s\n", buf);

}

void test_rw_8bit() {
    uint16_t addr = 0xffff;
    uint8_t data = 'c';
    fic_wb8(addr, data);
    uint8_t rcv = fic_rb8(addr);

    printf("TEST RW via 8bit IF write=%x, read=%x result=%s\n", data, rcv, data == rcv ? "PASS" : "FAIL");

//    // Multi byte transfer
//    addr = 0xffff;
//    uint8_t list[] = "aaaa\0";
//    uint8_t buf[8] = {0};
//
//    fic_write8(addr, list, 1);
//    rcv = fic_rb8(addr);
//    printf("DEBUG: rcv=%x\n", rcv);
//    fic_read8(addr, 1, buf);
//    printf("DEBUG: read=%s\n", buf);

}
//-----------------------------------------------------------------------------

int main() {
    fic_gpio_open();    // Open GPIO

    test_fpga_prog();
//    test_rw_8bit();
    test_rw_4bit();

    fic_gpio_close();   // Close GPIO

}
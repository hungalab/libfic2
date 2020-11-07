#include "ficlib2.h"

volatile unsigned *gpio;
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
            PErr("Communication timeout");
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
            PErr("Communication timeout");
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
    PDebug("L%d GPIO=%x ", line, GET_GPIO);

    if (GET_GPIO & RP_PIN_RREQ) {
        PDebug("RREQ = 1 ");
    } else {
        PDebug("RREQ = 0 ");
    }

    if (GET_GPIO & RP_PIN_FREQ) {
        PDebug("FREQ = 1 ");
    } else {
        PDebug("FREQ = 0 ");
    }

    if (GET_GPIO & RP_PIN_RSTB) {
        PDebug("RSTB = 1 ");
    } else {
        PDebug("RSTB = 0 ");
    }

    if (GET_GPIO & RP_PIN_FACK) {
        PDebug("FACK = 1 ");
    } else {
        PDebug("FACK = 0 ");
    }
    puts("");
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
inline int fic_comm_setup() {
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

    return 0;
}

//-----------------------------------------------------------------------------
// DESC: Reset I/F FSM
//-----------------------------------------------------------------------------
int fic_comm_reset() {
    if (fic_comm_setup() < 0) return -1;

    // RREQ dessert
    if (fic_clr_gpio(RP_PIN_RREQ) < 0) return -1;

    // STB assert
    if (fic_set_gpio(RP_PIN_RSTB) < 0) return -1;

    return 0;
}
//-----------------------------------------------------------------------------
// DESC: Change RPi-FiC I/F transfer direction
//-----------------------------------------------------------------------------
static inline void fic_comm_portdir(enum COMM_PORT_DIR dir) {
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

//-----------------------------------------------------------------------------
// DESC: Wait until fack signal is down
//-----------------------------------------------------------------------------
static inline int fic_comm_wait_fack_down() {
    time_t t1, t2;
    time(&t1);
    while (GET_GPIO_PIN(RP_FACK) == 1) {
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
          PErr("Communication timeout");
          return -1;
        }
    }

    return 0;
}

static inline int fic_comm_wait_fack_down_notimeout() {
    int i = 0;
    while (GET_GPIO_PIN(RP_FACK) == 1) {
        if (i > COMM_TIMEOUT_NUM) return -1;
        i++;
        //asm("nop");
    }

    return 0;
}

//-----------------------------------------------------------------------------
// DESC: Wait until fack signal is up
//-----------------------------------------------------------------------------
static inline int fic_comm_wait_fack_up() {
    time_t t1, t2;
    time(&t1);
    while (GET_GPIO_PIN(RP_FACK) == 0) {
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
          PErr("Communication timeout");
          return -1;
        }
    }

    return 0;
}

static inline int fic_comm_wait_fack_up_notimeout() {
    int i = 0;
    while (GET_GPIO_PIN(RP_FACK) == 0) {
        if (i > COMM_TIMEOUT_NUM) return -1;
        i++;
        //asm("nop");
    }
    return 0;
}

//-----------------------------------------------------------------------------
// DESC: Wait until freq signal is down
//-----------------------------------------------------------------------------
static inline int fic_comm_wait_freq_down() {
    time_t t1, t2;
    time(&t1);
    while (GET_GPIO_PIN(RP_FREQ) == 1) {
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
          PErr("Communication timeout");
          return -1;
        }
    }

    return 0;
}

static inline int fic_comm_wait_freq_down_notimeout() {
    int i = 0;
    while (GET_GPIO_PIN(RP_FREQ) == 1) {
        if (i > COMM_TIMEOUT_NUM) return -1;
        i++;
        //asm("nop");
    }

    return 0;
}

//-----------------------------------------------------------------------------
// DESC: Wait until freq signal is up
//-----------------------------------------------------------------------------
static inline int fic_comm_wait_freq_up() {
    time_t t1, t2;
    time(&t1);
    while (GET_GPIO_PIN(RP_FREQ) == 0) {
        time(&t2);
        if (t2 - t1 > COMM_TIMEOUT) {
          PErr("Communication timeout");
          return -1;
        }
    }

    return 0;
}

static inline int fic_comm_wait_freq_up_notimeout() {
    int i = 0;
    while (GET_GPIO_PIN(RP_FREQ) == 0) {
        if (i > COMM_TIMEOUT_NUM) return -1;
        i++;
        //asm("nop");
    }

    return 0;
}

//-----------------------------------------------------------------------------
// DESC: Send single word data
//-----------------------------------------------------------------------------
static inline int fic_comm_send(uint32_t bus) {
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
        PErr("fic_comm_wait_fack_up failed");
        return -1;
    }

    if (fic_clr_gpio(RP_PIN_RSTB) < 0) {
        return -1;
    }

    if (fic_comm_wait_fack_down() < 0) {
        PErr("fic_comm_wait_fack_down failed");
        return -1;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// DESC: Send single word data (Fast ver)
//-----------------------------------------------------------------------------
static inline int fic_comm_send_fast(uint32_t bus) {
    int ret = 0;

    // Clr RSTB and DATA bus
#ifndef FICMK2
    ret |= fic_clr_gpio(RP_PIN_RSTB | RP_PIN_DATA7 | RP_PIN_DATA6 |
                        RP_PIN_DATA5 | RP_PIN_DATA4);

#else
    ret |= fic_clr_gpio(RP_PIN_RSTB |
                     RP_PIN_DATA0 | RP_PIN_DATA1 | RP_PIN_DATA2 | RP_PIN_DATA3 |
                     RP_PIN_DATA4 | RP_PIN_DATA5 | RP_PIN_DATA6 | RP_PIN_DATA7 |
                     RP_PIN_DATA8 | RP_PIN_DATA9 | RP_PIN_DATA10 | RP_PIN_DATA11 |
                     RP_PIN_DATA12 | RP_PIN_DATA13 | RP_PIN_DATA14 | RP_PIN_DATA15);

#endif

    ret |= fic_set_gpio_fast(RP_PIN_RSTB | bus);
    ret |= fic_comm_wait_fack_up_notimeout();
    ret |= fic_clr_gpio_fast(RP_PIN_RSTB);
    //ret |= fic_comm_wait_fack_down_notimeout();         // Omit check

    return ret;
}

static inline int fic_comm_receive() {
    if (fic_clr_gpio(RP_PIN_RSTB) < 0) return -1;
    if (fic_set_gpio(RP_PIN_RSTB) < 0) return -1;

    if (fic_comm_wait_fack_up() < 0) {
        PErr("fic_comm_wait_fack_up failed");
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
        PErr("fic_comm_wait_fack_down failed");
        return -1;
    }

    return (int) rcv;
}

static inline int fic_comm_receive_fast(uint32_t *rcv) {
    int ret = 0;

    ret |= fic_clr_gpio_fast(RP_PIN_RSTB);
    ret |= fic_set_gpio_fast(RP_PIN_RSTB);
    ret |= fic_comm_wait_fack_up_notimeout();

    if (ret < 0) return ret;

#ifndef FICMK2
    (GET_GPIO);
    (GET_GPIO);
    *rcv = (GET_GPIO >> RP_DATA_LOW) & 0xf;
#else
    // Note: mk2's 16bit I/F is unstable. so read it twice then check.
    (GET_GPIO);
    (GET_GPIO);
    (GET_GPIO);
    *rcv = (GET_GPIO >> RP_DATA_LOW) & 0xffff;

#endif

    ret |= fic_clr_gpio_fast(RP_PIN_RSTB);
//    ret |= fic_comm_wait_fack_down();         // Omit check

    return ret;
}

//static inline int fic_comm_receive_debug(uint32_t *rcv) {
//    *rcv = 0;
//
//    if (fic_clr_gpio(RP_PIN_RSTB) < 0) return -1;
//    if (fic_set_gpio(RP_PIN_RSTB) < 0) return -1;
//
//    if (fic_comm_wait_fack_up() < 0) {
//        PErr\( "[libfic2][ERROR]: fic_comm_wait_fack_up failed at %s %s %d",
//                  __FILE__, __FUNCTION__, __LINE__);
//        return -1;
//    }
//
//#ifndef FICMK2
//    *rcv = (GET_GPIO >> RP_DATA_LOW) & 0xff;
//#else
//    *rcv = (GET_GPIO >> RP_DATA_LOW) & 0xffff;
//#endif
//
//    if (fic_clr_gpio(RP_PIN_RSTB) < 0) return -1;
//
//    if (fic_comm_wait_fack_down() < 0) {
//        PErr\( "[libfic2][ERROR]: fic_comm_wait_fack_down failed at %s %s %d",
//                  __FILE__, __FUNCTION__, __LINE__);
//        return -1;
//    }
//
//    return 0;
//}

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
// Write fic register via 4bit interface
//-----------------------------------------------------------------------------
inline int fic_write(uint16_t addr, uint16_t data) {
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
// Read fic register via 4bit interface
//-----------------------------------------------------------------------------
inline int fic_read(uint16_t addr) {
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
static inline int _fic_hls_comm_initiate(enum RASIO_CMD cmd) {
    if (fic_comm_setup() < 0) return -1;
    fic_comm_portdir(COMM_PORT_SND);

    // RREQ assert
//    PDebug("DEBUG: RREQ assert");
    if (fic_set_gpio(RP_PIN_RREQ) < 0) return -1;
//    if (fic_comm_wait_freq_up() < 0) return -1;

    // Send command
//    PDebug("DEBUG: Send command");
    if (fic_comm_send(cmd << RP_DATA_LOW) < 0) return -1;

    // Set address at HLS module entry point 0x1000
//    PDebug("DEBUG: Send addr");
    if (fic_comm_setaddr(COMM_ADDR_HLS) < 0) return -1;

    // Change port direction if cmd is READ
    if (cmd == COMM_CMD_READ) {
        // Change port direction
        fic_comm_portdir(COMM_PORT_RCV);
    }

    return 0;
}

static inline int _fic_hls_comm_terminate() {
    // RREQ dessert
    //if (fic_clr_gpio(RP_PIN_RREQ | RP_PIN_RSTB | COMM_DATABUS_MASK) < 0) return -1;
    if (fic_clr_gpio(RP_PIN_RREQ | RP_PIN_RSTB) < 0) return -1;
    if (fic_comm_wait_freq_down() < 0) return -1;

    SET_ALL_INPUT;
 
    return 0;
}

static inline int _fic_hls_send_bytes(uint8_t *data, size_t size) {
    size_t i;
    int ret = 0;

#ifndef FICMK2
    // For mk1 board
    for (i = 0; i < size; i++) {
        ret |= fic_comm_send_fast(((*(data+i) & 0xf0) >> 4) << RP_DATA_LOW);  // 4bit high
        ret |= fic_comm_send_fast((*(data+i) & 0x0f) << RP_DATA_LOW);         // 4bit low

        if (i > 0 && (i % (1024*1024)) == 0) {
            PInfo("%s Send %d bytes",__FUNCTION__, i);
        }
    }
    PInfo("%s Send %d bytes",__FUNCTION__, i);

#else
    // For mk2 board
    // Transfer 2B data each time

    if (size < 2) {
        PErr("%d is too small (size should more than 2 byte", size);
        return -1;
    }

    if (size % 2 != 0) {
        PErr("%d is not 2B aligned size", size);
        return -1;
    }

    for (i = 0; i < size; i+=2) {
        ret |= fic_comm_send_fast((*(data+i) | *(data+i+1) << 8) << RP_DATA_LOW);  // 8bit + 8bit
        if ((i % (1024*1024)) == 0) {
            PInfo("%s Send %d bytes",__FUNCTION__, i);
        }
    }
    PInfo("%s Send %d bytes",__FUNCTION__, i);

#endif

    if (ret < 0) {
        PErr("fic_comm_send_bytes failed");
        return -1;
    }

    return 0;
}

inline int fic_hls_send(uint8_t *data, size_t size) {
    // Begin send to HLS
    if (_fic_hls_comm_initiate(COMM_CMD_WRITE) < 0) return -1;

    // Send Data
    if (_fic_hls_send_bytes(data, size) < 0) return -1;

    // End send to HLS
    if (_fic_hls_comm_terminate() < 0) return -1;
 
    return 0;
}

//-----------------------------------------------------------------------------
// Receive bytes via 4bit interface
// Modify 2019.11.30: Make the function receive each 1B on *buf via 4bit interface
//-----------------------------------------------------------------------------
static inline int _fic_hls_receive_bytes(uint8_t *data, size_t size) {
    size_t i;
    int ret = 0;

#ifndef FICMK2
    // for mk1
    // mk2 has 4bit word width = 0.5bytes at once

    for (i = 0; i < size; i++) {
        int rcv = 0;
        uint32_t rvh, rvl;

        // Receive Low 4bit
        ret = fic_comm_receive_fast(&rvh);
        if (ret < 0) break;

        // Receive High 4bit
        ret = fic_comm_receive_fast(&rvl);
        if (ret < 0) break;

        // For mk1 last 4bit is valid in rvh and rvl 
        *(data+i) = ((rvh << 4) | rvl) & 0xff;

        if (i > 0 && (i % (1024*1024)) == 0) {
            PInfo("%s Received %d bytes",__FUNCTION__, i);
        }
    }
    
    PInfo("%s Received %d bytes",__FUNCTION__, i);

#else
    // for mk2 
    // mk2 has 16bit word width = 2bytes at once

    if (size < 2) {
        PErr("%d is too small (size should more than 2 byte", size);
        return -1;
    }

    if (size % 2 != 0) {
        PErr("%d is must be 2B aligned size", size);
        return -1;
    }

    for (i = 0; i < size; i+=2) {
        uint32_t rv = 0;

        // Receive 16bit
        ret = fic_comm_receive_fast(&rv);
        if (ret < 0) break;

        // For mk2 received 2B at once
        *(data+i)   = (rv & 0x00ff) >> 0;
        *(data+i+1) = (rv & 0xff00) >> 8;


        if ((i % (1024*1024)) == 0) {
            PInfo("%s Received %d bytes",__FUNCTION__, i);
        }
    }

    PInfo("%s Received %d bytes",__FUNCTION__, i);

#endif

    if (ret < 0) {
        PErr("fic_comm_receive_bytes failed");
        return -1;
    }

    return 0;
}

inline int fic_hls_receive(uint8_t *buf, size_t size) {
    // Begin receive data from HLS
    if (_fic_hls_comm_initiate(COMM_CMD_READ) < 0) return -1;

    // Receive data
    if(_fic_hls_receive_bytes(buf, size) < 0) return -1;

    // Begin receive data from HLS
    if (_fic_hls_comm_terminate() < 0) return -1;

    return 0;
}

//-----------------------------------------------------------------------------
// Write data to DDR module (rasddr) via RPI 4bit interface
//-----------------------------------------------------------------------------
int fic_hls_write(uint8_t *data, size_t size, uint32_t addr, enum RASDDR_CMD ctrl) {
    uint32_t cmd[3];

    cmd[0] = ctrl;
    cmd[1] = addr;
    cmd[2] = size;

    PDebug("addr = %08x, size = %d", addr, size);

    // Initiate communication port
    if (_fic_hls_comm_initiate(COMM_CMD_WRITE) < 0) {
        PErr("Communication error");
        return -1;
    }

    // Send cmd
    if (_fic_hls_send_bytes((uint8_t *)&cmd, 12) < 0) {
        PErr("Communication error");
        return -1;
    }

    // Send data
    if (_fic_hls_send_bytes(data, size) < 0) {
        PErr("Communication error");
        return -1;
    }

    // Terminate communication port
    if (_fic_hls_comm_terminate() < 0) {
        PErr("Communication error");
        return -1;
    }

    return 0;
}

int fic_hls_ddr_write(uint8_t *data, size_t size, uint32_t addr) {
    return fic_hls_write(data, size, addr, RASDDR_CMD_WRITE);
}

//-----------------------------------------------------------------------------
// Receive bytes via 4bit interface
//-----------------------------------------------------------------------------
int fic_hls_read(uint8_t *buf, size_t size, uint32_t addr, enum RASDDR_CMD ctrl) {
    uint32_t cmd[3];

    cmd[0] = ctrl;
    cmd[1] = addr;
    cmd[2] = size;

    PDebug("DEBUG: addr = %08x, size = %d", addr, size);

    // Send cmd to DDR module
    if (fic_hls_send((uint8_t *)&cmd, 12) < 0) {  // cmd
        PErr("Communication error");
        return -1;
    }

    // Initiate communication port
    if (_fic_hls_comm_initiate(COMM_CMD_READ) < 0) {
        PErr("Communication error");
        return -1;
    }

    // Receive bytes from DDR module
    if (_fic_hls_receive_bytes(buf, size) < 0) {
        PErr("Communication error");
        return -1;
    }

    // Terminate communication port
    if (_fic_hls_comm_terminate() < 0) {
        PErr("Communication error");
        return -1;
    }

//    fic_hls_send((uint8_t *)&cmd, 12);   // cmd
//    fic_hls_receive(buf, size);          // size of data

    return 0;
}

int fic_hls_ddr_read(uint8_t *buf, size_t size,  uint32_t addr) {
    return fic_hls_read(buf, size, addr, RASDDR_CMD_READ);
}

//-----------------------------------------------------------------------------
// Receive bytes via 4bit interface
//-----------------------------------------------------------------------------
int fic_hls_ddr_debug(uint8_t *buf, size_t size, uint32_t addr) {
    int ret = 0;
    uint32_t cmd[3];

    cmd[0] = RASDDR_CMD_DEBUG;
    cmd[1] = addr;
    cmd[2] = size;

    ret = fic_hls_send((uint8_t *)&cmd, 12);   // CMD
    ret = fic_hls_receive(buf, 12);          // size of data

    if (ret < 0) return -1;
    return 0;
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

#ifdef FICMK2
    // Set bus switch to FPGA configuration mode
    SET_OUTPUT(RP_CFSEL);
    if (fic_set_gpio(RP_PIN_CFSEL) < 0) return -1;  // Set CFG mode
    PDebug("RP_CFSEL is set for FiC Mark2 board");
#endif

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

#ifdef FICMK2
    // Set bus switch to FPGA configuration mode
    SET_OUTPUT(RP_CFSEL);
    if (fic_set_gpio(RP_PIN_CFSEL) < 0) return -1;  // Set CFG mode
    PDebug("RP_CFSEL is set for FiC Mark2 board");
#endif

    return 0;
}

//-----------------------------------------------------------------------------
// FPGA Init
//-----------------------------------------------------------------------------
int fic_prog_init(enum PROG_MODE pm) {

#ifdef FICMK2
    SET_OUTPUT(RP_CFSEL);
    if (fic_set_gpio(RP_PIN_CFSEL) < 0) return -1;  // Set CFG mode
    PDebug("RP_CFSEL is set for FiC Mark2 board");
#endif

    // Pin setup for FPGA Init
    SET_INPUT(RP_PROG_B); SET_OUTPUT(RP_PROG_B);
    SET_INPUT(RP_CSI_B); SET_OUTPUT(RP_CSI_B);
    SET_INPUT(RP_RDWR_B); SET_OUTPUT(RP_RDWR_B);

    // Set disabled pins
    if (fic_set_gpio(RP_PIN_PROG_B | RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) return -1;

    // Partial reconfiguration mode
    if (pm == PM_PR) {
        if (fic_clr_gpio(RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) return -1;  // Assert
        return 0;
    }

    // Do FPGA init sequence
    if (fic_set_gpio(RP_PIN_PROG_B | RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) return -1;
    if (fic_clr_gpio(RP_PIN_PROG_B | RP_PIN_CSI_B | RP_PIN_RDWR_B) < 0) return -1;
    if (fic_set_gpio(RP_PIN_PROG_B) < 0) return -1;

    while (GET_GPIO_PIN(RP_INIT) == 0) {
        PDebug("Awaiting FPGA reset...");
        (GET_GPIO);
        usleep(1000);
    }

    if (GET_GPIO_PIN(RP_DONE) == 1) {
        PDebug("FPGA reset failed");
        return -1;
    }

    PDebug("FPGA reset success...");

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

//-----------------------------------------------------------------------------
size_t fic_prog_sm16(uint8_t *data, size_t size, enum PROG_MODE pm) {

    // For profiler metrics
    PROG_ASYNC_STATUS.stat         = PM_STAT_PROG;
    PROG_ASYNC_STATUS.smap_mode    = PM_SMAP_16;
    PROG_ASYNC_STATUS.prog_mode    = pm;
    PROG_ASYNC_STATUS.prog_st_time = time(NULL);
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

    uint32_t _d = 0;
    for (i = 0; i < size; i+=2) {
        int ret = 0;
        uint32_t d = (data[i+1] << 8 | data[i]) << 8;
        if (d == _d) {
            ret |= fic_clr_gpio_fast(RP_PIN_CCLK);
            ret |= fic_set_gpio_fast(RP_PIN_CCLK);
//            SET_GPIO = RP_PIN_CCLK;

        } else {
//            CLR_GPIO = (~d & 0x00ffff00) | RP_PIN_CCLK;
//            SET_GPIO = d & 0x00ffff00;
//            SET_GPIO = (d & 0x00ffff00) | RP_PIN_CCLK;
            ret |= fic_clr_gpio((~d & 0x00ffff00) | RP_PIN_CCLK);
            ret |= fic_set_gpio(d & 0x00ffff00);
            ret |= fic_set_gpio((d & 0x00ffff00) | RP_PIN_CCLK);

        }

        if (ret < 0) goto PM_SM16_EXIT_ERROR;

        _d = d;

        PROG_ASYNC_STATUS.tx_size += 2;

        // Show progress
        time(&t2);
        if (t2 - t1 > 2) {
            PInfo("Transfer %d / %d [%.02f %%]", i, size, (i/(float)size)*100);
            t1 = t2;
        }

        if (GET_GPIO_PIN(RP_INIT) == 0) {
            PErr("FPGA configuration failed");
            goto PM_SM16_EXIT_ERROR;
        }
    }

    // Wait until RP_DONE asserted
    if (pm == PM_NORMAL) {
        PDebug("Waiting for RP_DONE");
        time(&t1);
        while (GET_GPIO_PIN(RP_DONE) == 0) {
            time(&t2);
            if (GET_GPIO_PIN(RP_INIT) == 0 || t2 - t1 > COMM_TIMEOUT) {
                PErr("FPGA configuration failed");
                goto PM_SM16_EXIT_ERROR;
            }
            if (fic_set_gpio(RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
            if (fic_clr_gpio(RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
        }
        PDebug("RP_DONE");
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
size_t fic_prog_sm16_fast(uint8_t *data, size_t size, enum PROG_MODE pm) {

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
            PInfo("Transfer %d / %d [%.02f %%]", i, size, (i/(float)size)*100);
            t1 = t2;
        }
#endif

    }

    // If something happen during send configuration
    if (GET_GPIO_PIN(RP_INIT) == 0) {
        PErr("FPGA configuration failed");
        goto PM_SM16_EXIT_ERROR;
    }

    // Wait until RP_DONE asserted
    if (pm == PM_NORMAL) {
        PDebug("Waiting for RP_DONE");
        time(&t1);
        while (GET_GPIO_PIN(RP_DONE) == 0) {
            time(&t2);
            if (GET_GPIO_PIN(RP_INIT) == 0 || t2 - t1 > COMM_TIMEOUT) {
                PErr("FPGA configuration failed");
                goto PM_SM16_EXIT_ERROR;
            }
            if (fic_set_gpio(RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
            if (fic_clr_gpio(RP_PIN_CCLK) < 0) goto PM_SM16_EXIT_ERROR;
        }
        PDebug("[libfic2][DEBUG]: RP_DONE");
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
    PROG_ASYNC_STATUS.prog_st_time = time(NULL);
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

    uint32_t _d = 0;
    for (i = 0; i < size; i++) {
        uint32_t d = (data[i] << 8);
        if (d == _d) {
            fic_clr_gpio_fast(RP_PIN_CCLK); // Securely CCLK down
            SET_GPIO = RP_PIN_CCLK;

        } else {
            CLR_GPIO = (~d & 0x0000ff00) | RP_PIN_CCLK;
            SET_GPIO = d & 0x0000ff00;
            SET_GPIO = (d & 0x0000ff00) | RP_PIN_CCLK;
        }

        _d = d;

        PROG_ASYNC_STATUS.tx_size++;

        // Show progress
        time(&t2);
        if (t2 - t1 > 2) {
            PInfo("Transfer %d / %d [%.02f %%]", i, size, (i/(float)size)*100);
            t1 = t2;
        }

        if (GET_GPIO_PIN(RP_INIT) == 0) {
            PErr("FPGA configuration failed");
            goto PM_SM8_EXIT_ERROR;
        }
    }

    // Waitng RP_DONE asserted
    if (pm == PM_NORMAL) {
        PDebug("Waiting for RP_DONE");
        time(&t1);
        while (GET_GPIO_PIN(RP_DONE) == 0) {
            time(&t2);
            if (GET_GPIO_PIN(RP_INIT) == 0 || t2 - t1 > COMM_TIMEOUT) {
                PErr("FPGA configuration failed");
                goto PM_SM8_EXIT_ERROR;
            }
            if (fic_set_gpio(RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
            if (fic_clr_gpio(RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
        }
        PDebug("RP_DONE");
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
size_t fic_prog_sm8_fast(uint8_t *data, size_t size, enum PROG_MODE pm) {

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
            PInfo("Transfer %d / %d [%.02f %%]", i, size, (i/(float)size)*100);
            t1 = t2;
        }
#endif

    }

    // If something happen during send configuration
    if (GET_GPIO_PIN(RP_INIT) == 0) {
        PErr("FPGA configuration failed");
        goto PM_SM8_EXIT_ERROR;
    }

    // Waitng RP_DONE asserted
    if (pm == PM_NORMAL) {
        PDebug("Waiting for RP_DONE");
        time(&t1);
        while (GET_GPIO_PIN(RP_DONE) == 0) {
            time(&t2);
            if (GET_GPIO_PIN(RP_INIT) == 0 || t2 - t1 > COMM_TIMEOUT) {
                PErr("FPGA configuration failed");
                goto PM_SM8_EXIT_ERROR;
            }
            if (fic_set_gpio(RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
            if (fic_clr_gpio(RP_PIN_CCLK) < 0) goto PM_SM8_EXIT_ERROR;
        }
        PDebug("RP_DONE");
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
// DESC: Send start hls
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

//-----------------------------------------------------------------------------
// DESC: Send reset hls
//-----------------------------------------------------------------------------
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
    PDebug("Obtaining GPIO lock...");

    // Check LOCKFILE
    time_t t1, t2;
    time(&t1);
    // Wait while lockfile is existed
    struct stat st;
    while (stat(LOCK_FILE, &st) == 0) {
        sleep(1);
        PDebug("Attempting obtaining GPIO lock...");
        time(&t2);
        if ((t2 - t1) > GPIO_LOCK_TIMEOUT) {
            PErr("GPIO lock timeout");
            return -1;
        }
    }

    // Create LOCKFILE
    int lock_fd = open(LOCK_FILE, O_CREAT|O_RDONLY, 0666);
    if (lock_fd < 0) {
		PErr("Cant create LOCK_FILE");
        return -1;
    }

    // flock LOCKFILE
    if (flock(lock_fd, LOCK_EX) < 0) {
		PErr("Cant lock LOCK_FILE");
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
		PErr("Cant remove LOCK_FILE");
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
		PErr("GPIO open failed");
        return -1;
    }

	/* open GPIO_DEV */
    int mem_fd = open(GPIO_DEV, O_RDWR|O_SYNC);
	if (mem_fd < 0) {
		PErr("can't open %s", GPIO_DEV);
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
		PErr("mmap error %d", (int)gpio_map);   //errno also set!
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

//-----------------------------------------------------------------------------

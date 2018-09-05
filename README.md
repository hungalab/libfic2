
# FiC Library 2

FiC library2 by nyacom <kzh@nyacom.net>

The library inspirated by original ficlib by hunga-san.

----

# APIs

## General

### GPIO *fic_gpio_open()
- Open GPIO and creates LOCKFILE
- return == NULL is error

### GPIO *fic_gpio_close()
- Close GPIO and removes LOCKFILE
- return == NULL is error

----

## FiC FPGA programmer

### fic_prog_sm16(GPIO *g, uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx)
- Program FiC FPGA by SelectMap x16 mode 
- PROG_MODE is reset PROG_NORMAL or PROG_PR (Partial reconfiguration)
- *tx is a size of transffered byte
- return == -1 is error

### fic_prog_sm8(GPIO *g, uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx)
- Program FiC FPGA by SelectMap x8 mode 
- PROG_MODE is reset PROG_NORMAL or PROG_PR (Partial reconfiguration)
- *tx is a size of transffered byte

### fic_prog_init(GPIO *g)
- Invoke FPGA init

----

## FiC General Read Write I/F
Note: The interface available after FiCSW FPGA programmed.

### int fic_wb8(uint16_t addr, uint8_t *data)
- Write single byte *data via fic 8bit interface.
- return -1 is error.

### int fic_rb8(uint16_t addr, uint8_t *buf)
- Read single byte from addr to *buf via fic 8bit interface.
- return -1 and buf == NULL is error.

### int fic_wb4(uint16_t addr, uint8_t *data)
- Write single byte *data via fic 4bit interface.
- return -1 is error.

### int fic_rb4(uint16_t addr, uint8_t *buf)
- Read single byte from addr to *buf via fic 8bit interface.
- return -1 and buf == NULL is error.

----

## FiC HLS module communication

### void fic_hls_reset4()
- Send reset HLS reset command

### void fic_hls_reset8()
- Send reset HLS reset command

### void fic_hls_start4()
- Send reset HLS start command

### void fic_hls_start8()
- Send reset HLS start command

### int fic_hls_send4(uint8_t *data, size_t size)
- Send size bytes of *data to the HLS module via 4bit interface
- All data is masked with 0x0f (so that only lower 4bit is valid)
- return -1 is error

### int fic_hls_receive4(uint8_t *buf, size_t size)
- Receive size bytes of *data from HLS module via 4bit interface
- return -1 and buf == NULL is error.

----

# FAQ

## What is the lockfile?
To prevent confilicted GPIO manupulation, the library uses LOCKFILE to prevent confliction. This file will be creating /tmp/gpio.lock

----

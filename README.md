
# FiC Library 2

FiC library2 by nyacom <kzh@nyacom.net>

The library inspirated by original ficlib by hunga-san.

----

# APIs

## General

### int fic_gpio_open()
- Open GPIO and creates LOCKFILE
- return -1 is error

### int fic_gpio_close()
- Close GPIO and removes LOCKFILE
- return -1 is error

----

## FiC FPGA programmer

### size_t fic_prog_sm16(uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx)
- Program FiC FPGA by SelectMap x16 mode 
- PROG_MODE is reset PROG_NORMAL or PROG_PR (Partial reconfiguration)
- Progress can observe by *tx
- return == 0 is error

### size_t fic_prog_sm8(uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx)
- Program FiC FPGA by SelectMap x8 mode 
- PROG_MODE is reset PROG_NORMAL or PROG_PR (Partial reconfiguration)
- Progress can observe by *tx
- return == 0 is error

### void fic_prog_init()
- Invoke FPGA init

----

## FiC General Read Write I/F
Note: The interface available after FiCSW FPGA programmed.

### int fic_wb8(uint16_t addr, uint8_t data)
- Write single byte data via fic 8bit interface.
- return -1 is error.

### int fic_rb8(uint16_t addr)
- Read single byte from addr to *buf via fic 8bit interface.
- return -1 is error.

### int fic_wb4(uint16_t addr, uint8_t data)
- Write single byte data via fic 4bit interface.
- return -1 is error.

### int fic_rb4(uint16_t addr)
- Read single byte from addr to *buf via fic 8bit interface.
- return -1 is error.

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

### int fic_hls_send4(uint8_t *data, size_t count)
- Send size bytes of *data to the HLS module via 4bit interface
- All data is masked with 0x0f (so that only lower 4bit is valid)
- return -1 is error

### int fic_hls_receive4(uint8_t *buf, size_t count)
- Receive size bytes of *data from HLS module via 4bit interface
- return -1 and buf == NULL is error.

----

# Python bindings
pyficlib2 is a python bindings of ficlib2.

## Usage
Import pyficlib like below

```
import pyficlib2 as fic
...
fic.gpio_open()

-- do something --

fic.gpio_close()

```

----
## Methods

### gpio_open()
- Wrap function of fic_gpio_open()

### gpio_close()
- Wrap function of fic_gpio_close()

### prog_sm16(data=_bytes_, progmode=_int_)
- Wrap funcion of fic_prog_sm16
- progmode = 0 is Normal FPGA prog mode
- progmode = 1 is Normal PR FPGA prog mode
- return is transfered bytes

### prog_sm8(data=_bytes_, progmode=_int_)
- Wrap funcion of fic_prog_sm8
- progmode = 0 is Normal FPGA prog mode
- progmode = 1 is Normal PR FPGA prog mode
- return is transfered bytes

### ~~prog_tx()~~
- ~~Get trasnfered byte while prog_sm16 or prog_sm8~~

### prog_init()
- Wrap function of fic_prog_init
- Init FPGA and erase configuration

### rb8(addr=_int_)
- Wrap function of fic_rb8
- addr is single 16bit address
- return is single _bytes_ at addr

### rb4(addr=_int_, data=_bytes_)
- Wrap function of fic_rb4
- addr is 16bit address value
- return is single _bytes_ at addr

### wb8(addr=_int_, data=_bytes_)
- Wrap function of fic_wb8
- addr is 16bit address value
- data is _bytes_ value

### wb4(addr=_int_, data=_bytes_)
- Wrap function of fic_wb4
- addr is 16bit address value
- data is _bytes_ value

### hls_reset8()
- Wrap function of fic_hls_reset8
- Send HLS reset CMD via 8bit I/F

### hls_reset4()
- Wrap function of fic_hls_reset4
- Send HLS reset CMD via 4bit I/F

### hls_start8()
- Wrap function of fic_hls_start8
- Send HLS start CMD via 8bit I/F

### hls_start4()
- Wrap function of fic_hls_start4
- Send HLS start CMD via 4bit I/F

### hls_send4(data=_bytes_)
- Wrap function of fic_hls_send4
- data is python _bytes_
- __NOT WELL TESTED__

### hls_receive4(count=_int_)
- Wrap function of fic_hls_receive4
- Read _count_ of data from HLS module 
- Return is _bytes_
- __NOT WELL TESTED__

----
# FAQ

## What is the lockfile?
To prevent confilicted GPIO manupulation, the library uses LOCKFILE to prevent confliction. This file will be creating /tmp/gpio.lock

----

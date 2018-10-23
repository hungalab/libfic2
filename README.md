
# FiC Library 2

FiC library2 by nyacom <kzh@nyacom.net>

The library inspirated by original ficlib by hunga-san.

----
## Table of contents

<!-- TOC -->

- [FiC Library 2](#fic-library-2)
    - [Table of contents](#table-of-contents)
- [Build](#build)
- [APIs](#apis)
    - [General](#general)
        - [int fic_gpio_open()](#int-fic_gpio_open)
        - [int fic_gpio_close()](#int-fic_gpio_close)
        - [int fic_power()](#int-fic_power)
        - [int fic_done()](#int-fic_done)
    - [FiC FPGA programmer](#fic-fpga-programmer)
        - [size_t fic_prog_sm16(uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx)](#size_t-fic_prog_sm16uint8_t-data-size_t-size-enum-prog_mode-pm-size_t-tx)
        - [size_t fic_prog_sm8(uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx)](#size_t-fic_prog_sm8uint8_t-data-size_t-size-enum-prog_mode-pm-size_t-tx)
        - [void fic_prog_init()](#void-fic_prog_init)
    - [FiC General Read Write I/F](#fic-general-read-write-if)
        - [int fic_wb8(uint16_t addr, uint8_t data)](#int-fic_wb8uint16_t-addr-uint8_t-data)
        - [int fic_rb8(uint16_t addr)](#int-fic_rb8uint16_t-addr)
        - [int fic_wb4(uint16_t addr, uint8_t data)](#int-fic_wb4uint16_t-addr-uint8_t-data)
        - [int fic_rb4(uint16_t addr)](#int-fic_rb4uint16_t-addr)
    - [FiC HLS module communication](#fic-hls-module-communication)
        - [void fic_hls_reset4()](#void-fic_hls_reset4)
        - [void fic_hls_reset8()](#void-fic_hls_reset8)
        - [void fic_hls_start4()](#void-fic_hls_start4)
        - [void fic_hls_start8()](#void-fic_hls_start8)
        - [int fic_hls_send4(uint8_t *data, size_t count)](#int-fic_hls_send4uint8_t-data-size_t-count)
        - [int fic_hls_receive4(uint8_t *buf, size_t count)](#int-fic_hls_receive4uint8_t-buf-size_t-count)
- [Python bindings](#python-bindings)
    - [Usage](#usage)
    - [Methods](#methods)
        - [gpio_open()](#gpio_open)
        - [gpio_close()](#gpio_close)
        - [prog_sm16(data=_bytes_, progmode=_int_)](#prog_sm16data_bytes_-progmode_int_)
        - [prog_sm8(data=_bytes_, progmode=_int_)](#prog_sm8data_bytes_-progmode_int_)
        - [~~prog_tx()~~](#prog_tx)
        - [prog_init()](#prog_init)
        - [rb8(addr=_int_)](#rb8addr_int_)
        - [rb4(addr=_int_, data=_bytes_)](#rb4addr_int_-data_bytes_)
        - [wb8(addr=_int_, data=_bytes_)](#wb8addr_int_-data_bytes_)
        - [wb4(addr=_int_, data=_bytes_)](#wb4addr_int_-data_bytes_)
        - [hls_reset8()](#hls_reset8)
        - [hls_reset4()](#hls_reset4)
        - [hls_start8()](#hls_start8)
        - [hls_start4()](#hls_start4)
        - [hls_send4(data=_bytes_)](#hls_send4data_bytes_)
        - [hls_receive4(count=_int_)](#hls_receive4count_int_)
- [FAQ](#faq)
    - [What is the lockfile?](#what-is-the-lockfile)

<!-- /TOC -->

----

# Build

To build ficlib2, you should do for preparation on your Raspbian.

    sudo apt install gcc make python3-dev

then make clean && make at ficlib2 directory.

    make clean && make

after compilation, you will get ficlib2 and pyficlib2.so

pyficlib2.so is a python binding version of ficlib2. (please refer pyficlibtest.py for an example.)

----

# APIs

## General

### int fic_gpio_open()
- Open GPIO and creates LOCKFILE
- return -1 is error

### int fic_gpio_close()
- Close GPIO and removes LOCKFILE
- return -1 is error

### int fic_power()
- Probe PW_OK signal from FiC board

### int fic_done()
- Probe DONE singal from FPGA

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

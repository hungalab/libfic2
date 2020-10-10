
# FiC Library 2

FiC library2 by nyacom <kzh@nyacom.net>

The library inspirated by original ficlib by hunga-san.

## NOTE: Due to support both mk1 and mk2 boards, API names has changed (Remove bitwidth from API calls).

----
## Table of contents

<!-- TOC -->

- [FiC Library 2](#fic-library-2)
  - [NOTE: Due to support both mk1 and mk2 boards, API names has changed (Remove bitwidth from API calls).](#note-due-to-support-both-mk1-and-mk2-boards-api-names-has-changed-remove-bitwidth-from-api-calls)
  - [Table of contents](#table-of-contents)
- [Build](#build)
- [APIs](#apis)
  - [General](#general)
    - [int fic_gpio_open()](#int-ficgpioopen)
    - [int fic_gpio_close(int fd_lock)](#int-ficgpiocloseint-fdlock)
    - [int fic_power()](#int-ficpower)
    - [int fic_done()](#int-ficdone)
  - [FiC FPGA programmer](#fic-fpga-programmer)
    - [size_t fic_prog_sm16(uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx)](#sizet-ficprogsm16uint8t-data-sizet-size-enum-progmode-pm-sizet-tx)
    - [size_t fic_prog_sm8(uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx)](#sizet-ficprogsm8uint8t-data-sizet-size-enum-progmode-pm-sizet-tx)
    - [void fic_prog_init()](#void-ficproginit)
  - [FiC General Read Write I/F](#fic-general-read-write-if)
    - [int fic_write(uint16_t addr, uint16_t data)](#int-ficwriteuint16t-addr-uint16t-data)
    - [int fic_read(uint16_t addr)](#int-ficreaduint16t-addr)
  - [FiC HLS module communication](#fic-hls-module-communication)
    - [void fic_hls_reset()](#void-fichlsreset)
    - [void fic_hls_start()](#void-fichlsstart)
    - [int fic_hls_send(uint8_t *data, size_t count)](#int-fichlssenduint8t-data-sizet-count)
    - [int fic_hls_receive(uint8_t *buf, size_t count)](#int-fichlsreceiveuint8t-buf-sizet-count)
- [Python bindings](#python-bindings)
  - [Usage](#usage)
  - [Methods](#methods)
    - [gpio_open()](#gpioopen)
    - [gpio_close()](#gpioclose)
    - [prog_sm16(data=_bytes_, progmode=_int_)](#progsm16databytes-progmodeint)
    - [prog_sm8(data=_bytes_, progmode=_int_)](#progsm8databytes-progmodeint)
    - [~~prog_tx()~~](#progtx)
    - [prog_init()](#proginit)
    - [fic_read(addr=_int_)](#ficreadaddrint)
    - [fic_write(addr=_int_, data=_data_)](#ficwriteaddrint-datadata)
    - [hls_reset()](#hlsreset)
    - [hls_start()](#hlsstart)
    - [hls_send(data=_bytes_)](#hlssenddatabytes)
    - [hls_receive(count=_int_)](#hlsreceivecountint)
- [FAQ](#faq)
  - [What is the lockfile?](#what-is-the-lockfile)
- [pyficprog.py](#pyficprogpy)
  - [What is this?](#what-is-this)

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

## General functions

### int fic_gpio_open()
- Open GPIO and creates LOCKFILE
- return is LOCKFILE fd
- return -1 is error

### int fic_gpio_close(int fd_lock)
- Close GPIO and removes LOCKFILE
- return -1 is error

### int fic_power()
- Probe PW_OK signal from FiC board

### int fic_done()
- Probe DONE singal from FPGA

### int fic_comm_reset()
- Reset FiC I/O interface

----

## FiC FPGA programmer functions

### size_t fic_prog_sm16(uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx)
- Program FiC FPGA by SelectMap x16 mode 
- PROG_MODE is reset PROG_NORMAL or PROG_PR (Partial reconfiguration)
- Progress can observe by *tx
- return == -1 is error

### size_t fic_prog_sm8(uint8_t *data, size_t size, enum PROG_MODE pm, size_t *tx)
- Program FiC FPGA by SelectMap x8 mode 
- PROG_MODE is reset PROG_NORMAL or PROG_PR (Partial reconfiguration)
- Progress can observe by *tx
- return == -1 is error

### void fic_prog_init()
- Invoke FPGA init (Reset FPGA)

----

## FiC General Read Write I/F functions
Note: The interface available after FiCSW FPGA programmed.

### int fic_write(uint16_t addr, uint16_t data)
- Write single byte data via fic 4bit interface.
- return -1 is error.

### int fic_read(uint16_t addr)
- Read single byte from addr to *buf via fic 8bit interface.
- return -1 is error.

----

## FiC HLS module communication functions
Note: The interface available after FiCSW FPGA programmed.

### void fic_hls_reset()
- Send reset HLS reset command

### void fic_hls_start()
- Send reset HLS start command

### int fic_hls_send(uint8_t *data, size_t count)
- Send size bytes of *data to the HLS module via 4bit interface
- All data is masked with 0x0f (so that only lower 4bit is valid)
- return -1 is error

### int fic_hls_receive(uint8_t *buf, size_t count)
- Receive size bytes of *data from HLS module via 4bit interface
- return -1 and buf == NULL is error.

### int fic_hls_ddr_write(uint8_t *data, size_t size, uint32_t addr)
- Write to data to DDR module
- Size and addr must be aligned to 4B (32bit)

### int fic_hls_ddr_read(uint8_t *buf, size_t size,  uint32_t addr);
- Read data from DDR module
- Size and addr must be aligned to 4B (32bit)

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

### fic_read(addr=_int_)
- Wrap function of fic_read
- addr is 16bit address value
- return is single 8bit or 16bit value at addr

### fic_write(addr=_int_, data=_data_)
- Wrap function of fic_write
- addr is 16bit address value
- data is _data_ value

### hls_reset()
- Wrap function of fic_hls_reset
- Send HLS reset CMD

### hls_start()
- Wrap function of fic_hls_start4
- Send HLS start CMD via 4bit I/F

### hls_send(data=_bytes_)
- Wrap function of fic_hls_send
- data is python _bytes_
- __NOT WELL TESTED__

### hls_receive(count=_int_)
- Wrap function of fic_hls_receive
- Read _count_ of data from HLS module 
- Return is _bytes_
- __NOT WELL TESTED__

----
# FAQ

## What is the lockfile?
To prevent confilicted GPIO manupulation, the library uses LOCKFILE to prevent confliction. This file will be creating /tmp/gpio.lock

----

# pyficprog.py

## What is this?
An alternative ficprog utility using libfic2.

You can configure FPGA on RPi3 like follows:

```
nyacom@m2fic12:~/project/fic/libfic2$ ./pyficprog.py -h
usage: pyficprog.py [-h] [-m [{8,16}]] [-pr] bitfile

positional arguments:
  bitfile               FPGA configuration *.bit

optional arguments:
  -h, --help            show this help message and exit
  -m [{8,16}], --mode [{8,16}]
                        Select map mode
  -pr, --partial        Partial reconfigure mode

nyacom@m2fic12:~/project/fic/libfic2$ ./pyficprog.py -m16 ../fpga/mk2virt4x4/fic_top_sm16.bin

 pyficprog FiC FPGA configuration utility       /\/\
 by nyacom 2019 (C) <kzh@nyacom.net>          =(____)=
------------------------------------------------U--U--------

INFO: Program mode: Selectmap 16
INFO: Partial mode: False
INFO: Open file ../fpga/mk2virt4x4/fic_top_sm16.bin
INFO: Bitstream is loaded 48251520B
INFO: RP_CFSEL is set for FiC Mark2 board
INFO: RP_CFSEL is set for FiC Mark2 board
Transfer 5217302 / 48251520 [10.81 %]
Transfer 11549474 / 48251520 [23.94 %]
Transfer 17871662 / 48251520 [37.04 %]
Transfer 24191372 / 48251520 [50.14 %]
Transfer 30519058 / 48251520 [63.25 %]
Transfer 36846288 / 48251520 [76.36 %]
Transfer 43177604 / 48251520 [89.48 %]
DEBUG: Waiting for RP_DONE
DEBUG: RP_DONE
INFO: FPGA configuration done

```

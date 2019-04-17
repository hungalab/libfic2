#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#------------------------------------------------------------------------------
# pyficlib2 test
# by nyacom (C) 2018
#------------------------------------------------------------------------------

import sys, traceback
import os
import struct
import argparse
import time
from multiprocessing import Process
import pyficlib2 as fic 
#import numba
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
# EXAMPLE 1 : Program FPGA
#------------------------------------------------------------------------------
fd = fic.gpio_open()

print("POWER:", fic.get_power())
print("DONE :", fic.get_done())

print("EXAMPLE1: Program FPGA")

# ##bit = open("AURORA.bin", "rb").read()
# #bit = open("fic_top.bin", "rb").read()
# bit = open("mk2_fic_top.bin", "rb").read()
# print("DEBUG: bitfile size=", len(bit))
# fic.prog_sm16(bit, 0)

fic.write(0xffff, 48)
print(fic.read(0xffff))

##fic.prog_sm8(bit, 0)

fic.gpio_close(fd)

#------------------------------------------------------------------------------
#fic.wb8(0xffff, b'A')
#print(fic.rb8(0xffff))

##------------------------------------------------------------------------------
#if __name__ == '__main__':
#    obj = gpiotest()
#    obj.run()


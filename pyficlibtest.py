#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys, traceback
import os
import struct
import argparse
import time
import pyficlib2 as fic 
#import numba
#------------------------------------------------------------------------------
fic.gpio_open()

#bit = open("ring_8bit.bin", "rb").read()
#print("DEBUG: size=", len(bit))
#tx = fic.prog_sm16(bit, 0)
#print("DEBUG: tx=", tx)

fic.wb8(0xffff, b'A')
print(fic.rb8(0xffff))

fic.gpio_close()
##------------------------------------------------------------------------------
#if __name__ == '__main__':
#    obj = gpiotest()
#    obj.run()


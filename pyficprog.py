#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#------------------------------------------------------------------------------
# pyficprog (ficprog using pyficlib)
# by nyacom (C) 2018
#------------------------------------------------------------------------------
import sys
import traceback
import os
import struct
import argparse
import time
import pdb
import pyficlib2 as Fic 
#------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Note: pyficlib2 is not manage any state
# so this library is for only handle open/close 
# ------------------------------------------------------------------------------
class Opengpio:
    def __init__(self):
        self.fd_lock = 0

    def __enter__(self):
        try:
            self.fd_lock = Fic.gpio_open()
            #print("DEBUG: gpio_open()", self.fd_lock)

        except:
            raise IOError
            
        return self

    def __exit__(self, type, value, traceback):
        try:
            Fic.gpio_close(self.fd_lock)
            #print("DEBUG: gpio_close()", self.fd_lock)

        except:
            print("DEBUG: gpio_close() failed")
            raise IOError

#------------------------------------------------------------------------------
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-m', '--mode', nargs='?', choices=['8', '16'], default='8', help='Select map mode')
    parser.add_argument('-pr', '--partial', action='store_true', help='Partial reconfigure mode')
    parser.add_argument('-f', '--fast', action='store_true', help='Fast configuration mode')
    parser.add_argument('bitfile', nargs=1, type=str, help='FPGA configuration *.bit')
    args = parser.parse_args()

    print(u"")
    print(u" pyficprog FiC FPGA configuration utility       /\/\        ")
    print(u" by nyacom 2019 (C) <kzh@nyacom.net>          =(____)=      ")
    print(u"------------------------------------------------U--U--------")
    print(u"")

#    print("DEBUG:", args)

    try:
        print("INFO: Program mode: Selectmap {0}".format(args.mode))
        print("INFO: Partial mode: {0}".format(args.partial))
        print("INFO: Open file {0}".format(args.bitfile[0]))

        with open(args.bitfile[0], 'rb') as f:
            bs = f.read()
            print("INFO: Bitstream is loaded {0}B".format(len(bs)))
            with Opengpio():
                if args.mode == '8':
                    if args.fast:
                        if args.partial:
                            Fic.prog_sm8_fast(data=bs, progmode=1)

                        else:
                            Fic.prog_sm8_fast(data=bs, progmode=0)

                    else:
                        if args.partial:
                            Fic.prog_sm8(data=bs, progmode=1)

                        else:
                            Fic.prog_sm8(data=bs, progmode=0)

                elif args.mode == '16':
                    if args.fast:
                        if args.partial:
                            Fic.prog_sm16_fast(data=bs, progmode=1)

                        else:
                            Fic.prog_sm16_fast(data=bs, progmode=0)

                    else:
                        if args.partial:
                            Fic.prog_sm16(data=bs, progmode=1)

                        else:
                            Fic.prog_sm16(data=bs, progmode=0)

            print("INFO: FPGA configuration done")

    except Exception as e:
        ex, ms, tb = sys.exc_info()
        print("ERROR: Something Happened", file=sys.stderr)
        traceback.print_exc(ex)
        traceback.print_tb(tb)

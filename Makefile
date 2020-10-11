BIN=ficlib2
CC=gcc -O2 -g -lpthread

mk1:
#	make $(BIN)
	make pyficlib2.so
#	make libfic2.so
	make testddr

mk2:
	make libfic2.so CFLAGS=-DFICMK2
	make pyficlib2.so CFLAGS=-DFICMK2
	make testddr CFLAGS=-DFICMK2
	make testprog CFLAGS=-DFICMK2

# all:
# 	make $(BIN)
# 	make pyficlib2.so

.c.o:
	$(CC) $(CFLAGS) -c $<

$(BIN): ficlib2.o
	$(CC) $(CFLAGS) $? -o $(BIN)

libfic2.so: ficlib2.c
	$(CC) $(CFLAGS) $? -shared -DSHARED_LIB -o $@

pyficlib2.so: pyficlib2.c ficlib2.o
	$(CC) $(CFLAGS) $? -shared -I/usr/include/python3.5 -o $@

#------------------------------------------------------------------------------
# Test scripts
#------------------------------------------------------------------------------
testddr: testddr.o ficlib2.o
	$(CC) $(CFLAGS) $^ -o $@

testprog: testprog.o ficlib2.o
	$(CC) $(CFLAGS) $^ -o $@

#------------------------------------------------------------------------------
clean:
	rm -rf $(BIN)
	rm -rf *.o
	rm -rf libfic2.so
	rm -rf pyficlib2.so
	rm -rf /tmp/gpio.lock

run: $(BIN)
	rm -rf /tmp/gpio.lock && ./$(BIN)

debug:
	gdb ./$(BIN)


BIN=ficlib2
CC=gcc -O2 -g -lpthread

mk1:
	make $(BIN)
	make pyficlib2.so

mk2:
	make $(BIN) CFLAGS=-DFICMK2
	make pyficlib2.so

# all:
# 	make $(BIN)
# 	make pyficlib2.so

.c.o:
	$(CC) $(CFLAGS) -c $<

$(BIN): ficlib2.o
	$(CC) $(CFLAGS) $? -o $(BIN)

libfic2.so: ficlib2.c
	$(CC) $? -shared -DSHARED_LIB -o $@

pyficlib2.so: pyficlib2.c ficlib2.o
	$(CC) $? -shared -I/usr/include/python3.5 -o $@

clean:
	rm -rf $(BIN)
	rm -rf *.o
	rm -rf pyficlib2.so
	rm -rf /tmp/gpio.lock

run: $(BIN)
	rm -rf /tmp/gpio.lock && ./$(BIN)

debug:
	gdb ./$(BIN)

